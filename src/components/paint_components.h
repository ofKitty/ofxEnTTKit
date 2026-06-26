#pragma once

#include "ofMain.h"
#include <entt.hpp>
#include <algorithm>
#include <string>
#include <vector>

// ============================================================================
// PAINT COMPONENTS  (canonical colour / gradient model)
// ============================================================================
// SVG-style paint model. A "paint" is a small ECS data component describing
// how to fill/stroke pixels:
//
//   solid_color_component  — a single colour.
//   gradient_component     — a multi-stop gradient (pure data + sample()).
//
// Colours are stored as a normalized linear RGBA glm::vec4 (each channel 0..1).
// glm ships with openFrameworks core (header-only, zero addon dependencies), so
// this data layer stays "headless": no ofxSwatches, no ImGui. Convert to/from
// ofColor only at framework boundaries (ofMesh, ofSetColor, ofPath). The paint
// *inspectors* (ImGradientHDR-based editing) live in ofxEnTTInspector.
//
// Shapes do NOT embed paint data. They reference a paint entity through
// fill_component / stroke_component (composition over inheritance). Several
// shapes can share one paint entity; editing the paint updates every user.
//
// gradient_mesh_cache_component holds derived GPU geometry. It is never
// serialized — it is rebuilt from the gradient_component on demand.
//
// Component types stay in namespace ecs (same registry across the stack).
// ============================================================================

namespace ecs {

// ----------------------------------------------------------------------------
// Solid paint
// ----------------------------------------------------------------------------

struct solid_color_component {
    glm::vec4 color { 1.0f, 1.0f, 1.0f, 1.0f };   // normalized RGBA 0..1

    solid_color_component() = default;
    explicit solid_color_component(const glm::vec4& c) : color(c) {}
};

// ----------------------------------------------------------------------------
// Gradient paint (pure data)
// ----------------------------------------------------------------------------

enum class GradientType          { Linear = 0, Radial };
enum class GradientInterpolation { RGB = 0, HSV };
enum class GradientSpread        { Pad = 0, Repeat, Mirror };

struct GradientStop {
    float     position = 0.0f;
    glm::vec4 color { 0.0f, 0.0f, 0.0f, 1.0f };   // normalized RGBA, carries alpha

    GradientStop() = default;
    GradientStop(float pos, const glm::vec4& col) : position(pos), color(col) {}
};

struct gradient_component {
    std::string               name = "Gradient";
    std::vector<GradientStop>  stops;

    GradientType          type   = GradientType::Linear;
    GradientInterpolation interp = GradientInterpolation::RGB;
    GradientSpread        spread = GradientSpread::Pad;

    float     angle = 90.0f;            // linear axis direction, degrees
    glm::vec2 center{ 0.5f, 0.5f };     // radial centre, normalized (0..1)
    float     innerRadius = 0.0f;       // radial, normalized to max(w,h)
    float     outerRadius = 1.0f;       // radial, normalized to max(w,h)
    int       numSteps = 0;             // 0 = full resolution

    gradient_component() {
        stops.push_back(GradientStop(0.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));
        stops.push_back(GradientStop(1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    }

    explicit gradient_component(const std::string& n) : name(n) {
        stops.push_back(GradientStop(0.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));
        stops.push_back(GradientStop(1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    }

    /// Colour at normalized position @p t. Honours spread + interpolation.
    /// Assumes stops are sorted (see sortStops()).
    glm::vec4 sample(float t) const;

    void sortStops();
    void addStop(float position, const glm::vec4& color);
    void removeStop(int index);
    void reverse();

    int count() const { return (int)stops.size(); }

    static gradient_component fromColors(const glm::vec4& start, const glm::vec4& end,
                                         const std::string& name = "");
};

// ----------------------------------------------------------------------------
// Derived GPU cache (NOT serialized)
// ----------------------------------------------------------------------------

struct gradient_mesh_cache_component {
    ofMesh mesh;
    bool   dirty = true;
};

// ----------------------------------------------------------------------------
// Paint references (a shape points at a paint entity)
// ----------------------------------------------------------------------------

struct fill_component {
    entt::entity paint = entt::null;
};

struct stroke_component {
    entt::entity paint = entt::null;
    float        width = 1.0f;
};

// ----------------------------------------------------------------------------
// Resolve helpers
// ----------------------------------------------------------------------------

/// Resolve @p paint to a representative solid colour (normalized RGBA):
///   solid_color_component -> its colour
///   gradient_component    -> sample(0.5)
/// Returns false when @p paint holds no paint component.
bool resolvePaintColor(const entt::registry& reg, entt::entity paint, glm::vec4& out);

/// Build a triangle mesh that fills [0,0]..[width,height] for @p g (linear axis
/// or radial). Used to populate gradient_mesh_cache_component.
ofMesh buildGradientMesh(const gradient_component& g, float width, float height);

/// Immediate-mode fill of a width x height area at the origin (used by the
/// generator render path). Honours type / angle / centre / radii / numSteps.
void drawGradientFill(const gradient_component& g, float width, float height);

} // namespace ecs
