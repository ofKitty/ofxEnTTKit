#include "modulator_system.h"

namespace ecs {
using namespace ecs;

void ModulatorSystem::update(entt::registry& registry, float deltaTime) {
    if (!m_enabled) return;
    
    // Get all entities with modulator components
    auto view = registry.view<modulator_component>();
    
    for (auto entity : view) {
        auto& mod = view.get<modulator_component>(entity);
        updateModulator(mod, deltaTime);
    }
}

void ModulatorSystem::updateModulator(modulator_component& mod, float deltaTime) {
    // Update the modulator's time and value
    mod.update(deltaTime);
    
    // Apply the current value to all bindings
    mod.applyBindings();
}

} // namespace ecs
