#pragma once
#include <cmath>

// ============================================================================
// EASING FUNCTIONS - Reusable easing library
// ============================================================================
// Implementations are in easing.cpp to reduce compilation overhead.
// ============================================================================

#ifndef PI
#define PI 3.14159265358979323846f
#endif

namespace ecs {

// ============================================================================
// Easing Types Enum
// ============================================================================

enum class EasingType {
    Linear = 0,
    
    // Quadratic
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    
    // Cubic
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    
    // Quartic
    EaseInQuart,
    EaseOutQuart,
    EaseInOutQuart,
    
    // Quintic
    EaseInQuint,
    EaseOutQuint,
    EaseInOutQuint,
    
    // Sinusoidal
    EaseInSine,
    EaseOutSine,
    EaseInOutSine,
    
    // Exponential
    EaseInExpo,
    EaseOutExpo,
    EaseInOutExpo,
    
    // Circular
    EaseInCirc,
    EaseOutCirc,
    EaseInOutCirc,
    
    // Elastic
    EaseInElastic,
    EaseOutElastic,
    EaseInOutElastic,
    
    // Back (overshoot)
    EaseInBack,
    EaseOutBack,
    EaseInOutBack,
    
    // Bounce
    EaseInBounce,
    EaseOutBounce,
    EaseInOutBounce,
    
    COUNT
};

// ============================================================================
// Easing Function Declarations
// ============================================================================

namespace easing {

// Apply easing by type
float apply(EasingType type, float t);

// Individual easing functions (declared, defined in .cpp)
float linear(float t);

float easeInQuad(float t);
float easeOutQuad(float t);
float easeInOutQuad(float t);

float easeInCubic(float t);
float easeOutCubic(float t);
float easeInOutCubic(float t);

float easeInQuart(float t);
float easeOutQuart(float t);
float easeInOutQuart(float t);

float easeInQuint(float t);
float easeOutQuint(float t);
float easeInOutQuint(float t);

float easeInSine(float t);
float easeOutSine(float t);
float easeInOutSine(float t);

float easeInExpo(float t);
float easeOutExpo(float t);
float easeInOutExpo(float t);

float easeInCirc(float t);
float easeOutCirc(float t);
float easeInOutCirc(float t);

float easeInElastic(float t);
float easeOutElastic(float t);
float easeInOutElastic(float t);

float easeInBack(float t);
float easeOutBack(float t);
float easeInOutBack(float t);

float easeOutBounce(float t);
float easeInBounce(float t);
float easeInOutBounce(float t);

// Get name of easing type for UI
const char* getName(EasingType type);

} // namespace easing
} // namespace ecs
