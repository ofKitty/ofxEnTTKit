#include "physics_system.h"

namespace ecs {
using namespace ecs;

void PhysicsSystem::update(entt::registry& registry, float deltaTime) {
	auto view = registry.view<rigidbody_component, node_component>();
	for (auto entity : view) {
		auto& rb = registry.get<rigidbody_component>(entity);
		if (rb.useBullet) continue;  // Driven by ofxBullet in the app
		auto& node = registry.get<node_component>(entity);
		updateRigidbody(rb, node, deltaTime);
	}
}

void PhysicsSystem::updateRigidbody(rigidbody_component & c_rb, node_component & c_node, float dt) {
    if (c_rb.isKinematic) return;
    
    // Apply gravity
    if (c_rb.useGravity) {
        c_rb.acceleration.y += -9.81f;
    }
    
    // Update velocity
    c_rb.velocity += c_rb.acceleration * dt;
    c_rb.angularVelocity += c_rb.angularAcceleration * dt;
    
    // Apply drag
    c_rb.velocity *= (1.0f - c_rb.drag);
    c_rb.angularVelocity *= (1.0f - c_rb.angularDrag);
    
    // Apply constraints
    if (c_rb.freezePositionX) c_rb.velocity.x = 0;
    if (c_rb.freezePositionY) c_rb.velocity.y = 0;
    if (c_rb.freezePositionZ) c_rb.velocity.z = 0;
    if (c_rb.freezeRotationX) c_rb.angularVelocity.x = 0;
    if (c_rb.freezeRotationY) c_rb.angularVelocity.y = 0;
    if (c_rb.freezeRotationZ) c_rb.angularVelocity.z = 0;
    
    // Update position via node
    glm::vec3 pos = c_node.node.getPosition();
    pos += c_rb.velocity * dt;
    c_node.node.setPosition(pos);
    
    // Update rotation via node
    glm::vec3 euler = c_node.node.getOrientationEulerDeg();
    euler += c_rb.angularVelocity * dt;
    c_node.node.setOrientation(glm::quat(glm::radians(euler)));
    
    // Reset acceleration (forces are applied each frame)
    c_rb.acceleration = glm::vec3(0, 0, 0);
    c_rb.angularAcceleration = glm::vec3(0, 0, 0);
}

void PhysicsSystem::applyForce(rigidbody_component & rb, const glm::vec3 & force) {
    if (rb.isKinematic) return;
    rb.acceleration += force / rb.mass;
}

void PhysicsSystem::applyImpulse(rigidbody_component & rb, const glm::vec3 & impulse) {
    if (rb.isKinematic) return;
    rb.velocity += impulse / rb.mass;
}

void PhysicsSystem::applyTorque(rigidbody_component & rb, const glm::vec3 & torque) {
    if (rb.isKinematic) return;
    rb.angularAcceleration += torque / rb.mass;
}

} // namespace ecs
