#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/base_components.h"

namespace ecs {

// ============================================================================
// Media Render System
// ============================================================================
// Handles rendering of images, videos, and FBOs.
// Components are pure data - this system contains the rendering logic.
//
// Note: webcam_component does not own an ofVideoGrabber (to keep it
// trivially-constructible for EnTT).  The owning subsystem (e.g. WebcamTracer)
// drives the grabber and writes state into webcam_component directly.

class MediaRenderSystem : public ISystem {
public:
    const char* getName() const override { return "MediaRenderSystem"; }
    
    void update(entt::registry& registry, float deltaTime) override;
    void draw(entt::registry& registry) override;
    
    // Individual operations
    static void updateVideo(video_component& comp);
    static void drawImage(const image_component& comp);
    static void drawVideo(const video_component& comp);
    static void drawFbo(const fbo_component& comp);
};

} // namespace ecs
