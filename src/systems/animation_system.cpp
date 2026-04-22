#include "animation_system.h"

namespace ecs {
using namespace ecs;

void AnimationSystem::update(entt::registry& registry, float deltaTime) {
    // Update all tweens
    auto tweens = registry.view<tween_component>();
    for (auto entity : tweens) {
        auto& tc = registry.get<tween_component>(entity);
        updateTween(tc, deltaTime);
    }
    
    // Update all particle emitters
    auto emitters = registry.view<particle_emitter_component>();
    for (auto entity : emitters) {
        auto& pe = registry.get<particle_emitter_component>(entity);
        updateParticles(pe, deltaTime);
    }
}

void AnimationSystem::draw(entt::registry& registry) {
    // Draw all particle emitters
    auto emitters = registry.view<particle_emitter_component>();
    for (auto entity : emitters) {
		auto & pe = registry.get<particle_emitter_component>(entity);
        drawParticles(pe);
    }
}

// ============================================================================
// Tween Update
// ============================================================================

void AnimationSystem::updateTween(tween_component & comp, float dt) {
    if (!comp.playing) return;
    
    comp.elapsed += dt;
    
    if (comp.elapsed >= comp.duration) {
        comp.current = comp.to;
        
        if (comp.loop) {
            comp.elapsed = 0.0f;
            comp.current = comp.from;
        } else if (comp.pingpong) {
            std::swap(comp.from, comp.to);
            comp.elapsed = 0.0f;
            comp.current = comp.from;
        } else {
            comp.playing = false;
            if (comp.onComplete) comp.onComplete();
        }
    } else {
        float t = comp.elapsed / comp.duration;
        float eased = ease(t, comp.easing);
        comp.current = comp.from + (comp.to - comp.from) * eased;
    }
    
    if (comp.onUpdate) comp.onUpdate(comp.current);
}

// ============================================================================
// Easing Functions - delegates to tween::ease()
// ============================================================================

float AnimationSystem::ease(float t, tween_component::EasingType type) {
	// TODO: THIS IS NOT RIGHT
	return tween_component::ease(t, type);
}

// ============================================================================
// Particle Emission - delegates to particle_emitter::emit()
// ============================================================================

void AnimationSystem::emitParticles(particle_emitter_component & comp, int count) {
    comp.emit(count);
}

// ============================================================================
// Particle Update
// ============================================================================

void AnimationSystem::updateParticles(particle_emitter_component & comp, float dt) {
    if (!comp.playing) return;
    
    // Emit new particles
    comp.emitAccumulator += dt;
    float emitInterval = 1.0f / comp.emitRate;
    while (comp.emitAccumulator >= emitInterval) {
        comp.emit(1);
        comp.emitAccumulator -= emitInterval;
    }
    
    // Update existing particles
    for (auto& p : comp.particles) {
        if (!p.isAlive()) continue;
        
        // Apply forces
        p.acceleration = comp.gravity;
        p.velocity *= comp.damping;
        
        // Update
        p.update(dt);
        
        // Fade color
        float t = 1.0f - (p.life / p.maxLife);
        p.color = comp.colorStart.getLerped(comp.colorEnd, t);
    }
    
    // Remove dead particles
    comp.particles.erase(
        std::remove_if(comp.particles.begin(), comp.particles.end(),
			[](const particle_emitter_component::Particle & p) { return !p.isAlive(); }),
        comp.particles.end()
    );
}

// ============================================================================
// Particle Drawing
// ============================================================================

void AnimationSystem::drawParticles(const particle_emitter_component & comp) {
    for (const auto& p : comp.particles) {
        if (p.isAlive()) {
            ofPushStyle();
            ofSetColor(p.color);
            ofDrawCircle(p.position, p.size);
            ofPopStyle();
        }
    }
}

// ============================================================================
// Post-FX
// ============================================================================

void AnimationSystem::applyPostFX(postfx_component & comp) {
    if (!comp.enabled || comp.effectChain.empty()) return;
    
    // TODO: Implement actual shader-based post-processing
    // Each effect would need its own shader
}

} // namespace ecs
