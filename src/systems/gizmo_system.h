#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/utility_components.h"
#include "../components/base_components.h"

namespace ecs {

// ============================================================================
// Gizmo System
// ============================================================================
// Handles rendering and interaction of transform gizmos.
// Components are pure data - this system contains the gizmo logic.

class GizmoSystem : public ISystem {
public:
    const char* getName() const override { return "GizmoSystem"; }
    
    void draw(entt::registry& registry) override;
    
    // Draw a gizmo at a specific position
    static void drawGizmo(const gizmo_component& comp, const glm::vec3& position);

private:
    static void drawTranslateGizmo(const gizmo_component& comp);
    static void drawRotateGizmo(const gizmo_component& comp);
    static void drawScaleGizmo(const gizmo_component& comp);
};

} // namespace ecs
