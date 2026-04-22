#include "swatch_components.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace ecs {

// ============================================================================
// SwatchColor Implementation
// ============================================================================

SwatchColor SwatchColor::fromCMYK(float c, float m, float y, float k, const std::string& name) {
    // CMYK to RGB conversion (CMYK values 0-100)
    c = ofClamp(c, 0.0f, 100.0f) / 100.0f;
    m = ofClamp(m, 0.0f, 100.0f) / 100.0f;
    y = ofClamp(y, 0.0f, 100.0f) / 100.0f;
    k = ofClamp(k, 0.0f, 100.0f) / 100.0f;
    
    float r = 255.0f * (1.0f - c) * (1.0f - k);
    float g = 255.0f * (1.0f - m) * (1.0f - k);
    float b = 255.0f * (1.0f - y) * (1.0f - k);
    
    SwatchColor result;
    result.color.set((unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
    result.name = name;
    result.type = SwatchColorType::CMYK;
    return result;
}

glm::vec4 SwatchColor::getCMYK() const {
    // RGB to CMYK conversion (returns 0-100 range)
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    
    float k = 1.0f - std::max({r, g, b});
    
    if (k >= 1.0f) {
        return glm::vec4(0.0f, 0.0f, 0.0f, 100.0f);
    }
    
    float c = (1.0f - r - k) / (1.0f - k);
    float m = (1.0f - g - k) / (1.0f - k);
    float y = (1.0f - b - k) / (1.0f - k);
    
    return glm::vec4(c * 100.0f, m * 100.0f, y * 100.0f, k * 100.0f);
}

glm::vec3 SwatchColor::getHSB() const {
    return glm::vec3(color.getHue(), color.getSaturation(), color.getBrightness());
}

SwatchColor SwatchColor::fromHSB(float h, float s, float b, const std::string& name) {
    SwatchColor result;
    result.color.setHsb(h, s, b);
    result.name = name;
    result.type = SwatchColorType::RGB;
    return result;
}

std::string SwatchColor::getDisplayName() const {
    if (!name.empty()) return name;
    
    // Auto-generate name from color values
    std::stringstream ss;
    if (type == SwatchColorType::CMYK) {
        glm::vec4 cmyk = getCMYK();
        ss << "C" << (int)cmyk.r << " M" << (int)cmyk.g << " Y" << (int)cmyk.b << " K" << (int)cmyk.a;
    } else {
        ss << "R" << (int)color.r << " G" << (int)color.g << " B" << (int)color.b;
    }
    return ss.str();
}

// ============================================================================
// Color Harmony Generation
// ============================================================================

SwatchColor SwatchColor::shiftHue(float degrees) const {
    float h = color.getHue();
    float s = color.getSaturation();
    float b = color.getBrightness();
    
    // Shift hue (0-255 range in OF, so convert degrees to that)
    float hueShift = (degrees / 360.0f) * 255.0f;
    h = fmod(h + hueShift + 255.0f, 255.0f);
    
    SwatchColor result;
    result.color.setHsb(h, s, b);
    result.color.a = color.a;
    result.type = type;
    return result;
}

SwatchColor SwatchColor::adjustSaturation(float factor) const {
    float h = color.getHue();
    float s = color.getSaturation();
    float b = color.getBrightness();
    
    s = ofClamp(s * factor, 0.0f, 255.0f);
    
    SwatchColor result;
    result.color.setHsb(h, s, b);
    result.color.a = color.a;
    result.type = type;
    return result;
}

SwatchColor SwatchColor::adjustBrightness(float factor) const {
    float h = color.getHue();
    float s = color.getSaturation();
    float b = color.getBrightness();
    
    b = ofClamp(b * factor, 0.0f, 255.0f);
    
    SwatchColor result;
    result.color.setHsb(h, s, b);
    result.color.a = color.a;
    result.type = type;
    return result;
}

SwatchColor SwatchColor::getComplementary() const {
    SwatchColor result = shiftHue(180.0f);
    result.name = name.empty() ? "Complementary" : name + " (Complementary)";
    return result;
}

std::vector<SwatchColor> SwatchColor::getTriadic() const {
    std::vector<SwatchColor> results;
    
    SwatchColor c1 = shiftHue(120.0f);
    c1.name = name.empty() ? "Triadic 1" : name + " (Triadic 1)";
    results.push_back(c1);
    
    SwatchColor c2 = shiftHue(240.0f);
    c2.name = name.empty() ? "Triadic 2" : name + " (Triadic 2)";
    results.push_back(c2);
    
    return results;
}

std::vector<SwatchColor> SwatchColor::getTetradic() const {
    std::vector<SwatchColor> results;
    
    SwatchColor c1 = shiftHue(90.0f);
    c1.name = name.empty() ? "Tetradic 1" : name + " (Tetradic 1)";
    results.push_back(c1);
    
    SwatchColor c2 = shiftHue(180.0f);
    c2.name = name.empty() ? "Tetradic 2" : name + " (Tetradic 2)";
    results.push_back(c2);
    
    SwatchColor c3 = shiftHue(270.0f);
    c3.name = name.empty() ? "Tetradic 3" : name + " (Tetradic 3)";
    results.push_back(c3);
    
    return results;
}

std::vector<SwatchColor> SwatchColor::getAnalogous(float angle) const {
    std::vector<SwatchColor> results;
    
    SwatchColor c1 = shiftHue(-angle);
    c1.name = name.empty() ? "Analogous 1" : name + " (Analogous 1)";
    results.push_back(c1);
    
    SwatchColor c2 = shiftHue(angle);
    c2.name = name.empty() ? "Analogous 2" : name + " (Analogous 2)";
    results.push_back(c2);
    
    return results;
}

std::vector<SwatchColor> SwatchColor::getSplitComplementary() const {
    std::vector<SwatchColor> results;
    
    // Complement is at 180°, split is at 150° and 210°
    SwatchColor c1 = shiftHue(150.0f);
    c1.name = name.empty() ? "Split Comp 1" : name + " (Split Comp 1)";
    results.push_back(c1);
    
    SwatchColor c2 = shiftHue(210.0f);
    c2.name = name.empty() ? "Split Comp 2" : name + " (Split Comp 2)";
    results.push_back(c2);
    
    return results;
}

std::vector<SwatchColor> SwatchColor::getMonochromatic(int count) const {
    std::vector<SwatchColor> results;
    
    float h = color.getHue();
    float s = color.getSaturation();
    float b = color.getBrightness();
    
    for (int i = 1; i <= count; i++) {
        // Vary brightness and saturation
        float factor = 0.3f + (0.7f * i / count);
        
        SwatchColor c;
        c.color.setHsb(h, s * factor, ofClamp(b * (0.5f + factor * 0.5f), 0, 255));
        c.color.a = color.a;
        c.type = type;
        c.name = name.empty() ? ("Mono " + std::to_string(i)) : (name + " (Mono " + std::to_string(i) + ")");
        results.push_back(c);
    }
    
    return results;
}

std::vector<SwatchColor> SwatchColor::generateHarmony(ColorHarmony harmony) const {
    std::vector<SwatchColor> results;
    
    switch (harmony) {
        case ColorHarmony::Complementary:
            results.push_back(getComplementary());
            break;
        case ColorHarmony::Triadic:
            results = getTriadic();
            break;
        case ColorHarmony::Tetradic:
            results = getTetradic();
            break;
        case ColorHarmony::Analogous:
            results = getAnalogous();
            break;
        case ColorHarmony::SplitComplementary:
            results = getSplitComplementary();
            break;
        case ColorHarmony::Monochromatic:
            results = getMonochromatic();
            break;
    }
    
    return results;
}

} // namespace ecs
