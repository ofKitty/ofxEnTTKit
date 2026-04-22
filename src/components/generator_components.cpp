#include "generator_components.h"

namespace ecs {

// ============================================================================
// GRADIENT GENERATOR
// ============================================================================

gradient_generator_component::gradient_generator_component(const ofColor& start, const ofColor& end, GradientDirection dir)
    : direction(dir), colorStart(start), colorEnd(end) {}

ofColor gradient_generator_component::getColorAt(float t) const {
    t = ofClamp(t, 0.0f, 1.0f);
    
    if (stops.empty()) {
        return colorStart.getLerped(colorEnd, t);
    }
    
    if (stops.size() == 1) return stops[0].color;
    
    for (size_t i = 0; i < stops.size() - 1; i++) {
        if (t >= stops[i].position && t <= stops[i + 1].position) {
            float range = stops[i + 1].position - stops[i].position;
            if (range < 0.0001f) return stops[i].color;
            float localT = (t - stops[i].position) / range;
            return stops[i].color.getLerped(stops[i + 1].color, localT);
        }
    }
    return stops.back().color;
}

void gradient_generator_component::draw(float width, float height) const {
    if (!enabled) return;
    
    ofPushStyle();
    ofFill();
    
    if (direction == GradientDirection::Vertical) {
        float stepHeight = height / numSteps;
        for (int i = 0; i < numSteps; i++) {
            float t = (float)i / (numSteps - 1);
            ofSetColor(getColorAt(t));
            ofDrawRectangle(0, i * stepHeight, width, stepHeight + 1);
        }
    } else if (direction == GradientDirection::Horizontal) {
        float stepWidth = width / numSteps;
        for (int i = 0; i < numSteps; i++) {
            float t = (float)i / (numSteps - 1);
            ofSetColor(getColorAt(t));
            ofDrawRectangle(i * stepWidth, 0, stepWidth + 1, height);
        }
    } else if (direction == GradientDirection::Radial) {
        float maxRadius = sqrt(width * width + height * height) / 2;
        float cx = width / 2;
        float cy = height / 2;
        for (int i = numSteps - 1; i >= 0; i--) {
            float t = (float)i / (numSteps - 1);
            float radius = maxRadius * (1.0f - t);
            ofSetColor(getColorAt(t));
            ofDrawCircle(cx, cy, radius);
        }
    } else {
        // Default to vertical
        float stepHeight = height / numSteps;
        for (int i = 0; i < numSteps; i++) {
            float t = (float)i / (numSteps - 1);
            ofSetColor(getColorAt(t));
            ofDrawRectangle(0, i * stepHeight, width, stepHeight + 1);
        }
    }
    
    ofPopStyle();
}

// ============================================================================
// DOTS GENERATOR
// ============================================================================

dots_generator_component::dots_generator_component(int cx, int cy, float size)
    : countX(cx), countY(cy), dotSize(size) {}

void dots_generator_component::draw(float width, float height) const {
    if (!enabled) return;
    
    ofPushStyle();
    
    ofSetColor(bgColor);
    ofFill();
    ofDrawRectangle(0, 0, width, height);
    
    float cellW = width / countX;
    float cellH = height / countY;
    
    ofSetColor(dotColor);
    if (filled) {
        ofFill();
    } else {
        ofNoFill();
        ofSetLineWidth(strokeWeight);
    }
    
    for (int y = 0; y < countY; y++) {
        float rowOffset = (offset && y % 2 == 1) ? cellW * 0.5f : 0;
        
        for (int x = 0; x < countX; x++) {
            float cx = x * cellW + cellW / 2 + rowOffset;
            float cy = y * cellH + cellH / 2;
            
            float size = dotSize;
            if (randomSize) {
                float seed = sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
                seed = seed - floor(seed);
                size = randomSizeMin + seed * (randomSizeMax - randomSizeMin);
            }
            
            ofDrawCircle(cx, cy, size / 2);
        }
    }
    
    ofPopStyle();
}

// ============================================================================
// STRIPES GENERATOR
// ============================================================================

stripes_generator_component::stripes_generator_component(int c, float w, bool vert)
    : vertical(vert), count(c), stripeWidth(w) {}

void stripes_generator_component::draw(float width, float height) const {
    if (!enabled) return;
    
    ofPushStyle();
    ofPushMatrix();
    
    if (abs(angle) > 0.01f) {
        ofTranslate(width / 2, height / 2);
        ofRotateDeg(angle);
        ofTranslate(-width / 2, -height / 2);
    }
    
    ofFill();
    
    float totalSize = vertical ? width : height;
    float pairSize = totalSize / count;
    float offsetPixels = offset * pairSize;
    
    ofSetColor(color1);
    ofDrawRectangle(-pairSize, -pairSize, width + pairSize * 2, height + pairSize * 2);
    
    ofSetColor(color2);
    
    int numStripes = count + 2;
    for (int i = -1; i < numStripes; i++) {
        float pos = i * pairSize + offsetPixels;
        
        if (vertical) {
            ofDrawRectangle(pos, -pairSize, stripeWidth, height + pairSize * 2);
        } else {
            ofDrawRectangle(-pairSize, pos, width + pairSize * 2, stripeWidth);
        }
    }
    
    ofPopMatrix();
    ofPopStyle();
}

// ============================================================================
// CHECKERBOARD GENERATOR
// ============================================================================

checkerboard_generator_component::checkerboard_generator_component(int cx, int cy)
    : countX(cx), countY(cy) {}

void checkerboard_generator_component::draw(float width, float height) const {
    if (!enabled) return;
    
    ofPushStyle();
    ofFill();
    
    float cellW = width / countX;
    float cellH = height / countY;
    
    for (int y = 0; y < countY; y++) {
        for (int x = 0; x < countX; x++) {
            bool isEven = (x + y) % 2 == 0;
            ofSetColor(isEven ? color1 : color2);
            ofDrawRectangle(x * cellW, y * cellH, cellW + 1, cellH + 1);
        }
    }
    
    ofPopStyle();
}

// ============================================================================
// NOISE GENERATOR
// ============================================================================

noise_generator_component::noise_generator_component(float s, int oct)
    : scale(s), octaves(oct) {}

void noise_generator_component::draw(float width, float height) const {
    if (!enabled) return;
    
    ofPushStyle();
    
    int step = 4;
    for (int y = 0; y < height; y += step) {
        for (int x = 0; x < width; x += step) {
            float n = ofNoise(x * scale, y * scale, timeOffset);
            ofColor c = colorLow.getLerped(colorHigh, n);
            ofSetColor(c);
            ofDrawRectangle(x, y, step, step);
        }
    }
    
    ofPopStyle();
}

void noise_generator_component::update(float dt) {
    timeOffset += dt * speed;
}

} // namespace ecs
