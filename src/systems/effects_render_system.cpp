#include "effects_render_system.h"

namespace ecs {
using namespace ecs;

// ============================================================================
// Shadow Functions
// ============================================================================

void EffectsRenderSystem::beginShadow(const shadow_component & comp) {
    if (!comp.enabled) return;
    
    ofPushMatrix();
    ofTranslate(comp.offset.x, comp.offset.y);
    ofPushStyle();
    ofSetColor(comp.getShadowColor());
}

void EffectsRenderSystem::endShadow(const shadow_component & comp) {
    if (!comp.enabled) return;
    
    ofPopStyle();
    ofPopMatrix();
}

void EffectsRenderSystem::drawRectShadow(const shadow_component & comp, float x, float y, float w, float h) {
    if (!comp.enabled) return;
    
    ofPushStyle();
    ofSetColor(comp.getShadowColor());
    ofFill();
    ofDrawRectangle(x + comp.offset.x, y + comp.offset.y, w, h);
    ofPopStyle();
}

void EffectsRenderSystem::drawCircleShadow(const shadow_component & comp, float x, float y, float radius) {
    if (!comp.enabled) return;
    
    ofPushStyle();
    ofSetColor(comp.getShadowColor());
    ofFill();
    ofDrawCircle(x + comp.offset.x, y + comp.offset.y, radius);
    ofPopStyle();
}

// ============================================================================
// Glow Functions
// ============================================================================

void EffectsRenderSystem::drawGlowCircle(const glow_component & comp, float x, float y, float baseRadius) {
    if (!comp.enabled) return;
    
    ofPushStyle();
    ofFill();
    
    for (int i = comp.passes - 1; i >= 0; i--) {
        float glowRadius = baseRadius + comp.radius * (i + 1) / comp.passes;
        ofSetColor(comp.getGlowColor(i));
        ofDrawCircle(x, y, glowRadius);
    }
    
    ofPopStyle();
}

void EffectsRenderSystem::drawGlowRect(const glow_component & comp, float x, float y, float w, float h) {
    if (!comp.enabled) return;
    
    ofPushStyle();
    ofFill();
    
    for (int i = comp.passes - 1; i >= 0; i--) {
        float expand = comp.radius * (i + 1) / comp.passes;
        ofSetColor(comp.getGlowColor(i));
        ofDrawRectangle(x - expand, y - expand, w + expand * 2, h + expand * 2);
    }
    
    ofPopStyle();
}

// ============================================================================
// Outline Functions
// ============================================================================

void EffectsRenderSystem::drawMeshOutline(const outline_component & comp, ofMesh & mesh) {
    if (!comp.enabled) return;
    
    ofPushStyle();
    ofSetColor(comp.outlineColor);
    ofSetLineWidth(comp.outlineWidth);
    
    for (int pass = 0; pass < comp.passes; pass++) {
        float scale = 1.0f + (comp.outlineWidth / 100.0f) * (pass + 1);
        
        ofPushMatrix();
        ofScale(scale, scale, scale);
        mesh.drawWireframe();
        ofPopMatrix();
    }
    
    ofPopStyle();
}

} // namespace ecs
