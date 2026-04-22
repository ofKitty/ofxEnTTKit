#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/utility_components.h"
#include "../components/base_components.h"

namespace ecs {

// ============================================================================
// Physics System
// ============================================================================
// Handles physics simulation for rigidbody components (integration, gravity, drag).
// Does not handle collision — no collider components or collision system in ofxEnTT.
// Components are pure data - this system contains the physics logic.

class PhysicsSystem : public ISystem {
public:
    const char* getName() const override { return "PhysicsSystem"; }
    
    void update(entt::registry& registry, float deltaTime) override;
    
    // Individual update functions
    static void updateRigidbody(rigidbody_component& rb, node_component& node, float dt);
    
    // Force application helpers
    static void applyForce(rigidbody_component& rb, const glm::vec3& force);
    static void applyImpulse(rigidbody_component& rb, const glm::vec3& impulse);
    static void applyTorque(rigidbody_component& rb, const glm::vec3& torque);
    
    // Gravity setting
    void setGravity(const glm::vec3& gravity) { m_gravity = gravity; }
    glm::vec3 getGravity() const { return m_gravity; }

private:
    glm::vec3 m_gravity = glm::vec3(0, -9.81f, 0);
};

} // namespace ecs
