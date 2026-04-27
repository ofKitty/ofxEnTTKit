#pragma once
#include "ofMain.h"
#include <entt.hpp>
#include <vector>
#include <cmath>

#ifndef TWO_PI
#define TWO_PI (3.14159265358979323846f * 2.0f)
#endif

// Forward declare

// ============================================================================
// MODULATOR COMPONENTS
// ============================================================================

namespace ecs {

// Waveform types for modulator
enum eModWaveType {
    MOD_SIN = 0,
    MOD_SQUARE,
    MOD_TRIANGLE,
    MOD_SAW,
    MOD_RAMP,
    MOD_NOISE,         // Random noise (new value each cycle)
    MOD_SMOOTH_NOISE,  // Smoothly interpolated noise
    MOD_SH,            // Sample & Hold (random steps)
    MOD_COUNT
};

// ============================================================================
// Mod Binding - connects a modulator to a target float value
// ============================================================================

struct mod_binding_component {
    float* target = nullptr;     // Pointer to target float
    float depth = 1.0f;          // Attenuator: 0 = no effect, 1 = full effect
    float min = 0.0f;            // Output range minimum
    float max = 1.0f;            // Output range maximum
    float baseValue = 0.0f;      // Original value (captured on bind)
    bool additive = false;       // true = baseValue + modulation, false = replace
    
    mod_binding_component() = default;
    
    mod_binding_component(float* t, float d = 1.0f, float minVal = 0.0f, float maxVal = 1.0f, bool add = false)
        : target(t)
        , depth(d)
        , min(minVal)
        , max(maxVal)
        , additive(add)
    {
        if (target) {
            baseValue = *target;
        }
    }
    
    // Apply modulator value (0-1) to target with depth and range mapping
    void apply(float modValue) {
        if (!target) return;
        
        // Map 0-1 modValue to min-max range, scaled by depth
        float range = max - min;
        float center = (min + max) * 0.5f;
        float mapped = center + (modValue - 0.5f) * range * depth;
        
        if (additive) {
            *target = baseValue + mapped - center;
        } else {
            *target = mapped;
        }
    }
    
    // Check if binding is valid
    bool isValid() const { return target != nullptr; }
};

// ============================================================================
// Modulator Component - generates normalized 0-1 oscillating signal
// ============================================================================

struct modulator_component {
    // Waveform settings
    eModWaveType waveType = MOD_SIN;
    float frequency = 1.0f;      // Hz (cycles per second)
    float phase = 0.0f;          // Phase offset (0-1)
    
    // Runtime state
    float time = 0.0f;           // Accumulated time
    float value = 0.0f;          // Current output (always 0-1)
    bool playing = true;         // Whether modulator is active
    
    // Noise state
    float noiseValue = 0.5f;     // Current noise target
    float noisePrev = 0.5f;      // Previous noise value (for interpolation)
    float lastCycle = 0.0f;      // Track cycle for S&H
    
    // Bindings to target values
    std::vector<mod_binding_component> bindings;
    
    modulator_component()
        : waveType(MOD_SIN)
        , frequency(1.0f)
        , phase(0.0f)
        , time(0.0f)
        , value(0.0f)
        , playing(true)
    {}
    
    // Generate normalized 0-1 waveform value
    float generate() {
        float t = fmod(time * frequency + phase, 1.0f);
        if (t < 0) t += 1.0f;
        
        switch (waveType) {
            case MOD_SIN:
                return (sin(t * TWO_PI) + 1.0f) * 0.5f;
                
            case MOD_SQUARE:
                return t < 0.5f ? 1.0f : 0.0f;
                
            case MOD_TRIANGLE:
                return t < 0.5f ? (t * 2.0f) : (2.0f - t * 2.0f);
                
            case MOD_SAW:
                return t;
                
            case MOD_RAMP:
                return 1.0f - t;
                
            case MOD_NOISE:
                // Pure random noise
                return (float)rand() / (float)RAND_MAX;
                
            case MOD_SMOOTH_NOISE:
                // Smoothly interpolated between noise values
                return noisePrev + (noiseValue - noisePrev) * t;
                
            case MOD_SH:
                // Sample & Hold - stays at noiseValue until next cycle
                return noiseValue;
                
            default:
                return 0.5f;
        }
    }
    
    // Update time and generate new value
    void update(float dt) {
        if (!playing) return;
        
        //float prevTime = time;
        time += dt;
        
        // Check for cycle boundary (for noise types)
        float currentCycle = floor(time * frequency + phase);
        if (currentCycle != lastCycle) {
            lastCycle = currentCycle;
            noisePrev = noiseValue;
            noiseValue = (float)rand() / (float)RAND_MAX;
        }
        
        value = generate();
    }
    
    // Apply current value to all bindings
    void applyBindings() {
        for (auto& binding : bindings) {
            binding.apply(value);
        }
    }
    
    // Helper to add a new binding
    void bind(float* target, float depth = 1.0f, float minVal = 0.0f, float maxVal = 1.0f, bool additive = false) {
        bindings.emplace_back(target, depth, minVal, maxVal, additive);
    }
    
    // Remove all bindings to a specific target
    void unbind(float* target) {
        bindings.erase(
            std::remove_if(bindings.begin(), bindings.end(),
                [target](const mod_binding_component& b) { return b.target == target; }),
            bindings.end()
        );
    }
    
    // Clear all bindings
    void clearBindings() {
        bindings.clear();
    }
    
    // Control methods
    void play() { playing = true; }
    void pause() { playing = false; }
    void reset() { time = 0.0f; value = generate(); }
};


} // namespace ecs
