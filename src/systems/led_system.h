#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/led_components.h"
#include "../components/base_components.h"

namespace ecs {

// ============================================================================
// LED System
// ============================================================================
// Handles LED UV mapping and sampling.
// Components are pure data - this system contains the logic.

class LEDSystem : public ISystem {
public:
    const char* getName() const override { return "LEDSystem"; }
    
    void update(entt::registry& registry, float deltaTime) override;
    void draw(entt::registry& registry) override;
    
    // UV-based sampling from texture
    static void sampleFromTexture(uv_component& uvComp, const ofTexture& tex, std::vector<ofColor>& colors);
    static void sampleFromFbo(uv_component& uvComp, ofFbo& fbo, std::vector<ofColor>& colors);
    static void sampleFromPixels(uv_component& uvComp, ofPixels& pixels, std::vector<ofColor>& colors);
    
    // Visualization
    static void drawUVPreview(const uv_component& uvComp, const std::vector<ofColor>& colors, float pointSize = 5.0f);
    
    // Output buffer preparation (RGB bytes for serial/UDP)
    static void prepareOutput(const uv_component& uvComp, const std::vector<ofColor>& colors, std::vector<unsigned char>& buffer);
};

} // namespace ecs
