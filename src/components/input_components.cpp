#include "input_components.h"

namespace ecs {

void keyboard_input_component::update() {
	if (!enabled) return;

	for (int key = 0; key < kKeyCount; ++key) {
		const bool now = ofGetKeyPressed(key);
		pressed[key]  = now && !down[key];
		released[key] = !now && down[key];
		down[key]     = now;
	}
}

void keyboard_input_component::clearEdges() {
	pressed.fill(false);
	released.fill(false);
}

bool keyboard_input_component::isDown(int key) const {
	if (key < 0 || key >= kKeyCount) return false;
	return down[key];
}

bool keyboard_input_component::isPressed(int key) const {
	if (key < 0 || key >= kKeyCount) return false;
	return pressed[key];
}

bool keyboard_input_component::isReleased(int key) const {
	if (key < 0 || key >= kKeyCount) return false;
	return released[key];
}

void joystick_input_component::setup(int id) {
	joyId = id;
}

void joystick_input_component::clearEdges() {
	buttonPressed.fill(false);
	buttonReleased.fill(false);
}

float joystick_input_component::getAxis(int index) const {
	if (index < 0 || index >= kMaxAxes) return 0.f;
	return axes[index];
}

bool joystick_input_component::isDown(int button) const {
	if (button < 0 || button >= kMaxButtons) return false;
	return buttonDown[button];
}

bool joystick_input_component::isPressed(int button) const {
	if (button < 0 || button >= kMaxButtons) return false;
	return buttonPressed[button];
}

bool joystick_input_component::isPushing(int button) const {
	if (button < 0 || button >= kMaxButtons) return false;
	return buttonPushing[button];
}

bool joystick_input_component::isReleased(int button) const {
	if (button < 0 || button >= kMaxButtons) return false;
	return buttonReleased[button];
}

bool joystick_input_component::anyButton() const {
	for (int i = 0; i < buttonCount; ++i) {
		if (buttonPressed[i]) return true;
	}
	return false;
}

} // namespace ecs
