#pragma once

#include <entt.hpp>
#include <functional>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <cmath>

// ============================================================================
// music_components — generic musical-time ECS components
// ============================================================================
// These are backend-agnostic: no audio library, no MIDI library dependency.
// They model time, transport, patterns, and note events as ECS data so that
// any system (ofxKit Properties panel, node patcher, ofxEnTTInspector) can
// inspect and drive them without special-casing.
//
// Companion systems (clock advance, sequencer step, MIDI flush) live in the
// app or a dedicated music addon — not here.
// ============================================================================

namespace ecs {

// ── Transport ────────────────────────────────────────────────────────────────
// Place this on one authoritative "transport entity" per scene. Systems read
// playing/paused, BPM, and song position from here.

struct transport_control_component {
    bool     playing          {false};
    bool     paused           {false};

    double   bpm              {120.0};  // 20–300
    int      timeSigNum       {4};
    int      timeSigDen       {4};

    double   songPositionBeats {0.0};
    uint64_t barIndex          {0};
    int      beatInBar         {0};

    bool     loopEnabled       {false};
    double   loopStartBeats    {0.0};
    double   loopEndBeats      {4.0};

    void play()
    {
        playing = true;
        paused  = false;
    }

    void stop()
    {
        playing           = false;
        paused            = false;
        songPositionBeats = 0.0;
        barIndex          = 0;
        beatInBar         = 0;
    }

    void pause()
    {
        if (playing) paused = !paused;
    }

    void seek(double beats)
    {
        songPositionBeats = beats < 0.0 ? 0.0 : beats;
        barIndex  = static_cast<uint64_t>(songPositionBeats / timeSigNum);
        beatInBar = static_cast<int>(std::fmod(songPositionBeats, timeSigNum));
    }

    // Advance by dt seconds — call from update() on the main thread.
    void update(double dt)
    {
        if (!playing || paused) return;
        double beatDuration = 60.0 / bpm;
        songPositionBeats += dt / beatDuration;
        barIndex  = static_cast<uint64_t>(songPositionBeats / timeSigNum);
        beatInBar = static_cast<int>(std::fmod(songPositionBeats, timeSigNum));
        if (loopEnabled && songPositionBeats >= loopEndBeats)
            seek(loopStartBeats);
    }
};

// ── Clock ────────────────────────────────────────────────────────────────────
// Fine-grained tick accumulator. One entity holds this; sequencer_components
// point to it via clockSource. A ClockSystem advances phaseTicks each frame.

struct clock_component {
    double   bpm             {120.0};
    int      ppq             {96};       // pulses per quarter note

    double   phaseTicks      {0.0};      // fractional running tick count
    double   samplesPerTick  {0.0};      // informational; set by audio setup

    float    swingAmount     {0.0f};     // 0=straight, 1=max swing
    int      swingSubdiv     {2};        // subdivision for swing (2 = 8th-note swing)

    bool     externalSync    {false};    // true = external system writes phaseTicks / beat / phase

    // Direct Link / external beat fields (written by ofxAbletonLinkKit when externalSync=true)
    double   beat            {0.0};      // beat position within session (from Link beatAtTime)
    double   phase           {0.0};      // phase within quantum (0..quantum)
    double   quantum         {4.0};      // Link quantum (bars worth of beats)

    uint64_t barCount        {0};
    int      beatInBar       {0};
    int      tickInBeat      {0};

    // Advance by dt seconds. Call from a ClockSystem or ofApp::update().
    // Returns number of whole ticks elapsed this frame (useful for step fires).
    // When externalSync=true, phaseTicks/beat/phase are written externally;
    // this method still updates barCount/beatInBar/tickInBeat from phaseTicks.
    int advance(double dt)
    {
        int elapsed = 0;
        if (!externalSync) {
            double ticksPerSec = (bpm / 60.0) * ppq;
            double prev = phaseTicks;
            phaseTicks += dt * ticksPerSec;
            elapsed = static_cast<int>(phaseTicks) - static_cast<int>(prev);
        }
        // Always refresh derived position fields from current phaseTicks
        int ticksPerBeat = ppq;
        int ticksPerBar  = ticksPerBeat * 4;
        tickInBeat = static_cast<int>(phaseTicks) % ticksPerBeat;
        beatInBar  = (static_cast<int>(phaseTicks) / ticksPerBeat) % 4;
        barCount   = static_cast<uint64_t>(phaseTicks) / ticksPerBar;
        return elapsed;
    }

