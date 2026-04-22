#include "animation_components.h"

namespace ecs {

// ============================================================================
// tween_component::ease - Static easing function
// ============================================================================

float tween_component::ease(float t, EasingType type) {
    switch (type) {
        case LINEAR: return t;
        
        // Quadratic
        case EASE_IN_QUAD: return t * t;
        case EASE_OUT_QUAD: return t * (2 - t);
        case EASE_IN_OUT_QUAD: return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
        
        // Cubic
        case EASE_IN_CUBIC: return t * t * t;
        case EASE_OUT_CUBIC: { float t1 = t - 1; return t1 * t1 * t1 + 1; }
        case EASE_IN_OUT_CUBIC: return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
        
        // Quartic
        case EASE_IN_QUART: return t * t * t * t;
        case EASE_OUT_QUART: { float t1 = t - 1; return 1 - t1 * t1 * t1 * t1; }
        case EASE_IN_OUT_QUART: { float t1 = t - 1; return t < 0.5f ? 8 * t * t * t * t : 1 - 8 * t1 * t1 * t1 * t1; }
        
        // Quintic
        case EASE_IN_QUINT: return t * t * t * t * t;
        case EASE_OUT_QUINT: { float t1 = t - 1; return 1 + t1 * t1 * t1 * t1 * t1; }
        case EASE_IN_OUT_QUINT: { float t1 = t - 1; return t < 0.5f ? 16 * t * t * t * t * t : 1 + 16 * t1 * t1 * t1 * t1 * t1; }
        
        // Sine
        case EASE_IN_SINE: return 1 - cos(t * PI / 2);
        case EASE_OUT_SINE: return sin(t * PI / 2);
        case EASE_IN_OUT_SINE: return -(cos(PI * t) - 1) / 2;
        
        // Exponential
        case EASE_IN_EXPO: return t == 0 ? 0 : pow(2, 10 * t - 10);
        case EASE_OUT_EXPO: return t == 1 ? 1 : 1 - pow(2, -10 * t);
        case EASE_IN_OUT_EXPO: {
            if (t == 0) return 0;
            if (t == 1) return 1;
            return t < 0.5f ? pow(2, 20 * t - 10) / 2 : (2 - pow(2, -20 * t + 10)) / 2;
        }
        
        // Circular
        case EASE_IN_CIRC: return 1 - sqrt(1 - t * t);
        case EASE_OUT_CIRC: return sqrt(1 - pow(t - 1, 2));
        case EASE_IN_OUT_CIRC: {
            return t < 0.5f
                ? (1 - sqrt(1 - pow(2 * t, 2))) / 2
                : (sqrt(1 - pow(-2 * t + 2, 2)) + 1) / 2;
        }
        
        // Elastic
        case EASE_IN_ELASTIC: {
            const float c4 = (2 * PI) / 3;
            return t == 0 ? 0 : t == 1 ? 1 : -pow(2, 10 * t - 10) * sin((t * 10 - 10.75f) * c4);
        }
        case EASE_OUT_ELASTIC: {
            const float c4 = (2 * PI) / 3;
            return t == 0 ? 0 : t == 1 ? 1 : pow(2, -10 * t) * sin((t * 10 - 0.75f) * c4) + 1;
        }
        case EASE_IN_OUT_ELASTIC: {
            const float c5 = (2 * PI) / 4.5f;
            return t == 0 ? 0 : t == 1 ? 1 : t < 0.5f
                ? -(pow(2, 20 * t - 10) * sin((20 * t - 11.125f) * c5)) / 2
                : (pow(2, -20 * t + 10) * sin((20 * t - 11.125f) * c5)) / 2 + 1;
        }
        
        // Back
        case EASE_IN_BACK: {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1;
            return c3 * t * t * t - c1 * t * t;
        }
        case EASE_OUT_BACK: {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1;
            return 1 + c3 * pow(t - 1, 3) + c1 * pow(t - 1, 2);
        }
        case EASE_IN_OUT_BACK: {
            const float c1 = 1.70158f;
            const float c2 = c1 * 1.525f;
            return t < 0.5f
                ? (pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2
                : (pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
        }
        
        // Bounce
        case EASE_IN_BOUNCE: return 1 - ease(1 - t, EASE_OUT_BOUNCE);
        case EASE_OUT_BOUNCE: {
            const float n1 = 7.5625f;
            const float d1 = 2.75f;
            if (t < 1 / d1) return n1 * t * t;
            else if (t < 2 / d1) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
            else if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
            else { t -= 2.625f / d1; return n1 * t * t + 0.984375f; }
        }
        case EASE_IN_OUT_BOUNCE: {
            return t < 0.5f
                ? (1 - ease(1 - 2 * t, EASE_OUT_BOUNCE)) / 2
                : (1 + ease(2 * t - 1, EASE_OUT_BOUNCE)) / 2;
        }
        
        default: return t;
    }
}

// ============================================================================
// particle_emitter_component::emit - Emit particles manually
// ============================================================================

void particle_emitter_component::emit(int count) {
    for (int i = 0; i < count && (int)particles.size() < maxParticles; i++) {
        Particle p;
        p.position = emitPosition;
        
        // Random velocity within spread cone
        float angle = ofRandom(-emitSpread, emitSpread) * DEG_TO_RAD;
        glm::vec3 randDir = glm::rotate(emitDirection, angle, glm::vec3(0, 0, 1));
        angle = ofRandom(0, 360) * DEG_TO_RAD;
        randDir = glm::rotate(randDir, angle, emitDirection);
        
        float speed = ofRandom(speedMin, speedMax);
        p.velocity = randDir * speed;
        p.acceleration = glm::vec3(0, 0, 0);
        
        p.life = ofRandom(lifeMin, lifeMax);
        p.maxLife = p.life;
        p.size = ofRandom(sizeMin, sizeMax);
        p.color = colorStart;
        p.rotation = ofRandom(0, 360);
        p.rotationSpeed = ofRandom(-180, 180);
        
        particles.push_back(p);
    }
}

// ============================================================================
// postfx_component - Data management helpers
// ============================================================================
// These methods manage the effect chain data structure.
// Actual rendering is handled by AnimationSystem::applyPostFX()

void postfx_component::addEffect(EffectType type) {
    effectChain.push_back(Effect(type));
    
    // Set default params
    switch (type) {
        case BLOOM:
            setParam(type, "threshold", 0.8f);
            setParam(type, "intensity", 1.0f);
            setParam(type, "radius", 5.0f);
            break;
        case BLUR:
            setParam(type, "radius", 5.0f);
            break;
        case CHROMATIC_ABERRATION:
            setParam(type, "amount", 0.02f);
            break;
        case VIGNETTE:
            setParam(type, "intensity", 0.5f);
            setParam(type, "radius", 0.8f);
            break;
        case GRAIN:
            setParam(type, "amount", 0.1f);
            break;
        default:
            break;
    }
}

void postfx_component::removeEffect(EffectType type) {
    effectChain.erase(
        std::remove_if(effectChain.begin(), effectChain.end(),
            [type](const Effect& e) { return e.type == type; }),
        effectChain.end()
    );
}

void postfx_component::setParam(EffectType type, const std::string& param, float value) {
    for (auto& effect : effectChain) {
        if (effect.type == type) {
            effect.params[param] = value;
            break;
        }
    }
}

float postfx_component::getParam(EffectType type, const std::string& param, float defaultValue) {
    for (const auto& effect : effectChain) {
        if (effect.type == type) {
            auto it = effect.params.find(param);
            if (it != effect.params.end()) {
                return it->second;
            }
        }
    }
    return defaultValue;
}

void postfx_component::apply() {
    // Placeholder - actual rendering is handled by AnimationSystem::applyPostFX()
    if (!enabled || effectChain.empty()) return;
}

} // namespace ecs
