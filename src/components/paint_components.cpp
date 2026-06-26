#include "paint_components.h"

namespace ecs {

namespace {

// glm::vec4 colours are normalized linear RGBA (0..1). Convert at framework
// boundaries only. HSV interpolation reuses ofColor's proven shortest-arc
// algorithm (hue range 0..255), so round-trip through ofColor for that path.
inline ofFloatColor toOf(const glm::vec4& c) { return ofFloatColor(c.x, c.y, c.z, c.w); }
inline ofColor      toOf255(const glm::vec4& c) {
    return ofColor(c.x * 255.0f, c.y * 255.0f, c.z * 255.0f, c.w * 255.0f);
}
inline glm::vec4    fromOf255(const ofColor& c) {
    return glm::vec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
}

glm::vec4 lerpRGB(const glm::vec4& a, const glm::vec4& b, float t) {
    return glm::mix(a, b, t);   // linear per-channel, including alpha
}

glm::vec4 lerpHSV(const glm::vec4& a, const glm::vec4& b, float t) {
    const ofColor ca = toOf255(a);
    const ofColor cb = toOf255(b);
    float ah, as, av, bh, bs, bv;
    ca.getHsb(ah, as, av);
    cb.getHsb(bh, bs, bv);

    // Interpolate hue along the shortest arc (ofColor hue is 0..255).
    float dh = bh - ah;
    if (dh > 127.5f)  dh -= 255.0f;
    if (dh < -127.5f) dh += 255.0f;
    float h = ah + dh * t;
    if (h < 0.0f)   h += 255.0f;
    if (h > 255.0f) h -= 255.0f;

    glm::vec4 out = fromOf255(ofColor::fromHsb(h, ofLerp(as, bs, t), ofLerp(av, bv, t)));
    out.w = ofLerp(a.w, b.w, t);   // alpha in normalized space
    return out;
}

glm::vec4 lerpStop(const gradient_component& g, const glm::vec4& a, const glm::vec4& b, float t) {
    return g.interp == GradientInterpolation::HSV ? lerpHSV(a, b, t) : lerpRGB(a, b, t);
}

} // namespace

glm::vec4 gradient_component::sample(float t) const {
    if (stops.empty())     return glm::vec4(1.0f);
    if (stops.size() == 1) return stops.front().color;

    switch (spread) {
        case GradientSpread::Repeat:
            t = t - std::floor(t);
            break;
        case GradientSpread::Mirror: {
            float tt = std::fmod(std::fabs(t), 2.0f);
            t = (tt > 1.0f) ? (2.0f - tt) : tt;
            break;
        }
        case GradientSpread::Pad:
        default:
            t = ofClamp(t, 0.0f, 1.0f);
            break;
    }

    if (t <= stops.front().position) return stops.front().color;
    if (t >= stops.back().position)  return stops.back().color;

    for (size_t i = 0; i + 1 < stops.size(); ++i) {
        const GradientStop& a = stops[i];
        const GradientStop& b = stops[i + 1];
        if (t >= a.position && t <= b.position) {
            float range  = b.position - a.position;
            float localT = (range < 1e-5f) ? 0.0f : (t - a.position) / range;
            return lerpStop(*this, a.color, b.color, localT);
        }
    }
    return stops.back().color;
}

void gradient_component::sortStops() {
    std::sort(stops.begin(), stops.end(),
              [](const GradientStop& a, const GradientStop& b) { return a.position < b.position; });
}

void gradient_component::addStop(float position, const glm::vec4& color) {
    stops.push_back(GradientStop(position, color));
    sortStops();
}

void gradient_component::removeStop(int index) {
    if ((int)stops.size() > 2 && index >= 0 && index < (int)stops.size()) {
        stops.erase(stops.begin() + index);
    }
}

void gradient_component::reverse() {
    for (auto& s : stops) s.position = 1.0f - s.position;
    std::reverse(stops.begin(), stops.end());
}

gradient_component gradient_component::fromColors(const glm::vec4& start, const glm::vec4& end,
                                                  const std::string& name) {
    gradient_component g(name.empty() ? "Gradient" : name);
    g.stops.clear();
    g.stops.push_back(GradientStop(0.0f, start));
    g.stops.push_back(GradientStop(1.0f, end));
    return g;
}

// ----------------------------------------------------------------------------
// Free helpers
// ----------------------------------------------------------------------------

bool resolvePaintColor(const entt::registry& reg, entt::entity paint, glm::vec4& out) {
    if (paint == entt::null || !reg.valid(paint)) return false;
    if (const auto* s = reg.try_get<solid_color_component>(paint)) {
        out = s->color;
        return true;
    }
    if (const auto* g = reg.try_get<gradient_component>(paint)) {
        out = g->sample(0.5f);
        return true;
    }
    return false;
}

ofMesh buildGradientMesh(const gradient_component& g, float width, float height) {
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);

