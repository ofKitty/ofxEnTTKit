#pragma once

#include <entt.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace ecs {

// ============================================================================
// Base System Interface
// ============================================================================
// Systems contain the logic that operates on components.
// Components should be pure data (POD structs).
// This follows the Entity-Component-System (ECS) pattern.

class ISystem {
public:
    virtual ~ISystem() = default;
    
    // Called once per frame during update phase
    virtual void update(entt::registry& registry, float deltaTime) {}
    
    // Called once per frame during draw phase
    virtual void draw(entt::registry& registry) {}
    
    // Called when system is first registered
    virtual void setup(entt::registry& registry) {}
    
    // Called when system is unregistered
    virtual void cleanup(entt::registry& registry) {}
    
    // System name for debugging
    virtual const char* getName() const = 0;
    
    // Enable/disable system
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

protected:
    bool m_enabled = true;
};

// ============================================================================
// System Manager
// ============================================================================
// Manages all systems and provides update/draw dispatch

class SystemManager {
public:
    template<typename T, typename... Args>
    T* addSystem(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = system.get();
        m_systems.push_back(std::move(system));
        return ptr;
    }
    
    void setup(entt::registry& registry) {
        for (auto& system : m_systems) {
            system->setup(registry);
        }
    }
    
    void update(entt::registry& registry, float deltaTime) {
        for (auto& system : m_systems) {
            if (system->isEnabled()) {
                system->update(registry, deltaTime);
            }
        }
    }
    
    void draw(entt::registry& registry) {
        for (auto& system : m_systems) {
            if (system->isEnabled()) {
                system->draw(registry);
            }
        }
    }
    
    void cleanup(entt::registry& registry) {
        for (auto& system : m_systems) {
            system->cleanup(registry);
        }
        m_systems.clear();
    }
    
    template<typename T>
    T* getSystem() {
        for (auto& system : m_systems) {
            if (auto* typed = dynamic_cast<T*>(system.get())) {
                return typed;
            }
        }
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<ISystem>> m_systems;
};

} // namespace ecs
