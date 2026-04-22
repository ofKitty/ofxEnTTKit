#pragma once

#include <entt.hpp>
#include <cstdint>
#include <sstream>
#include <string>

namespace ecs {

// Convert entt::entity to string (e.g. for JSON keys, ImGui IDs)
inline std::string getStringID(entt::entity e) {
    std::ostringstream oss;
    oss << static_cast<std::uint32_t>(e);
    return oss.str();
}

} // namespace ecs
