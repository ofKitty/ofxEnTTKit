#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/utility_components.h"
#include "../components/base_components.h"

namespace ecs {

// ============================================================================
// Utility Render System
// ============================================================================
// Handles rendering of utility/debug visualization components.
// Components are pure data - this system contains the drawing logic.

class UtilityRenderSystem : public ISystem {
public:
    const char* getName() const override { return "UtilityRenderSystem"; }
    
    void draw(entt::registry& registry) override;
    
    // Individual draw functions
    static void drawGridHelper(const grid_helper_component& comp);
    static void drawBoundingBox(const bounding_box_component& comp);
};

} // namespace ecs
