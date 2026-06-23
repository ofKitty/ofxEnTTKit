#include "filter_components.h"
#include "ofGraphics.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace ecs {

namespace {

void enableFboCopyBlending() {
    ofEnableAlphaBlending();
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void disableFboCopyBlending() {
    ofDisableAlphaBlending();
}

unsigned char clampU8(float v) {
    return static_cast<unsigned char>(std::clamp(v, 0.f, 255.f));
}

float luma(unsigned char r, unsigned char g, unsigned char b) {
    return 0.299f * r + 0.587f * g + 0.114f * b;
}

void readFboPixels(ofFbo& fbo, ofPixels& pix) {
    fbo.readToPixels(pix);
    if (pix.getNumChannels() != 4)
        pix.setImageType(OF_IMAGE_COLOR_ALPHA);
}

void writeFboPixels(ofFbo& fbo, const ofPixels& pix) {
    ofImage img;
    img.setFromPixels(pix);
    fbo.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);
    img.draw(0, 0, fbo.getWidth(), fbo.getHeight());
    disableFboCopyBlending();
    fbo.end();
}

void applyColorAdjustPixel(unsigned char& r, unsigned char& g, unsigned char& b,
                           const color_adjust_component& adj) {
    float rf = r, gf = g, bf = b;

    rf = (rf - 128.f) * adj.contrast + 128.f + adj.brightness * 128.f;
    gf = (gf - 128.f) * adj.contrast + 128.f + adj.brightness * 128.f;
    bf = (bf - 128.f) * adj.contrast + 128.f + adj.brightness * 128.f;

    if (adj.saturation != 1.f || adj.hueShift != 0.f) {
        ofColor c(clampU8(rf), clampU8(gf), clampU8(bf));
        float h = c.getHueAngle();
        float s = c.getSaturation();
        float br = c.getBrightness();
        h = std::fmod(h + adj.hueShift + 360.f, 360.f);
        s = std::clamp(s * adj.saturation, 0.f, 255.f);
        c.setHsb(h, s, br);
        rf = c.r;
        gf = c.g;
        bf = c.b;
    }

    if (std::abs(adj.gamma - 1.f) > 0.001f) {
        const float invG = 1.f / std::max(0.01f, adj.gamma);
        rf = std::pow(rf / 255.f, invG) * 255.f;
        gf = std::pow(gf / 255.f, invG) * 255.f;
        bf = std::pow(bf / 255.f, invG) * 255.f;
    }

    r = clampU8(rf);
    g = clampU8(gf);
    b = clampU8(bf);
}

void boxBlurPass(ofPixels& pix, int w, int h, int radius, bool horizontal) {
    if (radius < 1 || w < 3 || h < 3) return;
    const int nc = static_cast<int>(pix.getNumChannels());
    ofPixels out = pix;

    if (horizontal) {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                int sum[4] = {0, 0, 0, 0};
                int n = 0;
                for (int dx = -radius; dx <= radius; ++dx) {
                    const int sx = std::clamp(x + dx, 0, w - 1);
                    const int idx = (y * w + sx) * nc;
                    for (int c = 0; c < nc && c < 4; ++c) {
                        sum[c] += pix[idx + c];
                    }
                    ++n;
                }
                const int dst = (y * w + x) * nc;
                for (int c = 0; c < nc && c < 4; ++c)
                    out[dst + c] = static_cast<unsigned char>(sum[c] / n);
            }
        }
    } else {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                int sum[4] = {0, 0, 0, 0};
                int n = 0;
                for (int dy = -radius; dy <= radius; ++dy) {
                    const int sy = std::clamp(y + dy, 0, h - 1);
                    const int idx = (sy * w + x) * nc;
                    for (int c = 0; c < nc && c < 4; ++c) {
                        sum[c] += pix[idx + c];
                    }
                    ++n;
                }
                const int dst = (y * w + x) * nc;
                for (int c = 0; c < nc && c < 4; ++c)
                    out[dst + c] = static_cast<unsigned char>(sum[c] / n);
            }
        }
    }
    pix = out;
}

static const int kBayer4[4][4] = {
    { 0, 8, 2, 10 },
    { 12, 4, 14, 6 },
    { 3, 11, 1, 9 },
    { 15, 7, 13, 5 },
};

float bayerThreshold(int x, int y, int matrixSize) {
    const int m = std::clamp(matrixSize, 2, 8);
    const int bx = x % m;
    const int by = y % m;
    if (m == 2)
        return ((bx + by) % 2) ? 0.75f : 0.25f;
    if (m == 4)
        return (kBayer4[by][bx] + 0.5f) / 16.f;
    return ((bx * 7 + by * 3) % (m * m) + 0.5f) / float(m * m);
}

