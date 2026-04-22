#include "eased_pulse_component.h"

namespace ecs {

void eased_pulse_component::update(float dt) {
    if (!playing) return;
    
    time += dt;
    
    // Calculate phase durations
    float highTime = std::max(0.0f, getHighTime());
    float lowTime = std::max(0.0f, getLowTime());
    
    // State machine
    switch (phase) {
        case PulsePhase::Rising:
            transitionProgress += dt / riseTime;
            if (transitionProgress >= 1.0f) {
                transitionProgress = 0.0f;
                phase = PulsePhase::High;
                value = maxValue;
            } else {
                value = minValue + (maxValue - minValue) * easing::apply(riseEasing, transitionProgress);
            }
            break;
            
        case PulsePhase::High:
            transitionProgress += dt / std::max(0.001f, highTime);
            value = maxValue;
            if (transitionProgress >= 1.0f) {
                transitionProgress = 0.0f;
                phase = PulsePhase::Falling;
            }
            break;
            
        case PulsePhase::Falling:
            transitionProgress += dt / fallTime;
            if (transitionProgress >= 1.0f) {
                transitionProgress = 0.0f;
                phase = PulsePhase::Low;
                value = minValue;
            } else {
                value = maxValue - (maxValue - minValue) * easing::apply(fallEasing, transitionProgress);
            }
            break;
            
        case PulsePhase::Low:
            transitionProgress += dt / std::max(0.001f, lowTime);
            value = minValue;
            if (transitionProgress >= 1.0f) {
                transitionProgress = 0.0f;
                phase = PulsePhase::Rising;
            }
            break;
    }
}

void eased_pulse_component::applyBindings() {
    float normalizedVal = getNormalizedValue();
    for (auto& binding : bindings) {
        binding.apply(normalizedVal);
    }
}

void eased_pulse_component::reset() {
    time = 0.0f;
    phase = PulsePhase::Low;
    transitionProgress = 0.0f;
    value = minValue;
}

} // namespace ecs
