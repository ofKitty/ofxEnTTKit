#pragma once

#include "base_system.h"
#include "../components/modulator_component.h"

namespace ecs {

// ============================================================================
// Modulator System
// ============================================================================
// Updates all modulator components and applies their values to bound targets.
//
// Each frame:
//   1. Updates time for all modulators
//   2. Generates normalized 0-1 waveform values
//   3. Applies values to all bindings with depth/range mapping
//
// Usage:
//   ModulatorSystem modSystem;
//   modSystem.update(registry, deltaTime);

class ModulatorSystem : public ISystem {
public:
    const char* getName() const override { return "ModulatorSystem"; }
    
    // Update all modulators and apply bindings
    void update(entt::registry& registry, float deltaTime);
    
    // Static helper to update a single modulator
    static void updateModulator(modulator_component& mod, float deltaTime);
};

} // namespace ecs
