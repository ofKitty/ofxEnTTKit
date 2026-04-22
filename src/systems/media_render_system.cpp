#include "media_render_system.h"

namespace ecs {
using namespace ecs;

void MediaRenderSystem::update(entt::registry& registry, float deltaTime) {
    // Update all video components
    auto videos = registry.view<video_component>();
    for (auto entity : videos) {
        auto& vc = registry.get<video_component>(entity);
        updateVideo(vc);
    }
}

void MediaRenderSystem::draw(entt::registry& registry) {
    // TODO ...
}

// ============================================================================
// Video Update
// ============================================================================

void MediaRenderSystem::updateVideo(video_component& comp) {
    comp.videoPlayer.update();
}

// ============================================================================
// Image Drawing
// ============================================================================

void MediaRenderSystem::drawImage(const image_component & comp) {
    if (!comp.image.isAllocated()) return;
    
    ofPushStyle();
    ofSetColor(255);  // No tint
    comp.image.draw(0, 0);
    ofPopStyle();
}

// ============================================================================
// Video Drawing
// ============================================================================

void MediaRenderSystem::drawVideo(const video_component & comp) {
    if (!comp.videoPlayer.isLoaded()) return;
    
    ofPushStyle();
    ofSetColor(255);
    comp.videoPlayer.draw(0, 0);
    ofPopStyle();
}

// ============================================================================
// FBO Drawing
// ============================================================================

void MediaRenderSystem::drawFbo(const fbo_component & comp) {
    if (!comp.fbo.isAllocated()) return;
    
    ofPushStyle();
    ofSetColor(255);  // No tint
    comp.fbo.draw(0, 0);
    ofPopStyle();
}

} // namespace ecs
