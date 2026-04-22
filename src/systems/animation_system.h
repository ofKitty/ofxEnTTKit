#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/animation_components.h"

namespace ecs {

// ============================================================================
// Animation System
// ============================================================================
// Handles tweens, particles, and animation effects.
// Components are pure data - this system contains the update/draw logic.

class AnimationSystem : public ISystem {
public:
    const char* getName() const override { return "AnimationSystem"; }
    
    void update(entt::registry& registry, float deltaTime) override;
    void draw(entt::registry& registry) override;
    
    // Tween functions
    static void updateTween(tween_component& comp, float dt);
    static float ease(float t, tween_component::EasingType type);
    
    // Particle functions
    static void updateParticles(particle_emitter_component& comp, float dt);
    static void emitParticles(particle_emitter_component& comp, int count);
    static void drawParticles(const particle_emitter_component& comp);
    
    // Post-FX functions
    static void applyPostFX(postfx_component& comp);
};

} // namespace ecs
