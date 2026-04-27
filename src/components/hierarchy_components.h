#pragma once

#include <entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ecs {

// ============================================================================
// Relationship — linked-list scene-graph hierarchy
// ============================================================================
// Zero heap allocation per node. All links are entity handles.
// Use TransformSystem to propagate global transforms after modifying this.
//
// Invariants maintained by ofRNode helpers:
//   - parent != entt::null  <=>  entity is in some parent's child list
//   - first_child is the head of a doubly-linked sibling list
//   - prev_sibling == entt::null for the first child
//   - next_sibling == entt::null for the last child
// ============================================================================

struct Relationship {
    std::size_t  children_count = 0;
    entt::entity first_child    = entt::null;
    entt::entity prev_sibling   = entt::null;
    entt::entity next_sibling   = entt::null;
    entt::entity parent         = entt::null;
};

// ============================================================================
// LocalTransform — what the user sets (position/orientation/scale)
// ============================================================================

struct LocalTransform {
    glm::vec3 position    {0.f, 0.f, 0.f};
    glm::quat orientation {1.f, 0.f, 0.f, 0.f};
    glm::vec3 scale       {1.f, 1.f, 1.f};
};

// ============================================================================
// GlobalTransform — world-space matrix, written each frame by TransformSystem
// ============================================================================

struct GlobalTransform {
    glm::mat4 matrix {1.f};
};

// ============================================================================
// Helper: build a model matrix from a LocalTransform
// ============================================================================

inline glm::mat4 localToMatrix(const LocalTransform& lt)
{
    glm::mat4 m = glm::translate(glm::mat4(1.f), lt.position);
    m           *= glm::mat4_cast(lt.orientation);
    m           = glm::scale(m, lt.scale);
    return m;
}

} // namespace ecs
