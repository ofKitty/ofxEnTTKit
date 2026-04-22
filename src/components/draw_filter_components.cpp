#include "draw_filter_components.h"

namespace ecs {

// ============================================================================
// PIXELATE FILTER
// ============================================================================

pixelate_filter_component::pixelate_filter_component(int tx, int ty, PixelStyle s)
    : tilesX(tx), tilesY(ty), style(s) {}

static void drawCMYKPixel(float x, float y, float w, float h, const ofColor& c,
                          const ofColor& cyan, const ofColor& magenta, const ofColor& yellow, const ofColor& black) {
    float r = c.r / 255.0f;
    float g = c.g / 255.0f;
    float b = c.b / 255.0f;
    
    float k = 1.0f - std::max({r, g, b});
    float cyanVal = (1.0f - r - k) / (1.0f - k + 0.001f);
    float magentaVal = (1.0f - g - k) / (1.0f - k + 0.001f);
    float yellowVal = (1.0f - b - k) / (1.0f - k + 0.001f);
    
    float maxR = std::min(w, h) / 2;
    float offset = maxR * 0.3f;
    float cx = x + w / 2;
    float cy = y + h / 2;
    
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    ofSetColor(cyan, 200);
    ofDrawCircle(cx - offset, cy, maxR * cyanVal);
    ofSetColor(magenta, 200);
    ofDrawCircle(cx + offset, cy - offset, maxR * magentaVal);
    ofSetColor(yellow, 200);
    ofDrawCircle(cx + offset, cy + offset, maxR * yellowVal);
    ofSetColor(black, 200);
    ofDrawCircle(cx, cy, maxR * k);
    ofDisableBlendMode();
}

static void drawRGBPixel(float x, float y, float w, float h, const ofColor& c) {
    float maxR = std::min(w, h) / 2;
    float offset = maxR * 0.3f;
    float cx = x + w / 2;
    float cy = y + h / 2;
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetColor(c.r, 0, 0, 200);
    ofDrawCircle(cx - offset, cy, maxR * (c.r / 255.0f));
    ofSetColor(0, c.g, 0, 200);
    ofDrawCircle(cx + offset, cy - offset, maxR * (c.g / 255.0f));
    ofSetColor(0, 0, c.b, 200);
    ofDrawCircle(cx + offset, cy + offset, maxR * (c.b / 255.0f));
    ofDisableBlendMode();
}

void pixelate_filter_component::draw(ofImage& source, float width, float height) const {
    if (!enabled || tilesX <= 0 || tilesY <= 0) return;
    
    float cellW = width / tilesX;
    float cellH = height / tilesY;
    float srcCellW = source.getWidth() / tilesX;
    float srcCellH = source.getHeight() / tilesY;
    
    ofPushStyle();
    ofFill();
    
    for (int y = 0; y < tilesY; y++) {
        float rowOffset = (polka && y % 2 == 1) ? cellW * 0.5f : 0;
        
        for (int x = 0; x < tilesX; x++) {
            int srcX = ofClamp(x * srcCellW + srcCellW / 2, 0, source.getWidth() - 1);
            int srcY = ofClamp(y * srcCellH + srcCellH / 2, 0, source.getHeight() - 1);
            ofColor c = source.getColor(srcX, srcY);
            
            float px = x * cellW + rowOffset;
            float py = y * cellH;
            float pw = cellW * (1.0f - gapRatio);
            float ph = cellH * (1.0f - gapRatio);
            
            ofSetColor(c);
            
            switch (style) {
                case PixelStyle::Rectangle:
                    if (rounded > 0.01f) {
                        ofDrawRectRounded(px, py, pw, ph, std::min(pw, ph) * rounded * 0.5f);
                    } else {
                        ofDrawRectangle(px, py, pw, ph);
                    }
                    break;
                case PixelStyle::Circle:
                    ofDrawCircle(px + pw / 2, py + ph / 2, std::min(pw, ph) / 2);
                    break;
                case PixelStyle::Diamond:
                    {
                        ofPath path;
                        path.moveTo(px + pw / 2, py);
                        path.lineTo(px + pw, py + ph / 2);
                        path.lineTo(px + pw / 2, py + ph);
                        path.lineTo(px, py + ph / 2);
                        path.close();
                        path.setFillColor(c);
                        path.draw();
                    }
                    break;
                case PixelStyle::Halftone:
                    {
                        float brightness = c.getBrightness() / 255.0f;
                        float radius = std::min(pw, ph) / 2 * (1.0f - brightness);
                        if (radius > 0.5f) {
                            ofSetColor(0);
                            ofDrawCircle(px + pw / 2, py + ph / 2, radius);
                        }
                    }
                    break;
                case PixelStyle::CMYK:
                    drawCMYKPixel(px, py, pw, ph, c, cyanColor, magentaColor, yellowColor, blackColor);
                    break;
                case PixelStyle::RGB:
                    drawRGBPixel(px, py, pw, ph, c);
                    break;
                default:
                    ofDrawRectangle(px, py, pw, ph);
                    break;
            }
        }
    }
    
    ofPopStyle();
}

// ============================================================================
// MESH FILTER
// ============================================================================

mesh_filter_component::mesh_filter_component(int gx, int gy, float lw)
    : gridX(gx), gridY(gy), lineWidth(lw) {}

void mesh_filter_component::draw(ofImage& source, float width, float height) const {
    if (!enabled) return;
    
    ofPushStyle();
    
    if (drawBackground) {
        ofSetColor(backgroundColor);
        ofFill();
        ofDrawRectangle(0, 0, width, height);
    }
    
    ofSetLineWidth(lineWidth);
    
    float cellW = width / gridX;
    float cellH = height / gridY;
    float srcCellW = source.getWidth() / gridX;
    float srcCellH = source.getHeight() / gridY;
    
    if (drawVertical) {
        for (int x = 0; x <= gridX; x++) {
            for (int y = 0; y < gridY; y++) {
                int srcX = ofClamp(x * srcCellW, 0, source.getWidth() - 1);
                int srcY = ofClamp(y * srcCellH + srcCellH / 2, 0, source.getHeight() - 1);
                ofSetColor(colorFromImage ? source.getColor(srcX, srcY) : lineColor);
                ofDrawLine(x * cellW, y * cellH, x * cellW, (y + 1) * cellH);
            }
        }
    }
    
    if (drawHorizontal) {
        for (int y = 0; y <= gridY; y++) {
            for (int x = 0; x < gridX; x++) {
                int srcX = ofClamp(x * srcCellW + srcCellW / 2, 0, source.getWidth() - 1);
                int srcY = ofClamp(y * srcCellH, 0, source.getHeight() - 1);
                ofSetColor(colorFromImage ? source.getColor(srcX, srcY) : lineColor);
                ofDrawLine(x * cellW, y * cellH, (x + 1) * cellW, y * cellH);
            }
        }
    }
    
    ofPopStyle();
}

// ============================================================================
// RINGS FILTER
// ============================================================================

rings_filter_component::rings_filter_component(int count, float lw)
    : ringCount(count), lineWidth(lw) {}

void rings_filter_component::draw(ofImage& source, float width, float height) const {
    if (!enabled || ringCount <= 0) return;
    
    ofPushStyle();
    
    float cx = center.x * width;
    float cy = center.y * height;
    float maxRadius = sqrt(width * width + height * height);
    float ringSpacing = maxRadius / ringCount;
    
    if (filled) ofFill();
    else { ofNoFill(); ofSetLineWidth(lineWidth); }
    
    for (int i = 0; i < ringCount; i++) {
        float radius = (i + 1) * ringSpacing;
        
        if (colorFromImage) {
            int srcX = ofClamp(cx * source.getWidth() / width, 0, source.getWidth() - 1);
            int srcY = ofClamp(cy * source.getHeight() / height, 0, source.getHeight() - 1);
            ofSetColor(source.getColor(srcX, srcY));
        } else {
            ofSetColor(ringColor);
        }
        
        ofDrawCircle(cx, cy, radius);
    }
    
    ofPopStyle();
}

// ============================================================================
// NOISE DISPLACEMENT
// ============================================================================

noise_displacement_component::noise_displacement_component(float s, float amp)
    : scale(s), amplitude(amp) {}

void noise_displacement_component::update(float dt) {
    timeOffset += dt * speed;
}

void noise_displacement_component::draw(ofImage& source, float width, float height) const {
    if (!enabled) return;
    source.draw(0, 0, width, height);  // Shader needed for proper implementation
}

// ============================================================================
// LINE SCAN FILTER
// ============================================================================

line_scan_filter_component::line_scan_filter_component(int count, float lw, bool vert)
    : vertical(vert), lineCount(count), lineWidth(lw) {}

void line_scan_filter_component::draw(ofImage& source, float width, float height) const {
    if (!enabled || lineCount <= 0) return;
    
    ofPushStyle();
    
    ofSetColor(backgroundColor);
    ofFill();
    ofDrawRectangle(0, 0, width, height);
    
    float spacing = (vertical ? width : height) / lineCount;
    
    for (int i = 0; i < lineCount; i++) {
        float pos = i * spacing + spacing / 2;
        
        ofColor c;
        if (vertical) {
            int srcX = ofClamp(pos * source.getWidth() / width, 0, source.getWidth() - 1);
            c = source.getColor(srcX, source.getHeight() / 2);
        } else {
            int srcY = ofClamp(pos * source.getHeight() / height, 0, source.getHeight() - 1);
            c = source.getColor(source.getWidth() / 2, srcY);
        }
        
        float lw = lineWidth;
        if (sizeFromBrightness) {
            float brightness = c.getBrightness() / 255.0f;
            lw = minLineWidth + brightness * (maxLineWidth - minLineWidth);
        }
        
        ofSetColor(colorFromImage ? c : lineColor);
        ofSetLineWidth(lw);
        
        if (vertical) ofDrawLine(pos, 0, pos, height);
        else ofDrawLine(0, pos, width, pos);
    }
    
    ofPopStyle();
}

// ============================================================================
// ASCII FILTER
// ============================================================================

ascii_filter_component::ascii_filter_component(int cx, int cy, float size)
    : charsX(cx), charsY(cy), fontSize(size) {}

char ascii_filter_component::getCharForBrightness(float brightness) const {
    if (charset.empty()) return ' ';
    int index = ofClamp(brightness * (charset.length() - 1), 0, charset.length() - 1);
    if (invert) index = charset.length() - 1 - index;
    return charset[index];
}

void ascii_filter_component::draw(ofImage& source, float width, float height, ofTrueTypeFont& font) const {
    if (!enabled || charsX <= 0 || charsY <= 0) return;
    
    ofPushStyle();
    
    ofSetColor(backgroundColor);
    ofFill();
    ofDrawRectangle(0, 0, width, height);
    
    float cellW = width / charsX;
    float cellH = height / charsY;
    float srcCellW = source.getWidth() / charsX;
    float srcCellH = source.getHeight() / charsY;
    
    for (int y = 0; y < charsY; y++) {
        for (int x = 0; x < charsX; x++) {
            int srcX = ofClamp(x * srcCellW + srcCellW / 2, 0, source.getWidth() - 1);
            int srcY = ofClamp(y * srcCellH + srcCellH / 2, 0, source.getHeight() - 1);
            ofColor c = source.getColor(srcX, srcY);
            
            float brightness = c.getBrightness() / 255.0f;
            char ch = getCharForBrightness(brightness);
            
            ofSetColor(colorFromImage ? c : textColor);
            
            std::string s(1, ch);
            font.drawString(s, x * cellW, (y + 1) * cellH);
        }
    }
    
    ofPopStyle();
}

} // namespace ecs
