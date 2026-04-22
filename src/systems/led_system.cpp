#include "led_system.h"

namespace ecs {

void LEDSystem::update(entt::registry& registry, float deltaTime) {
    // Sample from uv_component.sourceEntity (canvas to sample from)
    auto uvView = registry.view<uv_component>();
    for (auto entity : uvView) {
        auto& uv = uvView.get<uv_component>(entity);
        if (uv.sourceEntity == entt::null) continue;
        if (!registry.valid(uv.sourceEntity)) continue;

        auto* fboComp = registry.try_get<fbo_component>(uv.sourceEntity);
        if (fboComp && fboComp->fbo.isAllocated()) {
            sampleFromFbo(uv, fboComp->fbo, uv.sampledColors);
        }
    }

    // Process entities with both fbo_component and uv_sample_component (legacy)
    auto view = registry.view<fbo_component, uv_sample_component>();
    for (auto entity : view) {
        auto& fboComp = view.get<fbo_component>(entity);
        auto& sampleComp = view.get<uv_sample_component>(entity);

        // Skip if auto-sample is disabled
        if (!sampleComp.autoSample) continue;

        // Skip if no UV entity set or invalid
        if (sampleComp.uvEntity == entt::null) continue;
        if (!registry.valid(sampleComp.uvEntity)) continue;

        // Get the UV component from the referenced entity
        auto* uvComp = registry.try_get<uv_component>(sampleComp.uvEntity);
        if (!uvComp) continue;

        // Sample from FBO
        if (fboComp.fbo.isAllocated()) {
            sampleFromFbo(*uvComp, fboComp.fbo, sampleComp.colors);
        }
    }
}

void LEDSystem::draw(entt::registry& registry) {
    // UV visualization is optional - application can draw if needed
}

// ============================================================================
// UV-based Sampling Functions
// ============================================================================

void LEDSystem::sampleFromTexture(uv_component& uvComp, const ofTexture& tex, std::vector<ofColor>& colors) {
    if (!tex.isAllocated()) return;
    
    // Read pixels from texture
    ofPixels pixels;
    tex.readToPixels(pixels);
    sampleFromPixels(uvComp, pixels, colors);
}

void LEDSystem::sampleFromFbo(uv_component& uvComp, ofFbo& fbo, std::vector<ofColor>& colors) {
    if (!fbo.isAllocated()) return;
    
    ofPixels pixels;
    fbo.readToPixels(pixels);
    sampleFromPixels(uvComp, pixels, colors);
}

void LEDSystem::sampleFromPixels(uv_component& uvComp, ofPixels& pixels, std::vector<ofColor>& colors) {
    colors.resize(uvComp.pixels.size());
    
    int width = pixels.getWidth();
    int height = pixels.getHeight();
    
    for (size_t i = 0; i < uvComp.pixels.size(); ++i) {
        const auto& pixel = uvComp.pixels[i];
        
        // Convert UV (0-1) to pixel coordinates
        int x = static_cast<int>(pixel.uv.x * width);
        int y = static_cast<int>(pixel.uv.y * height);
        x = ofClamp(x, 0, width - 1);
        y = ofClamp(y, 0, height - 1);
        
        colors[pixel.index] = pixels.getColor(x, y);
    }
}

// ============================================================================
// Visualization
// ============================================================================

void LEDSystem::drawUVPreview(const uv_component& uvComp, const std::vector<ofColor>& colors, float pointSize) {
    ofPushStyle();
    
    for (size_t i = 0; i < uvComp.pixels.size() && i < colors.size(); ++i) {
        const auto& pixel = uvComp.pixels[i];
        
        // Convert UV to absolute position using component's width/height
        float x = pixel.uv.x * uvComp.width;
        float y = pixel.uv.y * uvComp.height;
        
        ofSetColor(colors[pixel.index]);
        ofDrawCircle(x, y, pointSize);
    }
    
    ofPopStyle();
}

// ============================================================================
// Output Functions
// ============================================================================

void LEDSystem::prepareOutput(const uv_component& uvComp, const std::vector<ofColor>& colors, std::vector<unsigned char>& buffer) {
    // Prepare RGB buffer for serial/UDP output
    size_t requiredSize = uvComp.pixels.size() * 3; // RGB
    if (buffer.size() < requiredSize) {
        buffer.resize(requiredSize);
    }
    
    for (size_t i = 0; i < uvComp.pixels.size() && i < colors.size(); ++i) {
        int idx = uvComp.pixels[i].index;
        if (idx >= 0 && idx < (int)colors.size()) {
            buffer[i * 3 + 0] = colors[idx].r;
            buffer[i * 3 + 1] = colors[idx].g;
            buffer[i * 3 + 2] = colors[idx].b;
        }
    }
}

} // namespace ecs
