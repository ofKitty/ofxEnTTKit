#pragma once

#include "base_system.h"
#include "../components/layer_components.h"
#include <vector>
#include <algorithm>

namespace ecs {

// ============================================================================
// LayerSystem
// ============================================================================
// Maintains sorted, visibility-filtered views over all layer entities each
// frame.  Register this system FIRST in SystemManager so that rendering and
// domain systems can consume the pre-sorted lists without re-querying and
// re-sorting independently.
//
// Sorting criterion: ecs::layer_component::index ascending
//   (lower index = bottom of the stack / drawn first).
//
// Actual rendering is NOT this system's responsibility.  Consumers call
// visibleLayers() or allLayers() after SystemManager::update() returns.
//
// Usage:
//   auto* layers = systems.addSystem<ecs::LayerSystem>();
//
//   // In draw / render:
//   for (entt::entity e : layers->visibleLayers()) {
//       auto& lc = registry.get<ecs::layer_component>(e);
//       // ... render entities belonging to this layer ...
//   }
// ============================================================================

class LayerSystem : public ISystem {
public:
    const char* getName() const override { return "LayerSystem"; }

    /// Rebuilds the sorted layer lists from the current registry state.
    /// Called automatically by SystemManager::update().
    void update(entt::registry& registry, float deltaTime) override;

    /// Layer entities with visible == true, sorted by layer_component::index.
    const std::vector<entt::entity>& visibleLayers() const { return m_visible; }

    /// All layer entities sorted by layer_component::index (visible + hidden).
    const std::vector<entt::entity>& allLayers() const { return m_all; }

private:
    std::vector<entt::entity> m_visible;
    std::vector<entt::entity> m_all;
};

} // namespace ecs
