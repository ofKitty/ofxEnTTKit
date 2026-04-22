#include "led_components.h"
#include <fstream>

namespace ecs {

// ============================================================================
// uv_component
// ============================================================================

bool uv_component::loadFromJSON(const std::string& jsonPath) {
    try {
        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            ofLogError("uv_component") << "Failed to open file: " << jsonPath;
            return false;
        }
        nlohmann::json j;
        file >> j;
        return loadFromJSON(j);
    }
    catch (const std::exception& e) {
        ofLogError("uv_component") << "Failed to load JSON: " << e.what();
        return false;
    }
}

bool uv_component::loadFromJSON(const nlohmann::json& j) {
    try {
        name = j.value("name", "Untitled");
        width = j.value("width", 100.0f);
        height = j.value("height", 100.0f);
        
        pixels.clear();
        
        if (j.contains("points")) {
            for (const auto& p : j["points"]) {
                LEDPixel pixel;
                pixel.index = p.value("index", 0);
                pixel.uv.x = p.value("u", 0.0f);
                pixel.uv.y = p.value("v", 0.0f);
                pixels.push_back(pixel);
            }
        }
        
        ofLogNotice("uv_component") << "Loaded: " << name 
                                    << " (" << pixels.size() << " LEDs, "
                                    << width << "x" << height << ")";
        return true;
    }
    catch (const std::exception& e) {
        ofLogError("uv_component") << "Failed to parse JSON: " << e.what();
        return false;
    }
}

bool uv_component::saveToJSON(const std::string& jsonPath) const {
    try {
        nlohmann::json j = toJSON();
        std::ofstream file(jsonPath);
        if (!file.is_open()) {
            ofLogError("uv_component") << "Failed to open file for writing: " << jsonPath;
            return false;
        }
        file << j.dump(2);
        return file.good();
    }
    catch (const std::exception& e) {
        ofLogError("uv_component") << "Failed to save JSON: " << e.what();
        return false;
    }
}

nlohmann::json uv_component::toJSON() const {
    nlohmann::json j;
    j["name"] = name;
    j["width"] = width;
    j["height"] = height;
    j["points"] = nlohmann::json::array();
    
    for (const auto& pixel : pixels) {
        nlohmann::json p;
        p["index"] = pixel.index;
        p["u"] = pixel.uv.x;
        p["v"] = pixel.uv.y;
        j["points"].push_back(p);
    }
    
    j["LedCount"] = static_cast<int>(pixels.size());
    return j;
}

} // namespace ecs
