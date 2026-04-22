#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/rendering_components.h"
#include "../components/base_components.h"

namespace ecs {

// ============================================================================
// Trail System
// ============================================================================
// Handles updating and rendering of trail components.
// Components are pure data - this system contains the logic.

class TrailSystem : public ISystem {
public:
    const char* getName() const override { return "TrailSystem"; }
    
    void update(entt::registry& registry, float deltaTime) override;
    void draw(entt::registry& registry) override;
    
    // Set the camera for ribbon rendering (must be called before draw)
    void setCamera(ofCamera* cam) { m_camera = cam; }
    
    // Individual functions
    static void addTrailPoint(trail_component& comp, const glm::vec3& position);
    static void updateTrail(trail_component& comp, float dt);
    static void drawTrailRibbon(const trail_component& comp, const ofCamera& camera);
    static void clearTrail(trail_component& comp);

private:
    ofCamera* m_camera = nullptr;
};

} // namespace ecs
