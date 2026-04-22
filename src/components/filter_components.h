#pragma once
#include "ofMain.h"
#include <entt.hpp>

// ============================================================================
// IMAGE FILTER COMPONENTS - Post-processing effects for FBOs/canvases
// ============================================================================
// Declarations only - implementations in filter_components.cpp
// ============================================================================

namespace ecs {

// ============================================================================
// Base filter properties
// ============================================================================

struct filter_base {
    bool enabled = true;
    int order = 0;
    filter_base() = default;
    filter_base(bool e, int o = 0) : enabled(e), order(o) {}
};

// ============================================================================
// TINT FILTER
// ============================================================================

struct tint_filter_component : filter_base {
    ofColor color = ofColor::white;
    float strength = 1.0f;
    bool convertToGrey = false;
    
    tint_filter_component() = default;
    tint_filter_component(const ofColor& c, float s = 1.0f) : color(c), strength(s) {}
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// INVERT FILTER
// ============================================================================

struct invert_filter_component : filter_base {
    bool invertRGB = true;
    bool invertAlpha = false;
    float strength = 1.0f;
    
    invert_filter_component() = default;
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// MIRROR FILTER
// ============================================================================

struct mirror_filter_component : filter_base {
    bool horizontal = false;
    bool vertical = false;
    
    mirror_filter_component() = default;
    mirror_filter_component(bool h, bool v) : horizontal(h), vertical(v) {}
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// COLOR ADJUST FILTER
// ============================================================================

struct color_adjust_component : filter_base {
    float brightness = 0.0f;
    float contrast = 1.0f;
    float saturation = 1.0f;
    float gamma = 1.0f;
    float hueShift = 0.0f;
    
    color_adjust_component() = default;
    void apply(ofFbo& fbo) const;
    bool hasAdjustment() const;
};

// ============================================================================
// BLUR FILTER
// ============================================================================

struct blur_filter_component : filter_base {
    float radius = 5.0f;
    int passes = 1;
    bool horizontal = true;
    bool vertical = true;
    
    blur_filter_component() = default;
    blur_filter_component(float r, int p = 1) : radius(r), passes(p) {}
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// DITHER FILTER
// ============================================================================

enum class DitherType {
    FloydSteinberg = 0, Ordered2x2, Ordered4x4, Ordered8x8,
    Bayer2x2, Bayer4x4, Bayer8x8, Random
};

struct dither_filter_component : filter_base {
    DitherType type = DitherType::FloydSteinberg;
    int levels = 4;
    bool colorDither = false;
    float strength = 1.0f;
    
    dither_filter_component() = default;
    dither_filter_component(DitherType t, int l = 4) : type(t), levels(l) {}
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// ROTATE FILTER
// ============================================================================

struct rotate_filter_component : filter_base {
    float angleDegrees = 0.0f;
    bool rotate90CW = false;
    bool rotate90CCW = false;
    bool rotate180 = false;
    
    rotate_filter_component() = default;
    rotate_filter_component(float angle) : angleDegrees(angle) {}
    float getEffectiveAngle() const;
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// THRESHOLD FILTER
// ============================================================================

struct threshold_filter_component : filter_base {
    float threshold = 0.5f;
    bool invert = false;
    ofColor lowColor = ofColor::black;
    ofColor highColor = ofColor::white;
    
    threshold_filter_component() = default;
    threshold_filter_component(float t) : threshold(t) {}
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// POSTERIZE FILTER
// ============================================================================

struct posterize_filter_component : filter_base {
    int levels = 4;
    
    posterize_filter_component() = default;
    posterize_filter_component(int l) : levels(l) {}
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// NOISE FILTER
// ============================================================================

struct noise_filter_component : filter_base {
    float amount = 0.1f;
    bool monochrome = true;
    bool animated = false;
    int seed = 0;
    
    noise_filter_component() = default;
    noise_filter_component(float a, bool mono = true) : amount(a), monochrome(mono) {}
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// VIGNETTE FILTER
// ============================================================================

struct vignette_filter_component : filter_base {
    float radius = 0.75f;
    float softness = 0.45f;
    float strength = 1.0f;
    ofColor color = ofColor::black;
    
    vignette_filter_component() = default;
    vignette_filter_component(float r, float soft = 0.45f) : radius(r), softness(soft) {}
    void apply(ofFbo& fbo) const;
};

// ============================================================================
// CHROMATIC ABERRATION FILTER
// ============================================================================

struct chromatic_aberration_component : filter_base {
    float amount = 2.0f;
    float angle = 0.0f;
    bool radial = true;
    
    chromatic_aberration_component() = default;
    chromatic_aberration_component(float a) : amount(a) {}
    void apply(ofFbo& fbo) const;
};

} // namespace ecs
