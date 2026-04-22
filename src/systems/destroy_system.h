#pragma once

#include "base_system.h"

namespace ecs {

// ============================================================================
// DestroySystem - Handles on_destroy for standard ofxEnTT components
// ============================================================================
// Connects on_destroy handlers for parent_component (hierarchy cleanup)
// and camera_component (active camera switching). Call setup(registry) once
// to wire the handlers. Works standalone without ofxBapp.

class DestroySystem : public ISystem {
public:
    void setup(entt::registry& registry) override;

    const char* getName() const override { return "DestroySystem"; }
};

} // namespace ecs
