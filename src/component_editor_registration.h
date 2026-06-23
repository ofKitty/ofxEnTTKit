#pragma once

// ============================================================================
// component_editor_registration — ecs component picker registry
// ============================================================================
// EnTT types need no signup to use at runtime. This module is the single
// registry for “Add Component” picker rows (labels + has / add / remove hooks).
//
// Shipped ofxEnTTKit types register during finalizeComponentMenu(). Addons
// extend the same registry via ecs::registerComponent(). UI shells (ofxKit)
// read ecs::componentMenuEntries() — they do not own registration.
//
// ofxEnTTInspector stays type-driven: this list only drives editor pickers,
// not property widgets.
// ============================================================================

#include <entt.hpp>
#include <functional>
#include <string>
#include <vector>

namespace ecs {

/// One selectable row in an “Add Component” menu.
struct ComponentMenuEntry {
    std::string name;
    std::string category;
    std::string description;

    std::function<bool(entt::registry&, entt::entity)> has;
    std::function<void(entt::registry&, entt::entity)> add;
    std::function<void(entt::registry&, entt::entity)> remove;
};

/// Queue a picker row (built-in kit types or addon backends).
void registerComponentMenuEntry(ComponentMenuEntry entry);

/// Shorthand — generates has / remove from T; optional custom add.
template<typename T>
void registerComponent(const char* name,
                       const char* category,
                       std::function<void(entt::registry&, entt::entity)> add = {})
{
    ComponentMenuEntry d;
    d.name     = name;
    d.category = category;
    d.has      = [](entt::registry& r, entt::entity e) { return r.all_of<T>(e); };
    d.add      = add ? std::move(add)
                     : [](entt::registry& r, entt::entity e) {
                           if (!r.all_of<T>(e)) r.emplace<T>(e);
                       };
    d.remove = [](entt::registry& r, entt::entity e) { r.remove<T>(e); };
    registerComponentMenuEntry(std::move(d));
}

/// Build the registry once (shipped kit rows + queued addon rows). Idempotent.
void finalizeComponentMenu();

/// All picker rows in registration order. Calls finalizeComponentMenu() if needed.
const std::vector<ComponentMenuEntry>& componentMenuEntries();

/// Unique category names in registration order.
std::vector<std::string> componentMenuCategories();

} // namespace ecs
