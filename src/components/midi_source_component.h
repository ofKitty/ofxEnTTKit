#pragma once
#include "ofMain.h"
#include <entt.hpp>
#include <vector>
#include <array>
#include <map>
#include "modulator_component.h"

// ============================================================================
// MIDI SOURCE COMPONENT - MIDI input for modulation
// ============================================================================
// Declarations only - implementations in midi_source_component.cpp
// ============================================================================

namespace ecs {

enum class MidiMessageType {
    NoteOn, NoteOff, ControlChange, PitchBend, Aftertouch, ChannelPressure, ProgramChange
};

struct midi_note_state {
    bool active = false;
    int note = 0;
    int velocity = 0;
    int channel = 0;
    float age = 0.0f;
    
    void noteOn(int n, int vel, int ch);
    void noteOff();
    float getNormalizedVelocity() const { return velocity / 127.0f; }
};

struct midi_cc_mapping {
    int ccNumber = 0;
    int channel = -1;
    float value = 0.0f;
    float smoothing = 0.9f;
    float rawValue = 0.0f;
    bool learned = false;
    
    midi_cc_mapping() = default;
    midi_cc_mapping(int cc, int ch = -1) : ccNumber(cc), channel(ch) {}
    void setValue(int val) { rawValue = val / 127.0f; }
    void update() { value = value * smoothing + rawValue * (1.0f - smoothing); }
};

struct midi_source_component {
    bool enabled = true;
    int deviceIndex = 0;
    int listenChannel = -1;
    
    std::array<float, 128> ccValues = {};
    std::array<float, 128> smoothedCCValues = {};
    float ccSmoothing = 0.9f;
    std::map<std::string, midi_cc_mapping> namedMappings;
    std::array<midi_note_state, 128> noteStates = {};
    int lastNoteOn = -1;
    int lastVelocity = 0;
    int activeNoteCount = 0;
    float pitchBend = 0.0f;
    float smoothedPitchBend = 0.0f;
    float aftertouch = 0.0f;
    float smoothedAftertouch = 0.0f;
    float modWheel = 0.0f;
    float smoothedModWheel = 0.0f;
    float expression = 0.0f;
    float smoothedExpression = 0.0f;
    bool learnMode = false;
    std::string learnTargetName;
    bool streamActive = false;
    std::vector<mod_binding_component> bindings;
    
    midi_source_component();
    
    void processNoteOn(int channel, int note, int velocity);
    void processNoteOff(int channel, int note);
    void processCC(int channel, int cc, int value);
    void processPitchBend(int channel, int value);
    void processAftertouch(int channel, int value);
    void processPolyAftertouch(int channel, int note, int value);
    void update(float dt);
    
    float getCC(int ccNumber) const;
    float getRawCC(int ccNumber) const;
    float getNamedValue(const std::string& name) const;
    float getModWheel() const { return smoothedModWheel; }
    float getExpression() const { return smoothedExpression; }
    float getPitchBend() const { return smoothedPitchBend; }
    float getAftertouch() const { return smoothedAftertouch; }
    float getLastVelocity() const { return lastVelocity / 127.0f; }
    float getLastNote() const { return lastNoteOn / 127.0f; }
    float getNoteCount() const { return activeNoteCount / 10.0f; }
    bool isNoteActive(int note) const;
    float getOutputValue(const std::string& name) const;
    
    void startLearn(const std::string& targetName);
    void cancelLearn();
    void addMapping(const std::string& name, int ccNumber, int channel = -1);
    void removeMapping(const std::string& name);
    void bind(float* target, float depth = 1.0f, float minVal = 0.0f, float maxVal = 1.0f, bool additive = false);
    void unbind(float* target);
    void clearBindings();
    void start();
    void stop();
    void reset();
};

} // namespace ecs
