#pragma once
#include "ofMain.h"
#include <entt.hpp>
#include <functional>

// ============================================================================
// ANIMATION & EFFECTS COMPONENTS
// ============================================================================

namespace ecs {

// ============================================================================
// Tween Component
// ============================================================================

struct tween_component {
    enum EasingType {
        LINEAR,
        EASE_IN_QUAD, EASE_OUT_QUAD, EASE_IN_OUT_QUAD,
        EASE_IN_CUBIC, EASE_OUT_CUBIC, EASE_IN_OUT_CUBIC,
        EASE_IN_QUART, EASE_OUT_QUART, EASE_IN_OUT_QUART,
        EASE_IN_QUINT, EASE_OUT_QUINT, EASE_IN_OUT_QUINT,
        EASE_IN_SINE, EASE_OUT_SINE, EASE_IN_OUT_SINE,
        EASE_IN_EXPO, EASE_OUT_EXPO, EASE_IN_OUT_EXPO,
        EASE_IN_CIRC, EASE_OUT_CIRC, EASE_IN_OUT_CIRC,
        EASE_IN_ELASTIC, EASE_OUT_ELASTIC, EASE_IN_OUT_ELASTIC,
        EASE_IN_BACK, EASE_OUT_BACK, EASE_IN_OUT_BACK,
        EASE_IN_BOUNCE, EASE_OUT_BOUNCE, EASE_IN_OUT_BOUNCE
    };
    
    float from;
    float to;
    float current;
    float duration;
    float elapsed;
    
    EasingType easing;
    bool playing;
    bool loop;
    bool pingpong;
    bool yoyo;  // Reverse easing on return
    
    std::function<void(float)> onUpdate;
    std::function<void()> onComplete;
    
    tween_component()
        : from(0.0f)
        , to(1.0f)
        , current(0.0f)
        , duration(1.0f)
        , elapsed(0.0f)
        , easing(LINEAR)
        , playing(false)
        , loop(false)
        , pingpong(false)
        , yoyo(false)
    {}
    
    void play() {
        playing = true;
        elapsed = 0.0f;
    }
    
    void pause() {
        playing = false;
    }
    
    void stop() {
        playing = false;
        elapsed = 0.0f;
        current = from;
    }
    
    void reset() {
        elapsed = 0.0f;
        current = from;
    }
    
    // Static easing function (can be called without AnimationSystem)
    static float ease(float t, EasingType type);
    
    // Animation update handled by AnimationSystem::updateTween()
};

// ============================================================================
// Particle Emitter Component
// ============================================================================

struct particle_emitter_component {
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        ofColor color;
        float life;
        float maxLife;
        float size;
        float rotation;
        float rotationSpeed;
        
        bool isAlive() const { return life > 0; }
        
        void update(float dt) {
            if (!isAlive()) return;
            
            velocity += acceleration * dt;
            position += velocity * dt;
            rotation += rotationSpeed * dt;
            life -= dt;
        }
    };
    
    std::vector<Particle> particles;
    
    int maxParticles;
    float emitRate;  // particles per second
    float emitAccumulator;
    
    // Emission properties
    glm::vec3 emitPosition;
    glm::vec3 emitDirection;
    float emitSpread;
    
    // Particle properties
    float lifeMin, lifeMax;
    float sizeMin, sizeMax;
    float speedMin, speedMax;
    ofColor colorStart, colorEnd;
    
    // Forces
    glm::vec3 gravity;
    float damping;
    
    bool playing;
    bool loop;
    
    particle_emitter_component()
        : maxParticles(1000)
        , emitRate(50.0f)
        , emitAccumulator(0.0f)
        , emitPosition(0, 0, 0)
        , emitDirection(0, 1, 0)
        , emitSpread(45.0f)
        , lifeMin(1.0f)
        , lifeMax(3.0f)
        , sizeMin(5.0f)
        , sizeMax(10.0f)
        , speedMin(50.0f)
        , speedMax(100.0f)
        , colorStart(255, 255, 255, 255)
        , colorEnd(255, 255, 255, 0)
        , gravity(0, -98.0f, 0)
        , damping(0.98f)
        , playing(false)
        , loop(true)
    {
        particles.reserve(maxParticles);
    }
    
    void play() {
        playing = true;
    }
    
    void pause() {
        playing = false;
    }
    
    void stop() {
        playing = false;
        particles.clear();
    }
    
    // Emit particles manually (can be called without AnimationSystem)
    void emit(int count);
    
    // Particle update handled by AnimationSystem::updateParticles()
};

// ============================================================================
// Post-Processing Component
// ============================================================================

struct postfx_component {
    enum EffectType {
        BLOOM,
        BLUR,
        CHROMATIC_ABERRATION,
        VIGNETTE,
        GRAIN,
        SCANLINES,
        CRT,
        GLITCH,
        COLOR_GRADING,
        FXAA,
        DOF,
        SSAO
    };
    
    struct Effect {
        EffectType type;
        bool enabled;
        std::map<std::string, float> params;
        
        Effect(EffectType t) : type(t), enabled(true) {}
    };
    
    std::vector<Effect> effectChain;
    
    ofFbo inputFbo;
    ofFbo outputFbo;
    ofFbo tempFbo;
    
    bool enabled;
    
    postfx_component()
        : enabled(true)
    {}
    
    void addEffect(EffectType type);
    void removeEffect(EffectType type);
    void setParam(EffectType type, const std::string& param, float value);
    float getParam(EffectType type, const std::string& param, float defaultValue = 0.0f);
    void apply();
};


} // namespace ecs
