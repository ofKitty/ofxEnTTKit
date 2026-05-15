#include "layer_system.h"
#include "../components/hierarchy_components.h"

namespace ecs {

void LayerSystem::update(entt::registry& registry, float /*deltaTime*/)
{
    m_all.clear();
    m_visible.clear();

    // Locate the first root layer: has layer_component + Relationship, and
    // Relationship.parent == null AND Relationship.prev_sibling == null.
    entt::entity firstRoot = entt::null;
    auto view = registry.view<layer_component, Relationship>();
    for (auto e : view) {
        auto& rel = registry.get<Relationship>(e);
        if (rel.parent == entt::null && rel.prev_sibling == entt::null) {
            firstRoot = e;
            break;
        }
    }

    if (firstRoot == entt::null) {
        // Fallback: no Relationship component, just collect all layer entities
        for (auto e : registry.view<layer_component>())
            m_all.push_back(e);
        std::sort(m_all.begin(), m_all.end(),
            [&registry](entt::entity a, entt::entity b) {
                return registry.get<layer_component>(a).index
                     < registry.get<layer_component>(b).index;
            });
        for (auto e : m_all)
            if (registry.get<layer_component>(e).visible)
                m_visible.push_back(e);
        return;
    }

    // DFS from root sibling chain, accumulating parent-visibility for m_visible.
    // collect(e, parentVisible):
    //   - always push to m_all
    //   - push to m_visible only when parentVisible && lc.visible
    std::function<void(entt::entity, bool)> collect =
        [&](entt::entity e, bool parentVisible)
    {
        if (!registry.valid(e)) return;
        const auto& lc  = registry.get<layer_component>(e);
        const auto& rel = registry.get<Relationship>(e);

        m_all.push_back(e);
        bool selfVisible = parentVisible && lc.visible;
        if (selfVisible) m_visible.push_back(e);

        entt::entity child = rel.first_child;
        while (child != entt::null) {
            collect(child, selfVisible);
            child = registry.get<Relationship>(child).next_sibling;
        }
    };

    entt::entity root = firstRoot;
    while (root != entt::null) {
        collect(root, true);
        root = registry.get<Relationship>(root).next_sibling;
    }
}

} // namespace ecs