void ditherPixel(float& v, int x, int y, const dither_filter_component& d) {
    const int lv = std::max(2, d.levels);
    const float step = 255.f / (lv - 1);
    const float norm = v / 255.f;

    if (d.type == DitherType::Random) {
        const float t = (std::rand() % 1000) / 1000.f;
        v = std::floor(norm * (lv - 1) + t) * step;
        return;
    }

    if (d.type == DitherType::FloydSteinberg) {
        v = std::round(norm * (lv - 1)) * step;
        return;
    }

    int matrix = 4;
    switch (d.type) {
        case DitherType::Ordered2x2:
        case DitherType::Bayer2x2: matrix = 2; break;
        case DitherType::Ordered8x8:
        case DitherType::Bayer8x8: matrix = 8; break;
        default: matrix = 4; break;
    }
    const float thresh = bayerThreshold(x, y, matrix);
    v = (norm > thresh) ? std::min(255.f, step * std::ceil(norm * (lv - 1)))
                        : std::max(0.f, step * std::floor(norm * (lv - 1)));
}

void floydSteinbergPass(std::vector<float>& buf, int w, int h, int levels) {
    const int lv = std::max(2, levels);
    const float step = 255.f / (lv - 1);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int i = y * w + x;
            const float old = buf[i];
            const float neu = std::round(old / step) * step;
            buf[i] = neu;
            const float err = old - neu;
            if (x + 1 < w) buf[i + 1] += err * 7.f / 16.f;
            if (y + 1 < h) {
                if (x > 0) buf[i + w - 1] += err * 3.f / 16.f;
                buf[i + w] += err * 5.f / 16.f;
                if (x + 1 < w) buf[i + w + 1] += err * 1.f / 16.f;
            }
        }
    }
}

void applyEdgeDetectPixels(ofPixels& pix, float strength) {
    const int w = pix.getWidth();
    const int h = pix.getHeight();
    const int nc = static_cast<int>(pix.getNumChannels());
    if (w < 3 || h < 3 || nc < 3) return;

    std::vector<unsigned char> lumaIn(w * h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int idx = (y * w + x) * nc;
            lumaIn[y * w + x] = static_cast<unsigned char>(
                luma(pix[idx], pix[idx + 1], pix[idx + 2]));
        }
    }

    ofPixels out = pix;
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            const int gx =
                -lumaIn[(y - 1) * w + (x - 1)] + lumaIn[(y - 1) * w + (x + 1)]
                - 2 * lumaIn[y * w + (x - 1)] + 2 * lumaIn[y * w + (x + 1)]
                - lumaIn[(y + 1) * w + (x - 1)] + lumaIn[(y + 1) * w + (x + 1)];
            const int gy =
                -lumaIn[(y - 1) * w + (x - 1)] - 2 * lumaIn[(y - 1) * w + x]
                - lumaIn[(y - 1) * w + (x + 1)] + lumaIn[(y + 1) * w + (x - 1)]
                + 2 * lumaIn[(y + 1) * w + x] + lumaIn[(y + 1) * w + (x + 1)];
            const unsigned char edge =
                static_cast<unsigned char>(std::min(255, static_cast<int>(std::sqrt(gx * gx + gy * gy))));

            const int idx = (y * w + x) * nc;
            const float t = std::clamp(strength, 0.f, 1.f);
            for (int c = 0; c < 3; ++c) {
                const float blended = pix[idx + c] * (1.f - t) + edge * t;
                out[idx + c] = clampU8(blended);
            }
        }
    }
    pix = out;
}

} // namespace

void tint_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || strength <= 0.0f) return;

    ofPushStyle();
    fbo.begin();
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);

    ofColor tintColor = color;
    tintColor.a = static_cast<unsigned char>(strength * 255.0f);
    ofSetColor(tintColor);
    ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());

    ofDisableBlendMode();
    fbo.end();
    ofPopStyle();
}

