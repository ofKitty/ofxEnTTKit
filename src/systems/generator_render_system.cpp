#include "generator_render_system.h"
#include "../components/paint_components.h"   // gradient_component, drawGradientFill

namespace ecs {

// ============================================================================
// Registry storage
// ============================================================================

namespace {

std::vector<GeneratorRenderer>& renderers() {
    static std::vector<GeneratorRenderer> instance;
    return instance;
}

bool s_finalized = false;

} // namespace

void registerGeneratorRenderer(GeneratorRenderer renderer) {
    renderers().push_back(std::move(renderer));
}

void finalizeGenerators() {
    if (s_finalized) return;
    s_finalized = true;

    registerGenerator<dots_generator_component>("Dots", &GeneratorRenderSystem::drawDots);
    registerGenerator<stripes_generator_component>("Stripes", &GeneratorRenderSystem::drawStripes);
    registerGenerator<checkerboard_generator_component>("Checkerboard", &GeneratorRenderSystem::drawCheckerboard);
    registerGenerator<noise_generator_component>(
        "Noise",
        &GeneratorRenderSystem::drawNoise,
        [](noise_generator_component& c, float dt) { c.timeOffset += dt * c.speed; });

    // Gradient paint doubles as a full-area generator (pixels only). Custom
    // entry rather than registerGenerator<T> because gradient_component has no
    // `enabled` flag. Lives here (alongside the other built-ins) so the paint
    // data layer stays free of any systems dependency.
    {
        GeneratorRenderer r;
        r.name           = "Gradient";
        r.supportsVector = false;
        r.draw = [](entt::registry& reg) {
            auto view = reg.view<gradient_component>();
            for (auto e : view) {
                const gradient_component& g = view.get<gradient_component>(e);
                if (auto* fc = reg.try_get<fbo_component>(e)) {
                    if (!fc->fbo.isAllocated()) fc->reallocate();
                    fc->fbo.begin();
                    if (fc->clearFrame) ofClear(fc->clearColor);
                    drawGradientFill(g, (float)fc->width, (float)fc->height);
                    fc->fbo.end();
                    fc->dirty = false;
                } else {
                    const ofRectangle vp = ofGetCurrentViewport();
                    drawGradientFill(g, vp.width, vp.height);
                }
            }
        };
        registerGeneratorRenderer(std::move(r));
    }
}

const std::vector<GeneratorRenderer>& generatorRenderers() {
    finalizeGenerators();
    return renderers();
}

// ============================================================================
// System — drive the registry
// ============================================================================

void GeneratorRenderSystem::update(entt::registry& registry, float deltaTime) {
    finalizeGenerators();
    for (auto& r : renderers()) {
        if (r.update) r.update(registry, deltaTime);
    }
}

void GeneratorRenderSystem::draw(entt::registry& registry) {
    finalizeGenerators();
    for (auto& r : renderers()) {
        if (r.draw) r.draw(registry);
    }
}

// ============================================================================
// Dots
// ============================================================================

void GeneratorRenderSystem::drawDots(const dots_generator_component& c, float width, float height) {
    ofPushStyle();

    ofSetColor(c.bgColor);
    ofFill();
    ofDrawRectangle(0, 0, width, height);

    float cellW = width / c.countX;
    float cellH = height / c.countY;

    ofSetColor(c.dotColor);
    if (c.filled) {
        ofFill();
    } else {
        ofNoFill();
        ofSetLineWidth(c.strokeWeight);
    }

    for (int y = 0; y < c.countY; y++) {
        float rowOffset = (c.offset && y % 2 == 1) ? cellW * 0.5f : 0;

        for (int x = 0; x < c.countX; x++) {
            float cx = x * cellW + cellW / 2 + rowOffset;
            float cy = y * cellH + cellH / 2;

            float size = c.dotSize;
            if (c.randomSize) {
                float seed = sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
                seed = seed - floor(seed);
                size = c.randomSizeMin + seed * (c.randomSizeMax - c.randomSizeMin);
            }

            ofDrawCircle(cx, cy, size / 2);
        }
    }

    ofPopStyle();
}

// ============================================================================
// Stripes
// ============================================================================

void GeneratorRenderSystem::drawStripes(const stripes_generator_component& c, float width, float height) {
    ofPushStyle();
    ofPushMatrix();

    if (abs(c.angle) > 0.01f) {
        ofTranslate(width / 2, height / 2);
        ofRotateDeg(c.angle);
        ofTranslate(-width / 2, -height / 2);
    }

    ofFill();

    float totalSize = c.vertical ? width : height;
    float pairSize = totalSize / c.count;
    float offsetPixels = c.offset * pairSize;

    ofSetColor(c.color1);
    ofDrawRectangle(-pairSize, -pairSize, width + pairSize * 2, height + pairSize * 2);

    ofSetColor(c.color2);

    int numStripes = c.count + 2;
    for (int i = -1; i < numStripes; i++) {
        float pos = i * pairSize + offsetPixels;

        if (c.vertical) {
            ofDrawRectangle(pos, -pairSize, c.stripeWidth, height + pairSize * 2);
        } else {
            ofDrawRectangle(-pairSize, pos, width + pairSize * 2, c.stripeWidth);
        }
    }

    ofPopMatrix();
    ofPopStyle();
}

// ============================================================================
// Checkerboard
// ============================================================================

void GeneratorRenderSystem::drawCheckerboard(const checkerboard_generator_component& c, float width, float height) {
    ofPushStyle();
    ofFill();

    float cellW = width / c.countX;
    float cellH = height / c.countY;

    for (int y = 0; y < c.countY; y++) {
        for (int x = 0; x < c.countX; x++) {
            bool isEven = (x + y) % 2 == 0;
            ofSetColor(isEven ? c.color1 : c.color2);
            ofDrawRectangle(x * cellW, y * cellH, cellW + 1, cellH + 1);
        }
    }

    ofPopStyle();
}

// ============================================================================
// Noise
// ============================================================================

void GeneratorRenderSystem::drawNoise(const noise_generator_component& c, float width, float height) {
    ofPushStyle();

    int step = 4;
    for (int y = 0; y < height; y += step) {
        for (int x = 0; x < width; x += step) {
            float n = ofNoise(x * c.scale, y * c.scale, c.timeOffset);
            ofColor col = c.colorLow.getLerped(c.colorHigh, n);
            ofSetColor(col);
            ofDrawRectangle(x, y, step, step);
        }
    }

    ofPopStyle();
}

} // namespace ecs
