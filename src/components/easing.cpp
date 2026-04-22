#include "easing.h"

namespace ecs {
namespace easing {

// ============================================================================
// Apply easing by type
// ============================================================================

float apply(EasingType type, float t) {
    switch (type) {
        case EasingType::Linear:         return linear(t);
        case EasingType::EaseInQuad:     return easeInQuad(t);
        case EasingType::EaseOutQuad:    return easeOutQuad(t);
        case EasingType::EaseInOutQuad:  return easeInOutQuad(t);
        case EasingType::EaseInCubic:    return easeInCubic(t);
        case EasingType::EaseOutCubic:   return easeOutCubic(t);
        case EasingType::EaseInOutCubic: return easeInOutCubic(t);
        case EasingType::EaseInQuart:    return easeInQuart(t);
        case EasingType::EaseOutQuart:   return easeOutQuart(t);
        case EasingType::EaseInOutQuart: return easeInOutQuart(t);
        case EasingType::EaseInQuint:    return easeInQuint(t);
        case EasingType::EaseOutQuint:   return easeOutQuint(t);
        case EasingType::EaseInOutQuint: return easeInOutQuint(t);
        case EasingType::EaseInSine:     return easeInSine(t);
        case EasingType::EaseOutSine:    return easeOutSine(t);
        case EasingType::EaseInOutSine:  return easeInOutSine(t);
        case EasingType::EaseInExpo:     return easeInExpo(t);
        case EasingType::EaseOutExpo:    return easeOutExpo(t);
        case EasingType::EaseInOutExpo:  return easeInOutExpo(t);
        case EasingType::EaseInCirc:     return easeInCirc(t);
        case EasingType::EaseOutCirc:    return easeOutCirc(t);
        case EasingType::EaseInOutCirc:  return easeInOutCirc(t);
        case EasingType::EaseInElastic:  return easeInElastic(t);
        case EasingType::EaseOutElastic: return easeOutElastic(t);
        case EasingType::EaseInOutElastic: return easeInOutElastic(t);
        case EasingType::EaseInBack:     return easeInBack(t);
        case EasingType::EaseOutBack:    return easeOutBack(t);
        case EasingType::EaseInOutBack:  return easeInOutBack(t);
        case EasingType::EaseInBounce:   return easeInBounce(t);
        case EasingType::EaseOutBounce:  return easeOutBounce(t);
        case EasingType::EaseInOutBounce: return easeInOutBounce(t);
        default: return t;
    }
}

// ============================================================================
// Individual Easing Functions
// ============================================================================

float linear(float t) { return t; }

// Quadratic
float easeInQuad(float t) { return t * t; }
float easeOutQuad(float t) { return t * (2.0f - t); }
float easeInOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

// Cubic
float easeInCubic(float t) { return t * t * t; }
float easeOutCubic(float t) { float t1 = t - 1.0f; return t1 * t1 * t1 + 1.0f; }
float easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
}

// Quartic
float easeInQuart(float t) { return t * t * t * t; }
float easeOutQuart(float t) { float t1 = t - 1.0f; return 1.0f - t1 * t1 * t1 * t1; }
float easeInOutQuart(float t) {
    float t1 = t - 1.0f;
    return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - 8.0f * t1 * t1 * t1 * t1;
}

// Quintic
float easeInQuint(float t) { return t * t * t * t * t; }
float easeOutQuint(float t) { float t1 = t - 1.0f; return 1.0f + t1 * t1 * t1 * t1 * t1; }
float easeInOutQuint(float t) {
    float t1 = t - 1.0f;
    return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f + 16.0f * t1 * t1 * t1 * t1 * t1;
}

// Sinusoidal
float easeInSine(float t) { return 1.0f - cosf(t * PI / 2.0f); }
float easeOutSine(float t) { return sinf(t * PI / 2.0f); }
float easeInOutSine(float t) { return -(cosf(PI * t) - 1.0f) / 2.0f; }

// Exponential
float easeInExpo(float t) { return t == 0.0f ? 0.0f : powf(2.0f, 10.0f * t - 10.0f); }
float easeOutExpo(float t) { return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t); }
float easeInOutExpo(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    return t < 0.5f ? powf(2.0f, 20.0f * t - 10.0f) / 2.0f : (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

// Circular
float easeInCirc(float t) { return 1.0f - sqrtf(1.0f - t * t); }
float easeOutCirc(float t) { return sqrtf(1.0f - (t - 1.0f) * (t - 1.0f)); }
float easeInOutCirc(float t) {
    return t < 0.5f
        ? (1.0f - sqrtf(1.0f - 4.0f * t * t)) / 2.0f
        : (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

// Elastic
float easeInElastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * (2.0f * PI) / 3.0f);
}
float easeOutElastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * (2.0f * PI) / 3.0f) + 1.0f;
}
float easeInOutElastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return t < 0.5f
        ? -(powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * (2.0f * PI) / 4.5f)) / 2.0f
        : (powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * (2.0f * PI) / 4.5f)) / 2.0f + 1.0f;
}

