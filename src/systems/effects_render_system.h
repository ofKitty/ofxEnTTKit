#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/rendering_components.h"

namespace ecs {

// ============================================================================
// Effects Render System
// ============================================================================
// Handles rendering of visual effects: shadows, glows, outlines.
// Components are pure data - this system contains the drawing logic.

class EffectsRenderSystem : public ISystem {
public:
    const char* getName() const override { return "EffectsRenderSystem"; }
    
    // Note: Effects are typically drawn in conjunction with other draws,
    // so this system provides static helpers rather than auto-drawing
    
    // Shadow functions
    static void beginShadow(const shadow_component& comp);
    static void endShadow(const shadow_component& comp);
    static void drawRectShadow(const shadow_component& comp, float x, float y, float w, float h);
    static void drawCircleShadow(const shadow_component& comp, float x, float y, float radius);
    
    // Glow functions
    static void drawGlowCircle(const glow_component& comp, float x, float y, float baseRadius);
    static void drawGlowRect(const glow_component& comp, float x, float y, float w, float h);
    
    // Outline functions
    static void drawMeshOutline(const outline_component& comp, ofMesh& mesh);
};

} // namespace ecs
