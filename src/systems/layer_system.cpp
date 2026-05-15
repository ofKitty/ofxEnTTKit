#include "layer_system.h"

namespace ecs {

void LayerSystem::update(entt::registry& registry, float /*deltaTime*/)
{
    m_all.clear();
    m_visible.clear();

    auto view = registry.view<layer_component>();
    m_all.reserve(registry.storage<layer_component>().size());
    for (auto e : view)
        m_all.push_back(e);

    // Sort ascending by index (lower = bottom layer / drawn first)
    std::sort(m_all.begin(), m_all.end(),
        [&registry](entt::entity a, entt::entity b) {
            return registry.get<layer_component>(a).index
                 < registry.get<layer_component>(b).index;
        });

    for (auto e : m_all) {
        if (registry.get<layer_component>(e).visible)
            m_visible.push_back(e);
    }
}

} // namespace ecs