// Back
float easeInBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
}
float easeOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
}
float easeInOutBack(float t) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    return t < 0.5f
        ? (powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
        : (powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
}

// Bounce
float easeOutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    
    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

float easeInBounce(float t) {
    return 1.0f - easeOutBounce(1.0f - t);
}

float easeInOutBounce(float t) {
    return t < 0.5f
        ? (1.0f - easeOutBounce(1.0f - 2.0f * t)) / 2.0f
        : (1.0f + easeOutBounce(2.0f * t - 1.0f)) / 2.0f;
}

const char* getName(EasingType type) {
    switch (type) {
        case EasingType::Linear: return "Linear";
        case EasingType::EaseInQuad: return "Ease In Quad";
        case EasingType::EaseOutQuad: return "Ease Out Quad";
        case EasingType::EaseInOutQuad: return "Ease In/Out Quad";
        case EasingType::EaseInCubic: return "Ease In Cubic";
        case EasingType::EaseOutCubic: return "Ease Out Cubic";
        case EasingType::EaseInOutCubic: return "Ease In/Out Cubic";
        case EasingType::EaseInQuart: return "Ease In Quart";
        case EasingType::EaseOutQuart: return "Ease Out Quart";
        case EasingType::EaseInOutQuart: return "Ease In/Out Quart";
        case EasingType::EaseInQuint: return "Ease In Quint";
        case EasingType::EaseOutQuint: return "Ease Out Quint";
        case EasingType::EaseInOutQuint: return "Ease In/Out Quint";
        case EasingType::EaseInSine: return "Ease In Sine";
        case EasingType::EaseOutSine: return "Ease Out Sine";
        case EasingType::EaseInOutSine: return "Ease In/Out Sine";
        case EasingType::EaseInExpo: return "Ease In Expo";
        case EasingType::EaseOutExpo: return "Ease Out Expo";
        case EasingType::EaseInOutExpo: return "Ease In/Out Expo";
        case EasingType::EaseInCirc: return "Ease In Circ";
        case EasingType::EaseOutCirc: return "Ease Out Circ";
        case EasingType::EaseInOutCirc: return "Ease In/Out Circ";
        case EasingType::EaseInElastic: return "Ease In Elastic";
        case EasingType::EaseOutElastic: return "Ease Out Elastic";
        case EasingType::EaseInOutElastic: return "Ease In/Out Elastic";
        case EasingType::EaseInBack: return "Ease In Back";
        case EasingType::EaseOutBack: return "Ease Out Back";
        case EasingType::EaseInOutBack: return "Ease In/Out Back";
        case EasingType::EaseInBounce: return "Ease In Bounce";
        case EasingType::EaseOutBounce: return "Ease Out Bounce";
        case EasingType::EaseInOutBounce: return "Ease In/Out Bounce";
        default: return "Unknown";
    }
}

} // namespace easing
} // namespace ecs
