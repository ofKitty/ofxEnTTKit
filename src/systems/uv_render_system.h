#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/led_components.h"
#include "../components/base_components.h"

namespace ecs {

// ============================================================================
// UV Render System
// ============================================================================
// Draws UV/LED positions with sampled colors. Uses uv_component.sourceEntity
// for sampling (via LEDSystem) and optionally uv_component.drawableEntity
// for the shape to draw at each position (default: circle).

class UVRenderSystem : public ISystem {
public:
    const char* getName() const override { return "UVRenderSystem"; }

    void draw(entt::registry& registry) override;

    // Draw a single UV entity (assumes current GL matrix is already the entity's transform)
    static void drawEntity(entt::registry& registry, entt::entity entity);

    // Draw the referenced drawable entity at (x, y) with scale and current color
    static void drawDrawableAt(entt::registry& registry, entt::entity drawableEntity, float x, float y, float scale);
};

} // namespace ecs