    // Write beat/phase/bpm from an external sync source (e.g. Ableton Link).
    // Converts Link beat position to phaseTicks so the rest of the system is unaffected.
    void syncFromLink(double linkBeat, double linkPhase, double linkBpm, double linkQuantum)
    {
        beat     = linkBeat;
        phase    = linkPhase;
        bpm      = linkBpm;
        quantum  = linkQuantum;
        // Convert beat → ticks (beat * ppq = tick position within session)
        phaseTicks = linkBeat * ppq;
    }

    void reset()
    {
        phaseTicks = 0.0;
        beat       = 0.0;
        phase      = 0.0;
        barCount   = 0;
        beatInBar  = 0;
        tickInBeat = 0;
    }
};

// ── Sequencer ────────────────────────────────────────────────────────────────
// Backend-agnostic step state. The actual step data (notes, velocities) lives
// in the instrument addon (tb303::Pattern, tr808::Pattern etc). This component
// is the shared "where are we / is it playing" record that any system can read.

struct sequencer_component {
    int  numSteps          {16};
    int  currentStep       {0};

    bool playing           {false};
    bool quantizedStart    {false};

    double nextTriggerTick {0.0};   // next tick at which the step should fire
    entt::entity clockSource {entt::null};  // entity with clock_component to follow

    // Fired each time the step advances — wire to tb303::Sequencer::onStep etc.
    std::function<void(int step)> onStep;

    void reset()
    {
        currentStep      = 0;
        nextTriggerTick  = 0.0;
    }
};

// ── Pattern ──────────────────────────────────────────────────────────────────
// Lightweight metadata component. The heavy step data lives outside ECS (in
// tb303::Pattern etc.) and is identified by revision so systems detect edits.

struct pattern_component {
    std::string name      {"Pattern"};
    int  numSteps         {16};
    float bpm             {120.0f};  // default; overridden by clock_component
    uint32_t revision     {0};       // bump when steps change so systems refresh

    int  rootNote         {60};      // MIDI note (default C4)
    int  scale            {0};       // 0=chromatic, 1=major, 2=minor, 3=pentatonic, 4=blues
};

// ── Note Event ───────────────────────────────────────────────────────────────
// Represents a single discrete note/CC message as a fire-and-forget entity.
// Create it, let the MIDI output system flush it, then destroy it.

enum class NoteEventKind : uint8_t {
    NoteOn    = 0,
    NoteOff   = 1,
    CC        = 2,
    PitchBend = 3,
};

struct note_event_component {
    NoteEventKind kind        {NoteEventKind::NoteOn};
    double  startTick         {0.0};
    float   durationTicks     {0.0f};   // 0 = instantaneous (NoteOff handled separately)
    uint8_t channel           {1};
    uint8_t note              {60};
    uint8_t velocity          {100};
    entt::entity sourceEntity {entt::null};
};

// ── MIDI Output ──────────────────────────────────────────────────────────────
// Pure data MIDI sink: accumulate raw 3-byte messages in pending[], flush each
// frame via your MIDI library. Does NOT include ofxMidi headers here so this
// component stays library-agnostic.

struct midi_output_component {
    std::string portName   {};
    bool        isOpen     {false};

    std::vector<std::array<uint8_t, 3>> pending;

    void sendNoteOn(uint8_t note, uint8_t vel, uint8_t ch = 1)
    {
        pending.push_back({static_cast<uint8_t>(0x90 | ((ch - 1) & 0x0F)), note, vel});
    }

    void sendNoteOff(uint8_t note, uint8_t ch = 1)
    {
        pending.push_back({static_cast<uint8_t>(0x80 | ((ch - 1) & 0x0F)), note, 0});
    }

    void sendCC(uint8_t cc, uint8_t val, uint8_t ch = 1)
    {
        pending.push_back({static_cast<uint8_t>(0xB0 | ((ch - 1) & 0x0F)), cc, val});
    }

    void flush() { pending.clear(); }
};

} // namespace ecs
