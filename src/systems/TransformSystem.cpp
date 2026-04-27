#include "TransformSystem.h"
#include "components/hierarchy_components.h"

#include <glm/glm.hpp>

namespace ecs {

void TransformSystem::update(entt::registry& reg)
{
    // Process only root nodes (no parent) — recursion handles descendants.
    auto view = reg.view<ecs::Relationship, ecs::LocalTransform>();
    for (auto entity : view) {
        const auto& rel = view.get<ecs::Relationship>(entity);
        if (rel.parent != entt::null) continue; // skip non-roots

        const auto& lt = view.get<ecs::LocalTransform>(entity);
        glm::mat4 global = localToMatrix(lt);

        // Write this entity's GlobalTransform
        if (auto* gt = reg.try_get<ecs::GlobalTransform>(entity))
            gt->matrix = global;

        // Recurse into children
        propagate(reg, rel.first_child, global);
    }
}

void TransformSystem::propagate(entt::registry& reg,
                                entt::entity    entity,
                                const glm::mat4& parentGlobal)
{
    while (entity != entt::null) {
        if (!reg.valid(entity)) break;

        const auto* lt  = reg.try_get<ecs::LocalTransform>(entity);
        const auto* rel = reg.try_get<ecs::Relationship>(entity);
        if (!lt || !rel) break;

        glm::mat4 global = parentGlobal * localToMatrix(*lt);

        if (auto* gt = reg.try_get<ecs::GlobalTransform>(entity))
            gt->matrix = global;

        // Recurse into children before moving to next sibling
        propagate(reg, rel->first_child, global);

        entity = rel->next_sibling;
    }
}

} // namespace ecs
