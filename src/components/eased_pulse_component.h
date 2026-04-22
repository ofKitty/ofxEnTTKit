#pragma once
#include "ofMain.h"
#include <entt.hpp>
#include <vector>
#include "easing.h"
#include "modulator_component.h"  // For mod_binding_component

// ============================================================================
// EASED PULSE COMPONENT - Square wave with configurable easing transitions
// ============================================================================
// A modulation source that generates a pulse/square wave with smooth
// rise and fall transitions using various easing functions.
// Perfect for ESP32 LED animations with smooth state changes.
// ============================================================================

namespace ecs {

// ============================================================================
// Eased Pulse Component
// ============================================================================

enum class PulsePhase {
    High,       // At maximum value
    Falling,    // Transitioning from high to low
    Low,        // At minimum value
    Rising      // Transitioning from low to high
};

struct eased_pulse_component {
    // Timing
    float frequency = 1.0f;         // Hz (cycles per second)
    float dutyCycle = 0.5f;         // 0-1 (ratio of high time to period)
    
    // Easing for transitions
    float riseTime = 0.1f;          // Seconds for low->high transition
    float fallTime = 0.1f;          // Seconds for high->low transition
    EasingType riseEasing = EasingType::EaseOutQuad;
    EasingType fallEasing = EasingType::EaseInQuad;
    
    // Output range
    float minValue = 0.0f;
    float maxValue = 1.0f;
    
    // Runtime state
    float value = 0.0f;             // Current output value
    float time = 0.0f;              // Accumulated time in current cycle
    PulsePhase phase = PulsePhase::Low;
    float transitionProgress = 0.0f; // 0-1 progress through current transition
    bool playing = true;
    
    // Bindings (same pattern as modulator_component)
    std::vector<mod_binding_component> bindings;
    
    eased_pulse_component() = default;
    
    eased_pulse_component(float freq, float duty, float rise, float fall)
        : frequency(freq)
        , dutyCycle(duty)
        , riseTime(rise)
        , fallTime(fall)
    {}
    
    // Calculate timing for one cycle
    float getPeriod() const { return 1.0f / frequency; }
    float getHighTime() const { return getPeriod() * dutyCycle - riseTime; }
    float getLowTime() const { return getPeriod() * (1.0f - dutyCycle) - fallTime; }
    
    // Update the pulse generator
    void update(float dt);
    
    // Get normalized value (0-1)
    float getNormalizedValue() const {
        return (value - minValue) / (maxValue - minValue + 0.0001f);
    }
    
    // Apply current value to all bindings
    void applyBindings();
    
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
    void clearBindings() { bindings.clear(); }
    
    // Control methods
    void play() { playing = true; }
    void pause() { playing = false; }
    void reset();
    
    // Trigger immediate transition
    void triggerRise() {
        phase = PulsePhase::Rising;
        transitionProgress = 0.0f;
    }
    
    void triggerFall() {
        phase = PulsePhase::Falling;
        transitionProgress = 0.0f;
    }
};

} // namespace ecs
