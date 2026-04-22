#include "destroy_system.h"
#include "../components/base_components.h"

namespace ecs {

void DestroySystem::setup(entt::registry& registry) {
    registry.on_destroy<parent_component>().connect<&onParentDestroy>();
    registry.on_destroy<camera_component>().connect<&onCameraDestroy>();
}

} // namespace ecs
