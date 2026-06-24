#pragma once

// ============================================================================
// ecs::Entity — ergonomic entity handle (header-only)
// ============================================================================
// A lightweight {registry*, entity} wrapper that makes ECS sketches read as
// naturally as classic openFrameworks code. It is NOT required — every method
// is a thin pass-through to entt::registry, so raw entt usage stays valid.
//
//   ecs::Entity stroke = ecs::spawn(reg, "Stroke");
//   stroke.add<ecs::path_component>().path.moveTo(50, 80);
//   if (stroke.has<ecs::path_component>()) stroke.get<ecs::path_component>();
//
// Unlike ofxNode (which is a scene-graph node: Relationship + transforms),
// ecs::Entity carries no components of its own — it just wraps an id so you can
// attach whatever components you want with friendly syntax.
// ============================================================================

#include <entt.hpp>
#include "components/base_components.h"   // ecs::tag_component

#include <string>
#include <utility>

namespace ecs {

class Entity {
public:
    Entity() = default;
    Entity(entt::registry& reg, entt::entity e) : reg_(&reg), e_(e) {}

    /// Drops into any entt API expecting an entity id.
    operator entt::entity() const { return e_; }

    entt::entity    id()       const { return e_; }
    entt::registry& registry() const { return *reg_; }
    bool            valid()    const { return reg_ && reg_->valid(e_); }
    explicit operator bool()   const { return valid(); }

    /// Add (or replace) a component and return a reference to it.
    template <class T, class... Args>
    T& add(Args&&... args) {
        return reg_->emplace_or_replace<T>(e_, std::forward<Args>(args)...);
    }

    /// Add (or replace) a component and return the handle for chaining.
    template <class T, class... Args>
    Entity& with(Args&&... args) {
        reg_->emplace_or_replace<T>(e_, std::forward<Args>(args)...);
        return *this;
    }

    template <class T>       T& get()       { return reg_->get<T>(e_); }
    template <class T> const T& get() const { return reg_->get<T>(e_); }

    template <class T> T* tryGet() { return reg_->try_get<T>(e_); }

    template <class T> bool has() const { return reg_->all_of<T>(e_); }

    template <class T> void remove() { reg_->remove<T>(e_); }

    void destroy() {
        if (valid()) reg_->destroy(e_);
        e_ = entt::null;
    }

private:
    entt::registry* reg_ {nullptr};
    entt::entity    e_   {entt::null};
};

/// Create a bare entity handle.
inline Entity make(entt::registry& reg) {
    return Entity(reg, reg.create());
}

/// Create an entity with an optional tag_component name — the friendly default
/// for "give me a named thing I can hang components on".
inline Entity spawn(entt::registry& reg, const std::string& name = "") {
    Entity e = make(reg);
    if (!name.empty()) e.add<tag_component>(name);
    return e;
}

} // namespace ecs