void invert_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || strength <= 0.0f) return;
    if (!invertRGB && !invertAlpha) return;

    ofPixels pix;
    readFboPixels(fbo, pix);
    const int nc = static_cast<int>(pix.getNumChannels());
    const float t = std::clamp(strength, 0.f, 1.f);

    for (size_t i = 0; i + nc - 1 < pix.size(); i += nc) {
        if (invertRGB) {
            for (int c = 0; c < 3 && c < nc; ++c) {
                const float inv = 255.f - pix[i + c];
                pix[i + c] = clampU8(pix[i + c] * (1.f - t) + inv * t);
            }
        }
        if (invertAlpha && nc >= 4) {
            const float inv = 255.f - pix[i + 3];
            pix[i + 3] = clampU8(pix[i + 3] * (1.f - t) + inv * t);
        }
    }
    writeFboPixels(fbo, pix);
}

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

    const float x = horizontal ? fbo.getWidth() : 0.f;
    const float y = vertical ? fbo.getHeight() : 0.f;
    const float w = horizontal ? -fbo.getWidth() : fbo.getWidth();
    const float h = vertical ? -fbo.getHeight() : fbo.getHeight();

    temp.draw(x, y, w, h);
    disableFboCopyBlending();
    fbo.end();
}

void color_adjust_component::apply(ofFbo& fbo) const {
    if (!enabled || !hasAdjustment()) return;

    ofPixels pix;
    readFboPixels(fbo, pix);
    const int nc = static_cast<int>(pix.getNumChannels());
    if (nc < 3) return;

    for (size_t i = 0; i + 2 < pix.size(); i += nc)
        applyColorAdjustPixel(pix[i], pix[i + 1], pix[i + 2], *this);

    writeFboPixels(fbo, pix);
}

bool color_adjust_component::hasAdjustment() const {
    return brightness != 0.0f || contrast != 1.0f ||
           saturation != 1.0f || gamma != 1.0f || hueShift != 0.0f;
}

void blur_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || radius <= 0.0f) return;

    ofPixels pix;
    readFboPixels(fbo, pix);
    const int w = pix.getWidth();
    const int h = pix.getHeight();
    const int r = std::clamp(static_cast<int>(std::round(radius)), 1, 32);
    const int passCount = std::max(1, passes);

    for (int p = 0; p < passCount; ++p) {
        if (horizontal) boxBlurPass(pix, w, h, r, true);
        if (vertical) boxBlurPass(pix, w, h, r, false);
    }
    writeFboPixels(fbo, pix);
}

void dither_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || strength <= 0.0f) return;

    ofPixels pix;
    readFboPixels(fbo, pix);
    const int w = pix.getWidth();
    const int h = pix.getHeight();
    const int nc = static_cast<int>(pix.getNumChannels());
    if (nc < 3) return;

    ofPixels original = pix;
    const float t = std::clamp(strength, 0.f, 1.f);

    if (type == DitherType::FloydSteinberg) {
        if (colorDither) {
            for (int ch = 0; ch < 3; ++ch) {
                std::vector<float> buf(w * h);
                for (int i = 0; i < w * h; ++i)
                    buf[i] = pix[i * nc + ch];
                floydSteinbergPass(buf, w, h, levels);
                for (int i = 0; i < w * h; ++i)
                    pix[i * nc + ch] = clampU8(buf[i]);
            }
        } else {
            std::vector<float> buf(w * h);
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x) {
                    const int idx = (y * w + x) * nc;
                    buf[y * w + x] = luma(pix[idx], pix[idx + 1], pix[idx + 2]);
                }
            floydSteinbergPass(buf, w, h, levels);
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x) {
                    const unsigned char v = clampU8(buf[y * w + x]);
                    const int idx = (y * w + x) * nc;
                    pix[idx] = pix[idx + 1] = pix[idx + 2] = v;
                }
        }
    } else {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const int idx = (y * w + x) * nc;
                if (colorDither) {
                    for (int ch = 0; ch < 3; ++ch) {
                        float v = pix[idx + ch];
                        ditherPixel(v, x, y, *this);
                        pix[idx + ch] = clampU8(v);
                    }
                } else {
                    float v = luma(pix[idx], pix[idx + 1], pix[idx + 2]);
                    ditherPixel(v, x, y, *this);
                    const unsigned char g = clampU8(v);
                    pix[idx] = pix[idx + 1] = pix[idx + 2] = g;
                }
            }
        }
    }

    if (t < 1.f) {
        for (size_t i = 0; i < pix.size(); ++i)
            pix[i] = clampU8(original[i] * (1.f - t) + pix[i] * t);
    }
    writeFboPixels(fbo, pix);
}

float rotate_filter_component::getEffectiveAngle() const {
    float angle = angleDegrees;
    if (rotate90CW) angle += 90.0f;
    if (rotate90CCW) angle -= 90.0f;
    if (rotate180) angle += 180.0f;
    return std::fmod(angle, 360.0f);
}

namespace {

void copyFboToFbo(ofFbo& dst, const ofFbo& src, int w, int h)
{
    if (!dst.isAllocated() || dst.getWidth() != w || dst.getHeight() != h)
        dst.allocate(w, h, GL_RGBA);
    dst.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);
    src.draw(0, 0, w, h);
    disableFboCopyBlending();
    dst.end();
}

