#include "graphics2d_components.h"

namespace ecs {

// ============================================================================
// path_component
// ============================================================================

void path_component::updateColors() {
    if (filled) {
        path.setFilled(true);
        path.setFillColor(fillColor);
    } else {
        path.setFilled(false);
    }
    
    if (stroked) {
        path.setStrokeWidth(strokeWidth);
        path.setStrokeColor(strokeColor);
    } else {
        path.setStrokeWidth(0);
    }
}

// ============================================================================
// polyline_component
// ============================================================================

void polyline_component::drawVertices() {
    ofPushStyle();
    ofSetColor(color);
    for (const auto& v : polyline.getVertices()) {
        ofDrawCircle(v, lineWidth * 2);
    }
    ofPopStyle();
}

// ============================================================================
// Vesica Piscis Component
// ============================================================================

void vesica_piscis_component::getCircleCenters(glm::vec2& c1, glm::vec2& c2, float& radius) const {
    glm::vec2 cent(x + width/2, y + height/2);
    glm::vec2 midLeft(x, y + height/2);
    glm::vec2 topCenter(x + width/2, y);
    glm::vec2 midRight(x + width, y + height/2);
    
    glm::vec2 m1 = (midLeft + topCenter) * 0.5f;
    glm::vec2 m2 = (topCenter + midRight) * 0.5f;
    
    float p1_angle = atan2(topCenter.y - midLeft.y, topCenter.x - midLeft.x) + HALF_PI;
    float p2_angle = atan2(midRight.y - topCenter.y, midRight.x - topCenter.x) + HALF_PI;
    
    float dx1 = cos(p1_angle);
    float dy1 = sin(p1_angle);
    float dx2 = cos(p2_angle);
    float dy2 = sin(p2_angle);
    
    float det = dx1 * dy2 - dy1 * dx2;
    if (fabs(det) < 0.0001f) {
        c1 = cent + glm::vec2(0, -height * 0.3f);
        c2 = cent + glm::vec2(0, height * 0.3f);
        radius = width * 0.8f;
        return;
    }
    
    float t = ((m2.x - m1.x) * dy2 - (m2.y - m1.y) * dx2) / det;
    
    c1 = glm::vec2(m1.x + t * dx1, m1.y + t * dy1);
    c2 = glm::vec2(c1.x, -c1.y + height + 2 * y);
    radius = glm::distance(topCenter, c1);
}

// ============================================================================
// Soft Mask Component
// ============================================================================

void soft_mask_component::begin() {
    if (!maskFbo.isAllocated()) return;
    
    maskFbo.begin();
    ofClear(0, 0, 0, 0);
}

void soft_mask_component::end() {
    if (!maskFbo.isAllocated()) return;
    maskFbo.end();
}

void soft_mask_component::apply(ofFbo& targetFbo) {
    if (!maskFbo.isAllocated() || !targetFbo.isAllocated()) return;
    
    if (!maskShader.isLoaded()) {
        maskShader.load("shaders/mask");
    }
    
    if (maskShader.isLoaded()) {
        maskShader.begin();
        maskShader.setUniformTexture("maskTex", maskFbo.getTexture(), 1);
        maskShader.setUniform1i("invertMask", inverted ? 1 : 0);
        maskShader.setUniform1f("softness", softness);
        targetFbo.draw(0, 0);
        maskShader.end();
    }
}

} // namespace ecs
