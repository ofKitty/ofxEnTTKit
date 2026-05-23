#pragma once

#include "base_system.h"
#include "components/input_components.h"
#include <functional>

namespace ecs {

class InputSystem : public ISystem {
public:
	using JoystickPollFn = std::function<void(joystick_input_component&)>;

	const char* getName() const override { return "InputSystem"; }

	void update(entt::registry& registry, float deltaTime) override;

	static void setJoystickPollHook(JoystickPollFn fn);
	static void clearJoystickPollHook();
	static bool hasJoystickPollHook();

	static void updateKeyboard(entt::registry& registry);
	static void updateJoystick(entt::registry& registry);
};

} // namespace ecs
