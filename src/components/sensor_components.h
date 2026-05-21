#pragma once

#include "ofMain.h"
#include <entt.hpp>
#include <string>

// ============================================================================
// SENSOR COMPONENTS
// ============================================================================

namespace ecs {

// DFRobot C4001 mmWave — one entity per bus (0=onboard I2C, 1=UART1, 2=UART2)
struct mmwave_c4001_component {
    int bus = 0;
    entt::entity networkDevice = entt::null;

    bool connected = false;
    bool present = false;
    bool triggered = false;
    bool wasTriggered = false;
    bool justTriggered = false;
    bool justCleared = false;
    bool approaching = false;
    bool receding = false;

    float distance = 0.0f;
    float speed = 0.0f;
    uint8_t targets = 0;

    float triggerDistanceMin = 0.0f;
    float triggerDistanceMax = 3.0f;

    void clearEdges() {
        justTriggered = false;
        justCleared = false;
    }
};

// GPIO trigger pin — one entity per pin (1–4: P1-T1, P1-T2, P2-T1, P2-T2)
struct gpio_component {
    int pin = 1;
    entt::entity networkDevice = entt::null;

    bool state = false;
    bool lastState = false;
    bool justPressed = false;
    bool justReleased = false;

    void updateState(bool newState) {
        lastState = state;
        state = newState;
        justPressed = state && !lastState;
        justReleased = !state && lastState;
    }

    void clearEdges() {
        justPressed = false;
        justReleased = false;
    }
};

} // namespace ecs