void rotateFbo90CW(ofFbo& fbo)
{
    const int w = fbo.getWidth();
    const int h = fbo.getHeight();
    if (w <= 0 || h <= 0) return;

    ofFbo temp;
    copyFboToFbo(temp, fbo, w, h);

    fbo.allocate(h, w, GL_RGBA);
    fbo.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);
    ofPushMatrix();
    ofTranslate(0, w);
    ofRotateDeg(-90);
    temp.draw(0, 0, w, h);
    ofPopMatrix();
    disableFboCopyBlending();
    fbo.end();
}

void mirrorFbo(ofFbo& fbo, bool horizontal, bool vertical)
{
    if (!horizontal && !vertical) return;
    const int w = fbo.getWidth();
    const int h = fbo.getHeight();
    if (w <= 0 || h <= 0) return;

    ofFbo temp;
    copyFboToFbo(temp, fbo, w, h);

    fbo.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);
    ofPushMatrix();
    ofTranslate(horizontal ? w : 0.f, vertical ? h : 0.f);
    ofScale(horizontal ? -1.f : 1.f, vertical ? -1.f : 1.f);
    temp.draw(0, 0, w, h);
    ofPopMatrix();
    disableFboCopyBlending();
    fbo.end();
}

} // namespace

void rotate_filter_component::apply(ofFbo& fbo) const {
    if (!enabled) return;

    const int steps = std::clamp(rotate90Count, 0, 3);
    for (int i = 0; i < steps; ++i)
        rotateFbo90CW(fbo);

    mirrorFbo(fbo, horizontalMirror, verticalMirror);

    const float angle = getEffectiveAngle();
    if (std::abs(angle) < 0.01f) return;

    const int w = fbo.getWidth();
    const int h = fbo.getHeight();
    if (w <= 0 || h <= 0) return;

    ofFbo temp;
    copyFboToFbo(temp, fbo, w, h);

    fbo.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);
    ofPushMatrix();
    ofTranslate(w / 2.f, h / 2.f);
    ofRotateDeg(angle);
    temp.draw(-w / 2.f, -h / 2.f, w, h);
    ofPopMatrix();
    disableFboCopyBlending();
    fbo.end();
}

void duplicate_filter_component::apply(ofFbo& fbo) const {
    if (!enabled) return;

    const int hc = std::max(1, hCount);
    const int vc = std::max(1, vCount);
    const int w = fbo.getWidth();
    const int h = fbo.getHeight();
    if (w <= 0 || h <= 0) return;

    ofFbo temp;
    copyFboToFbo(temp, fbo, w, h);

    const float tileW = static_cast<float>(w) / hc;
    const float tileH = static_cast<float>(h) / vc;

    fbo.begin();
    ofClear(0, 0, 0, 0);
    enableFboCopyBlending();
    ofSetColor(255);

    for (int y = 0; y < vc; ++y) {
        ofPushMatrix();
        ofTranslate(0.f, y * tileH);
        for (int x = 0; x < hc; ++x) {
            ofPushMatrix();
            ofTranslate(x * tileW, 0.f);
            ofTranslate(tileW * 0.5f, tileH * 0.5f);
            if (mirror && (x % 2 == 1)) ofScale(-1.f, 1.f);
            if (mirror && (y % 2 == 1)) ofScale(1.f, -1.f);
            temp.draw(-tileW * 0.5f, -tileH * 0.5f, tileW, tileH);
            ofPopMatrix();
        }
        ofPopMatrix();
    }

    disableFboCopyBlending();
    fbo.end();
}

void threshold_filter_component::apply(ofFbo& fbo) const {
    if (!enabled) return;

    ofPixels pix;
    readFboPixels(fbo, pix);
    const int nc = static_cast<int>(pix.getNumChannels());
    if (nc < 3) return;

    const float t = std::clamp(threshold, 0.f, 1.f);
    for (size_t i = 0; i + 2 < pix.size(); i += nc) {
        const float lum = luma(pix[i], pix[i + 1], pix[i + 2]) / 255.f;
        const bool above = invert ? (lum < t) : (lum >= t);
        const ofColor& out = above ? highColor : lowColor;
        pix[i] = out.r;
        pix[i + 1] = out.g;
        pix[i + 2] = out.b;
        if (nc >= 4) pix[i + 3] = out.a;
    }
    writeFboPixels(fbo, pix);
}

