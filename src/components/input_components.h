#pragma once

#include "ofMain.h"
#include <entt.hpp>
#include <array>
#include <string>

namespace ecs {

// ============================================================================
// INPUT COMPONENTS
// ============================================================================
// Keyboard polling uses openFrameworks directly (always available).
// Gamepad/joystick state is a plain component — backends (e.g. ofxJoystick)
// attach poll hooks via InputSystem::setJoystickPollHook().
// ============================================================================

struct keyboard_input_component {
	static constexpr int kKeyCount = 512;

	bool enabled = true;
	/// When false, poll still runs but gameplay code should ignore held keys.
	bool active = true;

	std::array<bool, kKeyCount> down {};
	std::array<bool, kKeyCount> pressed {};
	std::array<bool, kKeyCount> released {};

	void update();
	void clearEdges();

	bool isDown(int key) const;
	bool isPressed(int key) const;
	bool isReleased(int key) const;
};

/// Gamepad state filled each frame by a registered poll hook (see InputSystem).
struct joystick_input_component {
	static constexpr int kMaxButtons = 32;
	static constexpr int kMaxAxes     = 16;
	static constexpr int kDefaultJoystickId = 0;

	bool enabled = true;
	bool active  = true;
	int  joyId   = kDefaultJoystickId;

	bool connected    = false;
	std::string name;
	int buttonCount = 0;
	int axisCount   = 0;

	std::array<float, kMaxAxes>     axes {};
	std::array<bool, kMaxButtons> buttonDown {};
	std::array<bool, kMaxButtons> buttonPressed {};
	std::array<bool, kMaxButtons> buttonReleased {};
	std::array<bool, kMaxButtons> buttonPushing {};

	void setup(int id = kDefaultJoystickId);
	void clearEdges();

	float getAxis(int index) const;
	bool  isDown(int button) const;
	bool  isPressed(int button) const;
	bool  isPushing(int button) const;
	bool  isReleased(int button) const;
	bool  anyButton() const;
};

} // namespace ecs
