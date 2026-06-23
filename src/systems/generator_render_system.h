#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/base_components.h"      // fbo_component
#include "../components/generator_components.h"

#include <functional>
#include <string>
#include <vector>

namespace ecs {

// ============================================================================
// Generator render registry
// ============================================================================
// Generator components are pure data. Each generator *type* registers how it is
// rendered. Pixels are the universal output (a fill drawn directly, or into an
// fbo_component when one is present on the entity). A generator may additionally
// emit vectors/paths for plotting — that capability is advertised by the single
// supportsVector flag (pixels can always be produced, vectors only sometimes).
//
// Built-in kit generators self-register via finalizeGenerators(). Addons extend
// the same registry with registerGenerator<T>(...).

struct GeneratorRenderer {
    std::string name;
    bool supportsVector = false;  // pixels always available; true if it can also emit vector paths

    // Per-frame time advance (optional) and rasterization (required).
    std::function<void(entt::registry&, float deltaTime)> update;
    std::function<void(entt::registry&)> draw;
};

/// Register a pre-built renderer entry (used by registerGenerator<T> and addons).
void registerGeneratorRenderer(GeneratorRenderer renderer);

/// Register a generator component type T.
///   drawFn:         rasterizes one component into a width x height area.
///   updateFn:       optional per-frame mutation (e.g. advancing time).
///   supportsVector: true if this generator can also emit vector paths.
template<typename T>
void registerGenerator(const char* name,
                       std::function<void(const T&, float, float)> drawFn,
                       std::function<void(T&, float)> updateFn = {},
                       bool supportsVector = false)
{
    GeneratorRenderer r;
    r.name = name;
    r.supportsVector = supportsVector;

    r.draw = [drawFn](entt::registry& reg) {
        auto view = reg.view<T>();
        for (auto entity : view) {
            const T& c = view.template get<T>(entity);
            if (!c.enabled) continue;

            if (auto* fc = reg.template try_get<fbo_component>(entity)) {
                if (!fc->fbo.isAllocated()) fc->reallocate();
                fc->fbo.begin();
                if (fc->clearFrame) ofClear(fc->clearColor);
                drawFn(c, (float)fc->width, (float)fc->height);
                fc->fbo.end();
                fc->dirty = false;
            } else {
                const ofRectangle vp = ofGetCurrentViewport();
                drawFn(c, vp.width, vp.height);
            }
        }
    };

    if (updateFn) {
        r.update = [updateFn](entt::registry& reg, float dt) {
            auto view = reg.view<T>();
            for (auto entity : view) {
                updateFn(view.template get<T>(entity), dt);
            }
        };
    }

    registerGeneratorRenderer(std::move(r));
}

/// Register the built-in kit generators once (idempotent).
void finalizeGenerators();

/// All registered generators (calls finalizeGenerators() first).
const std::vector<GeneratorRenderer>& generatorRenderers();

// ============================================================================
// Generator Render System
// ============================================================================
// Drives the registry: advances time in update(), rasterizes in draw().

class GeneratorRenderSystem : public ISystem {
public:
    const char* getName() const override { return "GeneratorRenderSystem"; }

    void update(entt::registry& registry, float deltaTime) override;
    void draw(entt::registry& registry) override;

    // Built-in pixel draw helpers (draw at 0,0 within a width x height area).
    static void drawDots(const dots_generator_component& c, float width, float height);
    static void drawStripes(const stripes_generator_component& c, float width, float height);
    static void drawCheckerboard(const checkerboard_generator_component& c, float width, float height);
    static void drawNoise(const noise_generator_component& c, float width, float height);
};

} // namespace ecs
