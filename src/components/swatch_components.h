#pragma once

#include "ofMain.h"
#include <string>
#include <vector>
#include <algorithm>

// ============================================================================
// SWATCH COMPONENTS - Color libraries and gradients for ECS
// ============================================================================

namespace ecs {

// ============================================================================
// Color Type Enum
// ============================================================================

enum class SwatchColorType {
    RGB = 0,
    CMYK = 1
};

// ============================================================================
// Color Harmony Types
// ============================================================================

enum class ColorHarmony {
    Complementary,      // Opposite on color wheel (180°)
    Triadic,            // 3 colors, 120° apart
    Tetradic,           // 4 colors, 90° apart (square)
    Analogous,          // Adjacent colors (±30°)
    SplitComplementary, // Complement + 2 adjacent to complement
    Monochromatic       // Same hue, different saturation/brightness
};

// ============================================================================
// SwatchColor - Named color with CMYK support
// ============================================================================

struct SwatchColor {
    std::string name;
    ofColor color;
    SwatchColorType type = SwatchColorType::RGB;
    bool isSpotColor = false;
    
    // Constructors
    SwatchColor() : color(ofColor::white) {}
    
    SwatchColor(const ofColor& col, const std::string& n = "")
        : name(n), color(col), type(SwatchColorType::RGB) {}
    
    SwatchColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255, const std::string& n = "")
        : name(n), color(r, g, b, a), type(SwatchColorType::RGB) {}
    
    // Create from CMYK values (0-100 range)
    static SwatchColor fromCMYK(float c, float m, float y, float k, const std::string& name = "");
    
    // Get CMYK values (0-100 range)
    glm::vec4 getCMYK() const;
    
    // Get HSB values
    glm::vec3 getHSB() const;
    
    // Create from HSB
    static SwatchColor fromHSB(float h, float s, float b, const std::string& name = "");
    
    // Auto-generate name from color values if empty
    std::string getDisplayName() const;
    
    // Conversion operator
    operator ofColor() const { return color; }
    
    // ========================================================================
    // Color Harmony Generation
    // ========================================================================
    
    // Generate harmony colors based on this color
    std::vector<SwatchColor> generateHarmony(ColorHarmony harmony) const;
    
    // Individual harmony generators
    SwatchColor getComplementary() const;
    std::vector<SwatchColor> getTriadic() const;
    std::vector<SwatchColor> getTetradic() const;
    std::vector<SwatchColor> getAnalogous(float angle = 30.0f) const;
    std::vector<SwatchColor> getSplitComplementary() const;
    std::vector<SwatchColor> getMonochromatic(int count = 5) const;
    
    // Utility: shift hue by degrees
    SwatchColor shiftHue(float degrees) const;
    
    // Utility: adjust saturation/brightness
    SwatchColor adjustSaturation(float factor) const;
    SwatchColor adjustBrightness(float factor) const;
};

// ============================================================================
// GradientStop - A color stop in a gradient
// ============================================================================

struct GradientStop {
    float position = 0.0f;  // 0.0 to 1.0
    SwatchColor color;
    
    GradientStop() = default;
    GradientStop(float pos, const SwatchColor& col) : position(pos), color(col) {}
    GradientStop(float pos, const ofColor& col) : position(pos), color(col) {}
};

// ============================================================================
// swatch_library_component - A collection of named colors
// ============================================================================

struct swatch_library_component {
    std::string libraryName = "Untitled Library";
    std::vector<SwatchColor> colors;
    
    swatch_library_component() = default;
    swatch_library_component(const std::string& name) : libraryName(name) {}
    
    // Add color
    void addColor(const SwatchColor& color) {
        colors.push_back(color);
    }
    
    void addColor(const ofColor& color, const std::string& name = "") {
        colors.push_back(SwatchColor(color, name));
    }
    
    // Add from CMYK (0-100 range)
    void addCMYK(float c, float m, float y, float k, const std::string& name = "") {
        colors.push_back(SwatchColor::fromCMYK(c, m, y, k, name));
    }
    
    // Remove color
    void removeColor(int index) {
        if (index >= 0 && index < (int)colors.size()) {
            colors.erase(colors.begin() + index);
        }
    }
    
    void removeColor(const std::string& name) {
        colors.erase(
            std::remove_if(colors.begin(), colors.end(),
                [&name](const SwatchColor& c) { return c.name == name; }),
            colors.end());
    }
    
