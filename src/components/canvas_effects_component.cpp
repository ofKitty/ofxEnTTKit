#include "canvas_effects_component.h"
#include "ofGraphics.h"

namespace ecs {

// Helper to set proper alpha blending for FBO-to-FBO copies that preserve transparency
static void enableFboCopyBlending() {
    ofEnableAlphaBlending();
    // Use premultiplied alpha blending for proper transparency
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

static void disableFboCopyBlending() {
    ofDisableAlphaBlending();
}

void canvas_effects_component::addEffect(entt::entity filterEntity) {
    // Check if already in stack
    for (const auto& e : effectStack) {
        if (e == filterEntity) return;
    }
    effectStack.push_back(filterEntity);
}

void canvas_effects_component::removeEffect(entt::entity filterEntity) {
    effectStack.erase(
        std::remove(effectStack.begin(), effectStack.end(), filterEntity),
        effectStack.end()
    );
}

void canvas_effects_component::moveUp(int index) {
    if (index > 0 && index < (int)effectStack.size()) {
        std::swap(effectStack[index], effectStack[index - 1]);
    }
}

void canvas_effects_component::moveDown(int index) {
    if (index >= 0 && index < (int)effectStack.size() - 1) {
        std::swap(effectStack[index], effectStack[index + 1]);
    }
}

void canvas_effects_component::ensurePingPong(int width, int height, int internalFormat) {
    if (!pingPongAllocated || 
        pingPongFbo.getWidth() != width || 
        pingPongFbo.getHeight() != height) {
        pingPongFbo.allocate(width, height, internalFormat);
        pingPongFbo.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        pingPongAllocated = true;
    }
}

ofFbo& canvas_effects_component::applyEffects(entt::registry& registry, ofFbo& sourceFbo) {
    if (effectStack.empty() || !autoApply) {
        return sourceFbo;
    }
    
    float width = sourceFbo.getWidth();
    float height = sourceFbo.getHeight();
    
    if (usePingPong) {
        ensurePingPong(width, height, GL_RGBA);
    }
    
    ofFbo* current = &sourceFbo;
    ofFbo* target = &pingPongFbo;
    
    for (entt::entity filterEntity : effectStack) {
        if (!registry.valid(filterEntity)) continue;
        
        bool applied = false;
        
        // Try each filter component type
        // First copy current to target with proper alpha blending, then apply filter
        #define TRY_APPLY_FILTER(CompType) \
            if (!applied && registry.any_of<CompType>(filterEntity)) { \
                auto& filter = registry.get<CompType>(filterEntity); \
                if (filter.enabled) { \
                    target->begin(); \
                    ofClear(0, 0, 0, 0); \
                    enableFboCopyBlending(); \
                    ofSetColor(255); \
                    current->draw(0, 0); \
                    disableFboCopyBlending(); \
                    target->end(); \
                    filter.apply(*target); \
                    std::swap(current, target); \
                    applied = true; \
                } \
            }
        
        TRY_APPLY_FILTER(tint_filter_component)
        TRY_APPLY_FILTER(invert_filter_component)
        TRY_APPLY_FILTER(mirror_filter_component)
        TRY_APPLY_FILTER(color_adjust_component)
        TRY_APPLY_FILTER(blur_filter_component)
        TRY_APPLY_FILTER(dither_filter_component)
        TRY_APPLY_FILTER(rotate_filter_component)
        TRY_APPLY_FILTER(threshold_filter_component)
        TRY_APPLY_FILTER(posterize_filter_component)
        TRY_APPLY_FILTER(noise_filter_component)
        TRY_APPLY_FILTER(vignette_filter_component)
        TRY_APPLY_FILTER(chromatic_aberration_component)
        
        #undef TRY_APPLY_FILTER
    }
    
    return *current;
}

// Serialization
ofJson serializeCanvasEffects(const canvas_effects_component& c) {
    ofJson effectsArr = ofJson::array();
    for (auto e : c.effectStack) {
        effectsArr.push_back(static_cast<uint32_t>(e));
    }
    return {
        {"autoApply", c.autoApply},
        {"usePingPong", c.usePingPong},
        {"effectStack", effectsArr}
    };
}

void deserializeCanvasEffects(canvas_effects_component& c, const ofJson& j) {
    c.autoApply = j.value("autoApply", true);
    c.usePingPong = j.value("usePingPong", true);
    c.effectStack.clear();
    if (j.contains("effectStack") && j["effectStack"].is_array()) {
        for (const auto& e : j["effectStack"]) {
            c.effectStack.push_back(static_cast<entt::entity>(e.get<uint32_t>()));
        }
    }
}

} // namespace ecs
