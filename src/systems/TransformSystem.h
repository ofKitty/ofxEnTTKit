#pragma once

#include <entt.hpp>

namespace ecs {

// ============================================================================
// TransformSystem
// ============================================================================
// Walks the Relationship tree top-down (roots first, then children) and
// writes a GlobalTransform for every entity that has both Relationship and
// LocalTransform components.
//
// Call once per frame, before any code reads GlobalTransform:
//
//   ecs::TransformSystem::update(registry);
//
// Root nodes (Relationship::parent == entt::null) use their LocalTransform
// directly. Children accumulate: GlobalTransform = parent_global * local.
// ============================================================================

class TransformSystem {
public:
    static void update(entt::registry& reg);

private:
    static void propagate(entt::registry& reg,
                          entt::entity    entity,
                          const glm::mat4& parentGlobal);
};

} // namespace ecs
