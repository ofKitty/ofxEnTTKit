#pragma once

#include "components/hierarchy_components.h"
#include "components/layer_components.h"
#include "components/base_components.h"

#include <entt.hpp>
#include <string>

namespace ecs {

// ============================================================================
// Relationship helpers — shared by LayersPanel, layer content API, reparent
// ============================================================================

void linkChild(entt::registry& reg, entt::entity parent, entt::entity child);
void unlinkChild(entt::registry& reg, entt::entity e);
void reparent(entt::registry& reg,
              entt::entity child,
              entt::entity newParent,
              entt::entity insertBefore = entt::null);

/// Destroy @p root and every Relationship descendant (depth-first).
void destroySubtree(entt::registry& reg, entt::entity root);

entt::entity findFirstLayerRoot(entt::registry& reg);
/// Read-only overload for const traversal (e.g. exporters).
entt::entity findFirstLayerRoot(const entt::registry& reg);

bool layerEntityVisible(entt::registry& reg, entt::entity layerEntity);
bool entityEffectivelyVisible(entt::registry& reg, entt::entity e);

std::string entityDisplayName(entt::registry& reg, entt::entity e);

// ============================================================================
// Layer — thin handle for a layer_component entity
// ============================================================================

class Layer {
public:
    Layer() = default;
    Layer(entt::registry& reg, entt::entity e) : reg_(&reg), entity_(e) {}

    bool valid() const;
    entt::entity entity() const { return entity_; }
    entt::registry& registry() const { return *reg_; }

    layer_component& component();
    const layer_component& component() const;

    std::string name() const;
    bool visible() const;
    bool locked() const;
    ofColor color() const;

private:
    entt::registry* reg_ {nullptr};
    entt::entity    entity_ {entt::null};
};

// ============================================================================
// Layer hierarchy API (membership only — no drawable-type factories)
// ============================================================================

Layer createLayer(entt::registry& reg,
                  const std::string& name = "",
                  entt::entity parentLayer = entt::null);

/// Reparent @p child under @p layer via Relationship. Does not create components.
entt::entity addToLayer(entt::registry& reg, Layer layer, entt::entity child);

} // namespace ecs
