#pragma once

#include <entt/entt.hpp>
#include "ofMain.h"
#include <algorithm>
#include <string>
#include <vector>

// ============================================================================
// COLOUR COMPONENTS - Pure data structures for colour in ECS.
// Implementation (CMYK conversion, harmony generation, JSON I/O, UI panels)
// lives in ofxSwatches. These components hold only the data; systems and
// ofxSwatches operate on them independently.
// ============================================================================

namespace ecs {

enum class SwatchColorType {
    RGB  = 0,
    CMYK = 1
};

enum class ColorHarmony {
    Complementary,
    Triadic,
    Tetradic,
    Analogous,
    SplitComplementary,
    Monochromatic
};

/// CMYK ink ratios (0–100) used when generating rich-black separations.
struct RichBlackPreset {
    float c = 60.0f;
    float m = 40.0f;
    float y = 40.0f;
    float k = 100.0f;
};

/// A named colour entry. Stores the display RGB value and optional CMYK data.
/// Harmony generation and CMYK conversion are not stored here; use ofxSwatches.
struct SwatchColor {
    std::string   name;
    ofColor       color        = ofColor::white;
    SwatchColorType type       = SwatchColorType::RGB;
    bool          isSpotColor  = false;
    std::string   spotInkName;
    glm::vec4     cmyk100      = glm::vec4(0.0f); ///< C,M,Y,K 0–100 when type == CMYK

    SwatchColor() = default;

    SwatchColor(const ofColor& col, const std::string& n = "")
        : name(n), color(col), type(SwatchColorType::RGB) {}

    SwatchColor(unsigned char r, unsigned char g, unsigned char b,
                unsigned char a = 255, const std::string& n = "")
        : name(n), color(r, g, b, a), type(SwatchColorType::RGB) {}

    operator ofColor() const { return color; }
};

struct GradientStop {
    float       position = 0.0f;
    SwatchColor color;

    GradientStop() = default;
    GradientStop(float pos, const SwatchColor& col) : position(pos), color(col) {}
    GradientStop(float pos, const ofColor& col)     : position(pos), color(col) {}
};

/// One ECS entity per palette; swatches live in `colors[]`, not as separate entities.
struct swatch_library_component {
    std::string            libraryName = "Untitled Library";
    std::vector<SwatchColor> colors;
    RichBlackPreset        richBlack;

    swatch_library_component() = default;
    explicit swatch_library_component(const std::string& name) : libraryName(name) {}

    int  count() const { return (int)colors.size(); }
    bool empty() const { return colors.empty(); }
};

struct color_gradient_component {
    std::string             name = "Untitled Gradient";
    std::vector<GradientStop> stops;

    color_gradient_component() {
        stops.push_back(GradientStop(0.0f, ofColor::black));
        stops.push_back(GradientStop(1.0f, ofColor::white));
    }

    explicit color_gradient_component(const std::string& n) : name(n) {
        stops.push_back(GradientStop(0.0f, ofColor::black));
        stops.push_back(GradientStop(1.0f, ofColor::white));
    }

    ofColor getColorAt(float t) const {
        if (stops.empty()) return ofColor::white;
        if (stops.size() == 1) return stops[0].color.color;

        t = ofClamp(t, 0.0f, 1.0f);
        for (size_t i = 0; i < stops.size() - 1; i++) {
            if (t >= stops[i].position && t <= stops[i + 1].position) {
                float range = stops[i + 1].position - stops[i].position;
                if (range < 0.0001f) return stops[i].color.color;
                float localT = (t - stops[i].position) / range;
                return stops[i].color.color.getLerped(stops[i + 1].color.color, localT);
            }
        }
        return stops.back().color.color;
    }

    void addStop(float position, const SwatchColor& color) {
        stops.push_back(GradientStop(position, color));
        sortStops();
    }

    void addStop(float position, const ofColor& color) {
        stops.push_back(GradientStop(position, color));
        sortStops();
    }

    void removeStop(int index) {
        if (stops.size() > 2 && index >= 0 && index < (int)stops.size()) {
            stops.erase(stops.begin() + index);
        }
    }

    void sortStops() {
        std::sort(stops.begin(), stops.end(),
            [](const GradientStop& a, const GradientStop& b) {
                return a.position < b.position;
            });
    }

    void reverse() {
        for (auto& stop : stops) {
            stop.position = 1.0f - stop.position;
        }
        std::reverse(stops.begin(), stops.end());
    }

    int count() const { return (int)stops.size(); }

    static color_gradient_component fromColors(const ofColor& start, const ofColor& end,
                                               const std::string& name = "") {
        color_gradient_component g(name);
        g.stops.clear();
        g.stops.push_back(GradientStop(0.0f, start));
        g.stops.push_back(GradientStop(1.0f, end));
        return g;
    }

    static color_gradient_component fromLibrary(const swatch_library_component& library,
                                                const std::string& name = "") {
        color_gradient_component g(name.empty() ? library.libraryName + " Gradient" : name);
        g.stops.clear();
        int n = library.count();
        if (n == 0) return g;
        if (n == 1) {
            g.stops.push_back(GradientStop(0.0f, library.colors[0]));
            g.stops.push_back(GradientStop(1.0f, library.colors[0]));
        } else {
            for (int i = 0; i < n; i++) {
                float pos = (float)i / (n - 1);
                g.stops.push_back(GradientStop(pos, library.colors[i]));
            }
        }
        return g;
    }
};

/// Bind a layer/path to a specific palette entry (e.g. for plotter colour mapping).
///
/// Pattern:
///   1. Create a swatch_library_component entity (palette).
///   2. Attach this component to the layer/path group entity.
///   3. Set `library` to the palette entity; resolve stroke/fill via colorIndex or colorName.
struct swatch_palette_ref_component {
    entt::entity library    = entt::null;
    int          colorIndex = -1;
    std::string  colorName;
};

} // namespace ecs