    if (g.type == GradientType::Radial) {
        const int   segments = 64;
        const float maxDim   = std::max(width, height);
        const glm::vec2 c(g.center.x * width, g.center.y * height);
        const float outer = g.outerRadius * maxDim;

        // Sample radius into rings so multi-stop radial gradients render.
        const int rings = std::max(2, g.numSteps > 0 ? g.numSteps : 32);
        for (int r = 0; r < rings; ++r) {
            float t0 = (float)r / rings;
            float t1 = (float)(r + 1) / rings;
            ofFloatColor col0 = toOf(g.sample(t0));
            ofFloatColor col1 = toOf(g.sample(t1));
            float rad0 = ofLerp(g.innerRadius * maxDim, outer, t0);
            float rad1 = ofLerp(g.innerRadius * maxDim, outer, t1);
            for (int i = 0; i < segments; ++i) {
                float a0 = TWO_PI * i / segments;
                float a1 = TWO_PI * (i + 1) / segments;
                glm::vec2 p00 = c + glm::vec2(cos(a0), sin(a0)) * rad0;
                glm::vec2 p01 = c + glm::vec2(cos(a1), sin(a1)) * rad0;
                glm::vec2 p10 = c + glm::vec2(cos(a0), sin(a0)) * rad1;
                glm::vec2 p11 = c + glm::vec2(cos(a1), sin(a1)) * rad1;
                mesh.addVertex(glm::vec3(p00, 0)); mesh.addColor(col0);
                mesh.addVertex(glm::vec3(p10, 0)); mesh.addColor(col1);
                mesh.addVertex(glm::vec3(p11, 0)); mesh.addColor(col1);
                mesh.addVertex(glm::vec3(p00, 0)); mesh.addColor(col0);
                mesh.addVertex(glm::vec3(p11, 0)); mesh.addColor(col1);
                mesh.addVertex(glm::vec3(p01, 0)); mesh.addColor(col0);
            }
        }
        return mesh;
    }

    // Linear: build a strip of quads along the gradient axis, clipped visually
    // by the [0,width]x[0,height] area via a covering rotated band.
    const glm::vec2 cc(width * 0.5f, height * 0.5f);
    const float     rad = glm::radians(g.angle);
    const glm::vec2 dir(cos(rad), sin(rad));        // gradient axis
    const glm::vec2 perp(-dir.y, dir.x);
    const float     half = 0.5f * std::sqrt(width * width + height * height);

    const int slices = std::max(2, g.numSteps > 0 ? g.numSteps : 64);
    for (int i = 0; i < slices; ++i) {
        float t0 = (float)i / slices;
        float t1 = (float)(i + 1) / slices;
        ofFloatColor col0 = toOf(g.sample(t0));
        ofFloatColor col1 = toOf(g.sample(t1));
        glm::vec2 a0 = cc + dir * ((t0 * 2.0f - 1.0f) * half) - perp * half;
        glm::vec2 b0 = cc + dir * ((t0 * 2.0f - 1.0f) * half) + perp * half;
        glm::vec2 a1 = cc + dir * ((t1 * 2.0f - 1.0f) * half) - perp * half;
        glm::vec2 b1 = cc + dir * ((t1 * 2.0f - 1.0f) * half) + perp * half;
        mesh.addVertex(glm::vec3(a0, 0)); mesh.addColor(col0);
        mesh.addVertex(glm::vec3(a1, 0)); mesh.addColor(col1);
        mesh.addVertex(glm::vec3(b1, 0)); mesh.addColor(col1);
        mesh.addVertex(glm::vec3(a0, 0)); mesh.addColor(col0);
        mesh.addVertex(glm::vec3(b1, 0)); mesh.addColor(col1);
        mesh.addVertex(glm::vec3(b0, 0)); mesh.addColor(col0);
    }
    return mesh;
}

void drawGradientFill(const gradient_component& g, float width, float height) {
    ofPushStyle();
    ofFill();

    if (g.type == GradientType::Radial) {
        const float maxDim = std::max(width, height);
        const float cx = g.center.x * width;
        const float cy = g.center.y * height;
        const float outer = g.outerRadius * maxDim;
        const float inner = g.innerRadius * maxDim;
        const int   steps = g.numSteps > 0 ? g.numSteps : 256;
        for (int i = steps - 1; i >= 0; --i) {
            float t = (float)i / (steps - 1);
            float radius = ofLerp(inner, outer, t);
            ofSetColor(toOf(g.sample(t)));
            ofDrawCircle(cx, cy, radius);
        }
        ofPopStyle();
        return;
    }

    // Linear at arbitrary angle: rotate around the centre and draw bands.
    const float diag = std::sqrt(width * width + height * height);
    const int   steps = g.numSteps > 0 ? g.numSteps : 256;
    const float band  = diag / steps;

    ofPushMatrix();
    ofTranslate(width * 0.5f, height * 0.5f);
    ofRotateDeg(g.angle);
    ofTranslate(-diag * 0.5f, -diag * 0.5f);
    for (int i = 0; i < steps; ++i) {
        float t = (float)i / (steps - 1);
        ofSetColor(toOf(g.sample(t)));
        ofDrawRectangle(i * band, 0, band + 1.0f, diag);
    }
    ofPopMatrix();
    ofPopStyle();
}

} // namespace ecs