void posterize_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || levels >= 256) return;

    ofPixels pix;
    readFboPixels(fbo, pix);
    const int nc = static_cast<int>(pix.getNumChannels());
    const int lv = std::max(2, levels);
    const float step = 255.f / (lv - 1);

    for (size_t i = 0; i < pix.size(); ++i) {
        if (nc >= 4 && (i % nc) == 3) continue;
        const float v = pix[i];
        pix[i] = clampU8(std::round(v / step) * step);
    }
    writeFboPixels(fbo, pix);
}

void noise_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || amount <= 0.0f) return;

    ofPixels pix;
    readFboPixels(fbo, pix);
    const int nc = static_cast<int>(pix.getNumChannels());
    const float amp = amount * 255.f;
    unsigned int rng = static_cast<unsigned int>(seed);
    if (animated)
        rng ^= static_cast<unsigned int>(ofGetElapsedTimef() * 1000.f);

    auto nextRand = [&rng]() -> float {
        rng = rng * 1664525u + 1013904223u;
        return (rng & 0xffff) / 65535.f * 2.f - 1.f;
    };

    for (size_t i = 0; i < pix.size(); i += nc) {
        const float n = nextRand() * amp;
        if (monochrome) {
            for (int c = 0; c < 3 && c < nc; ++c)
                pix[i + c] = clampU8(pix[i + c] + n);
        } else {
            for (int c = 0; c < 3 && c < nc; ++c)
                pix[i + c] = clampU8(pix[i + c] + nextRand() * amp);
        }
    }
    writeFboPixels(fbo, pix);
}

void vignette_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || strength <= 0.0f) return;

    ofPixels pix;
    readFboPixels(fbo, pix);
    const int w = pix.getWidth();
    const int h = pix.getHeight();
    const int nc = static_cast<int>(pix.getNumChannels());
    if (nc < 3) return;

    const float cx = (w - 1) * 0.5f;
    const float cy = (h - 1) * 0.5f;
    const float maxDist = std::sqrt(cx * cx + cy * cy);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const float dx = x - cx;
            const float dy = y - cy;
            const float dist = std::sqrt(dx * dx + dy * dy) / std::max(1.f, maxDist);
            float v = (dist - radius) / std::max(0.001f, softness);
            v = std::clamp(v, 0.f, 1.f) * strength;

            const int idx = (y * w + x) * nc;
            pix[idx]     = clampU8(pix[idx]     * (1.f - v) + color.r * v);
            pix[idx + 1] = clampU8(pix[idx + 1] * (1.f - v) + color.g * v);
            pix[idx + 2] = clampU8(pix[idx + 2] * (1.f - v) + color.b * v);
        }
    }
    writeFboPixels(fbo, pix);
}

void chromatic_aberration_component::apply(ofFbo& fbo) const {
    if (!enabled || amount <= 0.0f) return;

    ofPixels src;
    readFboPixels(fbo, src);
    const int w = src.getWidth();
    const int h = src.getHeight();
    const int nc = static_cast<int>(src.getNumChannels());
    if (nc < 4 || w < 2 || h < 2) return;

    ofPixels out = src;
    const float cx = (w - 1) * 0.5f;
    const float cy = (h - 1) * 0.5f;
    const float rad = amount;
    const float ang = angle * 3.14159265f / 180.f;
    const float dirX = std::cos(ang);
    const float dirY = std::sin(ang);

    auto sample = [&](float sx, float sy, int channel) -> unsigned char {
        sx = std::clamp(sx, 0.f, w - 1.f);
        sy = std::clamp(sy, 0.f, h - 1.f);
        const int x = static_cast<int>(sx);
        const int y = static_cast<int>(sy);
        return src[(y * w + x) * nc + channel];
    };

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float ox = dirX * rad;
            float oy = dirY * rad;
            if (radial) {
                const float dx = (x - cx) / std::max(1.f, cx);
                const float dy = (y - cy) / std::max(1.f, cy);
                const float d = std::sqrt(dx * dx + dy * dy);
                ox *= d;
                oy *= d;
            }
            const int idx = (y * w + x) * nc;
            out[idx]     = sample(x - ox, y - oy, 0);
            out[idx + 1] = sample(x, y, 1);
            out[idx + 2] = sample(x + ox, y + oy, 2);
        }
    }
    writeFboPixels(fbo, out);
}

void edge_detect_filter_component::apply(ofFbo& fbo) const {
    if (!enabled || strength <= 0.0f) return;

    ofPixels pix;
    readFboPixels(fbo, pix);
    applyEdgeDetectPixels(pix, strength);
    writeFboPixels(fbo, pix);
}

} // namespace ecs
