#include "filter_components.h"
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

// ============================================================================
// TINT FILTER
// ============================================================================

void tint_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || strength <= 0.0f) return;
    
    ofPushStyle();
    fbo.begin();
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    
    ofColor tintColor = color;
    tintColor.a = (unsigned char)(strength * 255.0f);
    ofSetColor(tintColor);
    ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
    
    ofDisableBlendMode();
    fbo.end();
    ofPopStyle();
}

// ============================================================================
// INVERT FILTER
// ============================================================================

void invert_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || strength <= 0.0f) return;
    // Requires shader - stub for now
}

// ============================================================================
// MIRROR FILTER
// ============================================================================

void mirror_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || (!horizontal && !vertical)) return;
    
    ofFbo temp;
    temp.allocate(fbo.getWidth(), fbo.getHeight(), GL_RGBA);
    
    temp.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);
    fbo.draw(0, 0);
    disableFboCopyBlending();
    temp.end();
    
    fbo.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);
    
    float x = horizontal ? fbo.getWidth() : 0;
    float y = vertical ? fbo.getHeight() : 0;
    float w = horizontal ? -fbo.getWidth() : fbo.getWidth();
    float h = vertical ? -fbo.getHeight() : fbo.getHeight();
    
    temp.draw(x, y, w, h);
    disableFboCopyBlending();
    fbo.end();
}

// ============================================================================
// COLOR ADJUST FILTER
// ============================================================================

void color_adjust_component::apply(ofFbo& fbo) const {
    if (!enabled) return;
    // Requires shader - stub for now
}

bool color_adjust_component::hasAdjustment() const {
    return brightness != 0.0f || contrast != 1.0f || 
           saturation != 1.0f || gamma != 1.0f || hueShift != 0.0f;
}

// ============================================================================
// BLUR FILTER
// ============================================================================

void blur_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || radius <= 0.0f) return;
    // Requires shader - stub for now
}

// ============================================================================
// DITHER FILTER
// ============================================================================

void dither_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || strength <= 0.0f) return;
    // Requires shader - stub for now
}

// ============================================================================
// ROTATE FILTER
// ============================================================================

float rotate_filter_component::getEffectiveAngle() const {
    float angle = angleDegrees;
    if (rotate90CW) angle += 90.0f;
    if (rotate90CCW) angle -= 90.0f;
    if (rotate180) angle += 180.0f;
    return fmod(angle, 360.0f);
}

void rotate_filter_component::apply(ofFbo& fbo) const {
    if (!enabled) return;
    float angle = getEffectiveAngle();
    if (abs(angle) < 0.01f) return;
    
    ofFbo temp;
    temp.allocate(fbo.getWidth(), fbo.getHeight(), GL_RGBA);
    
    temp.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);
    fbo.draw(0, 0);
    disableFboCopyBlending();
    temp.end();
    
    fbo.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);
    ofPushMatrix();
    ofTranslate(fbo.getWidth() / 2, fbo.getHeight() / 2);
    ofRotateDeg(angle);
    temp.draw(-temp.getWidth() / 2, -temp.getHeight() / 2);
    ofPopMatrix();
    disableFboCopyBlending();
    fbo.end();
}

// ============================================================================
// THRESHOLD FILTER
// ============================================================================

void threshold_filter_component::apply(ofFbo& fbo) const {
    if (!enabled) return;
    // Requires shader - stub for now
}

// ============================================================================
// POSTERIZE FILTER
// ============================================================================

void posterize_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || levels >= 256) return;
    // Requires shader - stub for now
}

// ============================================================================
// NOISE FILTER
// ============================================================================

void noise_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || amount <= 0.0f) return;
    // Requires shader - stub for now
}

// ============================================================================
// VIGNETTE FILTER
// ============================================================================

void vignette_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || strength <= 0.0f) return;
    // Requires shader - stub for now
}

// ============================================================================
// CHROMATIC ABERRATION FILTER
// ============================================================================

void chromatic_aberration_component::apply(ofFbo& fbo) const {
    if (!enabled || amount <= 0.0f) return;
    // Requires shader - stub for now
}

} // namespace ecs