    // Get color by index
    SwatchColor* getColor(int index) {
        if (index >= 0 && index < (int)colors.size()) {
            return &colors[index];
        }
        return nullptr;
    }
    
    const SwatchColor* getColor(int index) const {
        if (index >= 0 && index < (int)colors.size()) {
            return &colors[index];
        }
        return nullptr;
    }
    
    // Get color by name
    SwatchColor* getByName(const std::string& name) {
        for (auto& c : colors) {
            if (c.name == name) return &c;
        }
        return nullptr;
    }
    
    const SwatchColor* getByName(const std::string& name) const {
        for (const auto& c : colors) {
            if (c.name == name) return &c;
        }
        return nullptr;
    }
    
    // Check if color exists
    bool hasColor(const std::string& name) const {
        return getByName(name) != nullptr;
    }
    
    // Count
    int count() const { return (int)colors.size(); }
    bool empty() const { return colors.empty(); }
    
    // Clear all colors
    void clear() { colors.clear(); }
    
    // Generate harmony from a color and add to library
    void generateHarmonyFrom(int sourceIndex, ColorHarmony harmony) {
        if (sourceIndex < 0 || sourceIndex >= (int)colors.size()) return;
        auto newColors = colors[sourceIndex].generateHarmony(harmony);
        for (auto& c : newColors) {
            colors.push_back(c);
        }
    }
};

// ============================================================================
// color_gradient_component - A multi-stop color gradient for swatches
// (Named to avoid conflict with graphics2d gradient_component)
// ============================================================================

struct color_gradient_component {
    std::string name = "Untitled Gradient";
    std::vector<GradientStop> stops;
    
    color_gradient_component() {
        // Default: black to white
        stops.push_back(GradientStop(0.0f, ofColor::black));
        stops.push_back(GradientStop(1.0f, ofColor::white));
    }
    
    color_gradient_component(const std::string& n) : name(n) {
        stops.push_back(GradientStop(0.0f, ofColor::black));
        stops.push_back(GradientStop(1.0f, ofColor::white));
    }
    
    // Get interpolated color at position t (0.0 to 1.0)
    ofColor getColorAt(float t) const {
        if (stops.empty()) return ofColor::white;
        if (stops.size() == 1) return stops[0].color.color;
        
        t = ofClamp(t, 0.0f, 1.0f);
        
        // Find the two stops to interpolate between
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
    
    // Add a stop
    void addStop(float position, const SwatchColor& color) {
        stops.push_back(GradientStop(position, color));
        sortStops();
    }
    
    void addStop(float position, const ofColor& color) {
        stops.push_back(GradientStop(position, color));
        sortStops();
    }
    
    // Remove a stop (keeps at least 2)
    void removeStop(int index) {
        if (stops.size() > 2 && index >= 0 && index < (int)stops.size()) {
            stops.erase(stops.begin() + index);
        }
    }
    
    // Sort stops by position
    void sortStops() {
        std::sort(stops.begin(), stops.end(),
            [](const GradientStop& a, const GradientStop& b) {
                return a.position < b.position;
            });
    }
    
    // Reverse gradient
    void reverse() {
        for (auto& stop : stops) {
            stop.position = 1.0f - stop.position;
        }
        std::reverse(stops.begin(), stops.end());
    }
    
    // Count stops
    int count() const { return (int)stops.size(); }
    
    // Create gradient from two colors
    static color_gradient_component fromColors(const ofColor& start, const ofColor& end, const std::string& name = "") {
        color_gradient_component g(name);
        g.stops.clear();
        g.stops.push_back(GradientStop(0.0f, start));
        g.stops.push_back(GradientStop(1.0f, end));
        return g;
    }
    
    // Create gradient from swatch library colors
    static color_gradient_component fromLibrary(const swatch_library_component& library, const std::string& name = "") {
        color_gradient_component g(name.empty() ? library.libraryName + " Gradient" : name);
        g.stops.clear();
        int count = library.count();
        if (count == 0) return g;
        if (count == 1) {
            g.stops.push_back(GradientStop(0.0f, library.colors[0]));
            g.stops.push_back(GradientStop(1.0f, library.colors[0]));
        } else {
            for (int i = 0; i < count; i++) {
                float pos = (float)i / (count - 1);
                g.stops.push_back(GradientStop(pos, library.colors[i]));
            }
        }
        return g;
    }
};

} // namespace ecs
