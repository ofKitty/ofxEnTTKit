#pragma once

#include "base_system.h"
#include "../components/layer_components.h"
#include "../components/hierarchy_components.h"
#include <vector>
#include <algorithm>
#include <functional>

namespace ecs {

// ============================================================================
// LayerSystem
// ============================================================================
// Performs a depth-first traversal over the layer entity tree each frame,
// producing two lists: allLayers() (every entity in DFS order) and
// visibleLayers() (entities whose entire ancestor chain is also visible).
//
// Tree structure is encoded via ecs::Relationship (sibling linked list +
// first_child / parent pointers).  If no Relationship components exist the
// system falls back to a flat sort by layer_component::index.
//
// Register this system FIRST so that rendering and domain systems can consume
// the pre-built lists without re-traversing independently.
//
// Usage:
//   auto* layers = systems.addSystem<ecs::LayerSystem>();
//   for (entt::entity e : layers->visibleLayers()) { ... }
// ============================================================================

class LayerSystem : public ISystem {
public:
    const char* getName() const override { return "LayerSystem"; }

    /// Rebuilds the DFS layer lists from the current registry state.
    void update(entt::registry& registry, float deltaTime) override;

    /// Entities that are effectively visible (self + all ancestors visible),
    /// in DFS order (root sibling chain first, children interleaved).
    const std::vector<entt::entity>& visibleLayers() const { return m_visible; }

    /// All layer entities in DFS order (visible + hidden).
    const std::vector<entt::entity>& allLayers() const { return m_all; }

private:
    std::vector<entt::entity> m_visible;
    std::vector<entt::entity> m_all;
};

} // namespace ecs
