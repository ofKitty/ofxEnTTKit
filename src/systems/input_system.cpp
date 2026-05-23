#include "input_system.h"

namespace ecs {

namespace {
JoystickPollFn s_joystickPoll;
}

void InputSystem::setJoystickPollHook(JoystickPollFn fn) {
	s_joystickPoll = std::move(fn);
}

void InputSystem::clearJoystickPollHook() {
	s_joystickPoll = nullptr;
}

bool InputSystem::hasJoystickPollHook() {
	return static_cast<bool>(s_joystickPoll);
}

void InputSystem::updateKeyboard(entt::registry& registry) {
	auto view = registry.view<keyboard_input_component>();
	for (auto entity : view) {
		registry.get<keyboard_input_component>(entity).update();
	}
}

void InputSystem::updateJoystick(entt::registry& registry) {
	if (!s_joystickPoll) return;

	auto view = registry.view<joystick_input_component>();
	for (auto entity : view) {
		auto& joy = registry.get<joystick_input_component>(entity);
		if (joy.enabled) {
			s_joystickPoll(joy);
		}
	}
}

void InputSystem::update(entt::registry& registry, float /*deltaTime*/) {
	updateKeyboard(registry);
	updateJoystick(registry);
}

} // namespace ecs
