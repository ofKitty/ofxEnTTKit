#pragma once
#include "ofMain.h"
#include <entt.hpp>

// ============================================================================
// DRAW FILTER COMPONENTS - Re-draw image with artistic effects
// ============================================================================
// Declarations only - implementations in draw_filter_components.cpp
// ============================================================================

namespace ecs {

enum class PixelStyle {
    Rectangle = 0, Circle, RoundedRect, Diamond, CMYK, RGB, Halftone
};

struct pixelate_filter_component {
    bool enabled = true;
    int tilesX = 32;
    int tilesY = 32;
    PixelStyle style = PixelStyle::Rectangle;
    float rounded = 0.0f;
    bool polka = false;
    float gapRatio = 0.1f;
    ofColor cyanColor = ofColor(0, 174, 239);
    ofColor magentaColor = ofColor(236, 0, 140);
    ofColor yellowColor = ofColor(255, 242, 0);
    ofColor blackColor = ofColor::black;
    
    pixelate_filter_component() = default;
    pixelate_filter_component(int tx, int ty, PixelStyle s = PixelStyle::Rectangle);
    void draw(ofImage& source, float width, float height) const;
};

struct mesh_filter_component {
    bool enabled = true;
    int gridX = 20;
    int gridY = 20;
    float lineWidth = 1.0f;
    bool drawVertical = true;
    bool drawHorizontal = true;
    bool drawDiagonal = false;
    bool colorFromImage = true;
    ofColor lineColor = ofColor::white;
    ofColor backgroundColor = ofColor::black;
    bool drawBackground = true;
    
    mesh_filter_component() = default;
    mesh_filter_component(int gx, int gy, float lw = 1.0f);
    void draw(ofImage& source, float width, float height) const;
};

struct rings_filter_component {
    bool enabled = true;
    int ringCount = 20;
    float lineWidth = 2.0f;
    bool colorFromImage = true;
    ofColor ringColor = ofColor::white;
    glm::vec2 center = {0.5f, 0.5f};
    bool filled = false;
    float gapRatio = 0.5f;
    
    rings_filter_component() = default;
    rings_filter_component(int count, float lw = 2.0f);
    void draw(ofImage& source, float width, float height) const;
};

struct noise_displacement_component {
    bool enabled = true;
    float scale = 0.01f;
    float amplitude = 10.0f;
    float speed = 1.0f;
    float timeOffset = 0.0f;
    
    noise_displacement_component() = default;
    noise_displacement_component(float s, float amp);
    void update(float dt);
    void draw(ofImage& source, float width, float height) const;
};

struct line_scan_filter_component {
    bool enabled = true;
    bool vertical = false;
    int lineCount = 100;
    float lineWidth = 1.0f;
    bool colorFromImage = true;
    ofColor lineColor = ofColor::white;
    ofColor backgroundColor = ofColor::black;
    bool sizeFromBrightness = false;
    float minLineWidth = 0.5f;
    float maxLineWidth = 3.0f;
    
    line_scan_filter_component() = default;
    line_scan_filter_component(int count, float lw = 1.0f, bool vert = false);
    void draw(ofImage& source, float width, float height) const;
};

struct ascii_filter_component {
    bool enabled = true;
    int charsX = 80;
    int charsY = 40;
    std::string charset = " .:-=+*#%@";
    bool colorFromImage = true;
    ofColor textColor = ofColor::white;
    ofColor backgroundColor = ofColor::black;
    float fontSize = 10.0f;
    bool invert = false;
    
    ascii_filter_component() = default;
    ascii_filter_component(int cx, int cy, float size = 10.0f);
    char getCharForBrightness(float brightness) const;
    void draw(ofImage& source, float width, float height, ofTrueTypeFont& font) const;
};

} // namespace ecs
