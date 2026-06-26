#pragma once
#include "ofMain.h"
#include <entt.hpp>
#include <vector>

// ============================================================================
// GENERATOR COMPONENTS - Procedural pattern generators
// ============================================================================
// Pure data structs. Rendering and time-advance logic lives in
// systems/generator_render_system.h (GeneratorRenderSystem).
// ============================================================================

namespace ecs {

// NOTE: gradients are not generators. The canonical gradient model lives in
// components/paint_components.h (ecs::gradient_component). It is registered with
// the generator render registry inside finalizeGenerators(), so a gradient paint
// still rasterizes full-area like any other generator.

// ============================================================================
// DOTS GENERATOR
// ============================================================================

struct dots_generator_component {
    bool enabled = true;
    int countX = 10;
    int countY = 10;
    float dotSize = 5.0f;
    float spacing = 2.0f;
    ofColor dotColor = ofColor::white;
    ofColor bgColor = ofColor::black;
    bool randomSize = false;
    float randomSizeMin = 2.0f;
    float randomSizeMax = 8.0f;
    bool filled = true;
    float strokeWeight = 1.0f;
    bool offset = false;
    
    dots_generator_component() = default;
    dots_generator_component(int cx, int cy, float size);
};

// ============================================================================
// STRIPES GENERATOR
// ============================================================================

struct stripes_generator_component {
    bool enabled = true;
    bool vertical = true;
    int count = 10;
    float stripeWidth = 5.0f;
    ofColor color1 = ofColor::black;
    ofColor color2 = ofColor::white;
    float offset = 0.0f;
    float angle = 0.0f;
    
    stripes_generator_component() = default;
    stripes_generator_component(int c, float w, bool vert = true);
};

// ============================================================================
// CHECKERBOARD GENERATOR
// ============================================================================

struct checkerboard_generator_component {
    bool enabled = true;
    int countX = 8;
    int countY = 8;
    ofColor color1 = ofColor::black;
    ofColor color2 = ofColor::white;
    
    checkerboard_generator_component() = default;
    checkerboard_generator_component(int cx, int cy);
};

// ============================================================================
// NOISE GENERATOR
// ============================================================================

struct noise_generator_component {
    bool enabled = true;
    float scale = 0.01f;
    float speed = 1.0f;
    int octaves = 4;
    float persistence = 0.5f;
    ofColor colorLow = ofColor::black;
    ofColor colorHigh = ofColor::white;
    float timeOffset = 0.0f;
    
    noise_generator_component() = default;
    noise_generator_component(float s, int oct = 4);
};

} // namespace ecs
