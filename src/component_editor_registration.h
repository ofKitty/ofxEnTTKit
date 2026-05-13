#pragma once

// ============================================================================
// component_editor_registration — shipped ecs::* rows for “Add Component” UIs
// ============================================================================
// This is **not** EnTT runtime registration — EnTT types need no signup to use.
// Shells such as ofxKit pass a callback; we invoke it once per shipped kit
// component with display labels plus has / add / remove hooks for the registry.
//
// ofxEnTTInspector stays type-driven: this list only drives editor pickers that
// add or remove components, not property widgets themselves.
// ============================================================================

#include <entt.hpp>
#include <functional>
#include <string>

namespace ecs {

/// One selectable row in an editor shell’s “Add Component” menu for kit types.
struct ComponentMenuEntry {
    std::string name;
    std::string category;
    std::string description;

    std::function<bool(entt::registry&, entt::entity)> has;
    std::function<void(entt::registry&, entt::entity)> add;
    std::function<void(entt::registry&, entt::entity)> remove;
};

/// Receives each ComponentMenuEntry; forward into your shell (e.g. ofxKit picker).
using ComponentMenuCallback = std::function<void(const ComponentMenuEntry&)>;

/// All default menu rows for ecs::* components shipped by ofxEnTTKit.
void registerKitComponentMenu(ComponentMenuCallback sink);

} // namespace ecs
