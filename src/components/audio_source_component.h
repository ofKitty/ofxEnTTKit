#pragma once
#include "ofMain.h"
#include <entt.hpp>
#include <vector>
#include <array>
#include <functional>
#include "modulator_component.h"

// ============================================================================
// AUDIO SOURCE COMPONENT - Audio input with FFT analysis for modulation
// ============================================================================
// Declarations only - implementations in audio_source_component.cpp
// ============================================================================

namespace ecs {

enum class FFTBand {
    SubBass = 0, Bass, LowMid, Mid, HighMid, Presence, Brilliance, COUNT
};

struct FFTBandRange {
    float minFreq;
    float maxFreq;
    const char* name;
};

extern const FFTBandRange s_fftBandRanges[];

struct beat_detector {
    float threshold = 1.5f;
    float decay = 0.95f;
    float cooldownTime = 0.1f;
    float average = 0.0f;
    float cooldown = 0.0f;
    bool triggered = false;
    
    void update(float value, float dt);
    void reset();
};

struct audio_source_component {
    bool enabled = true;
    int deviceIndex = 0;
    int bufferSize = 512;
    int sampleRate = 44100;
    float inputGain = 1.0f;
    float smoothing = 0.9f;
    float rawVolume = 0.0f;
    float smoothedVolume = 0.0f;
    float peakVolume = 0.0f;
    float peakDecay = 0.99f;
    
    std::array<float, (size_t)FFTBand::COUNT> bandValues = {};
    std::array<float, (size_t)FFTBand::COUNT> smoothedBandValues = {};
    std::array<beat_detector, (size_t)FFTBand::COUNT> beatDetectors;
    beat_detector volumeBeatDetector;
    std::vector<float> fftMagnitudes;
    int fftBins = 256;
    std::vector<float> leftChannel;
    std::vector<float> rightChannel;
    bool streamActive = false;
    
    std::vector<mod_binding_component> bindings;
    
    audio_source_component();
    
    void processAudioInput(const float* samples, int numFrames, int numChannels);
    void update(float dt);
    void processFFT();
    int getBinForFrequency(float freq) const;
    float getMagnitudeForRange(float minFreq, float maxFreq) const;
    void updateBandsFromFFT();
    
    float getVolume() const { return smoothedVolume; }
    float getRawVolume() const { return rawVolume; }
    float getPeak() const { return peakVolume; }
    float getBand(FFTBand band) const;
    float getSubBass() const;
    float getBass() const;
    float getLowMid() const;
    float getMid() const;
    float getHighMid() const;
    float getPresence() const;
    float getBrilliance() const;
    bool isKickBeat() const;
    bool isSnareBeat() const;
    bool isHihatBeat() const;
    bool isVolumeBeat() const;
    float getOutputValue(const std::string& name) const;
    
    void bind(float* target, const std::string& outputName, float depth = 1.0f, 
              float minVal = 0.0f, float maxVal = 1.0f, bool additive = false);
    void unbind(float* target);
    void clearBindings();
    void start();
    void stop();
    void reset();
};

} // namespace ecs
