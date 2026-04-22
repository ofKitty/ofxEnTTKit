#pragma once
#include "ofMain.h"
#include <nlohmann/json.hpp>
#include <entt.hpp>
#include <vector>

// ============================================================================
// LED UV COMPONENTS
// Minimal UV-based LED mapping for ESP32 integration
// ============================================================================

namespace ecs {

// ============================================================================
// LED Pixel - stores UV coordinates only
// ============================================================================

struct LEDPixel {
    int index;
    glm::vec2 uv;       // Normalized 0-1 coordinates
    ofColor color;      // Runtime sampling result (not serialized)
    
    LEDPixel() 
        : index(0)
        , uv(0, 0)
        , color(0, 0, 0)
    {}
    
    LEDPixel(int idx, float u, float v)
        : index(idx)
        , uv(u, v)
        , color(0, 0, 0)
    {}
};

// ============================================================================
// UV Component
// Primary component for LED UV mapping - matches ESP32 JSON format:
// { "name": "...", "width": N, "height": N, "points": [{index, u, v}...], "LedCount": N }
// ============================================================================

struct uv_component {
    std::string name;
    float width;                    // Canvas width (mm or pixels)
    float height;                   // Canvas height (mm or pixels)
    std::vector<LEDPixel> pixels;
    
    // Rendering system: source canvas and drawable
    entt::entity sourceEntity = entt::null;    // Canvas to sample from
    entt::entity drawableEntity = entt::null;  // Shape to draw (optional, default=circle)
    std::vector<ofColor> sampledColors;         // Sampled color per LED
    float drawScale = 1.0f;                     // Scale factor for drawable
    bool showPreview = true;                    // Enable/disable visualization
    
    uv_component()
        : name("Untitled")
        , width(100)
        , height(100)
    {}
    
    // Get LED count
    int getLedCount() const { return static_cast<int>(pixels.size()); }
    
    // Get absolute position from UV (for sampling)
    glm::vec2 getAbsolutePosition(int index) const {
        if (index < 0 || index >= static_cast<int>(pixels.size())) {
            return glm::vec2(0, 0);
        }
        return glm::vec2(pixels[index].uv.x * width, pixels[index].uv.y * height);
    }
    
    // Get absolute position for a given canvas size (for FBO sampling)
    glm::vec2 getAbsolutePosition(int index, float canvasWidth, float canvasHeight) const {
        if (index < 0 || index >= static_cast<int>(pixels.size())) {
            return glm::vec2(0, 0);
        }
        return glm::vec2(pixels[index].uv.x * canvasWidth, pixels[index].uv.y * canvasHeight);
    }
    
    // Load from JSON file (ESP32 UV format)
    bool loadFromJSON(const std::string& jsonPath);
    
    // Save to JSON file (ESP32 UV format)
    bool saveToJSON(const std::string& jsonPath) const;
    
    // Load from JSON object
    bool loadFromJSON(const nlohmann::json& j);
    
    // Export to JSON object
    nlohmann::json toJSON() const;
};

// ============================================================================
// UV Sample Component
// Composes with fbo_component to enable UV sampling from any FBO canvas.
// Add this component to an entity that has fbo_component to sample LEDs.
// ============================================================================

struct uv_sample_component {
    entt::entity uvEntity = entt::null;   // Entity with uv_component to sample from
    std::vector<ofColor> colors;           // Sampled results (indexed by LED index)
    bool autoSample = true;                // Sample every frame when true
    
    uv_sample_component() = default;
    
    uv_sample_component(entt::entity uv) 
        : uvEntity(uv)
        , autoSample(true) 
    {}
    
    // Get number of sampled colors
    int getColorCount() const { return static_cast<int>(colors.size()); }
};

} // namespace ecs
