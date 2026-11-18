#include "main.h"
#include "touch_input.h"
#include "../high_impact/src/platform.h"

#if defined(__EMSCRIPTEN__)
	#include <emscripten.h>
	#define TOUCH_EXPORT EMSCRIPTEN_KEEPALIVE
#else
	#define TOUCH_EXPORT
#endif

static bool touch_used = false;
static bool tap_release_pending = false;

static float clamp_axis(float value) {
	if (value < -1.0f) return -1.0f;
	if (value > 1.0f) return 1.0f;
	return value;
}

static void set_axis(button_t negative, button_t positive, float value) {
	if (value > INPUT_DEADZONE) {
		input_set_button_state(positive, value);
		input_set_button_state(negative, 0);
	}
	else if (value < -INPUT_DEADZONE) {
		input_set_button_state(negative, -value);
		input_set_button_state(positive, 0);
	}
	else {
		input_set_button_state(negative, 0);
		input_set_button_state(positive, 0);
	}
}

TOUCH_EXPORT void shapewars_touch_move(float x, float y) {
	touch_used = true;
	x = clamp_axis(x);
	y = clamp_axis(y);

	// The game binds movement to the left stick, so drive those buttons
	// directly. This also makes the same touch input work in the menus.
	set_axis(INPUT_GAMEPAD_L_STICK_LEFT, INPUT_GAMEPAD_L_STICK_RIGHT, x);
	set_axis(INPUT_GAMEPAD_L_STICK_UP, INPUT_GAMEPAD_L_STICK_DOWN, y);
}

TOUCH_EXPORT void shapewars_touch_aim(float x, float y) {
	touch_used = true;
	x = clamp_axis(x);
	y = clamp_axis(y);

	// The game reads the raw right stick for aiming and fires automatically
	// while the stick is deflected.
	platform_gamepad_right_stick = vec2(x, y);
}

TOUCH_EXPORT void shapewars_touch_tap(float nx, float ny, int down) {
	touch_used = true;

	// Tap coordinates arrive normalized to the whole canvas. Convert them
	// into the letterboxed 16:9 game viewport before handing them to the
	// mouse emulation, so menu hit testing stays accurate on any screen.
	float scale = render_screen_scale();
	vec2i_t screen = platform_screen_size();
	vec2i_t logical = render_size();
	float content_w = logical.x * scale;
	float content_h = logical.y * scale;
	float ox = (screen.x - content_w) * 0.5f;
	float oy = (screen.y - content_h) * 0.5f;

	input_set_mouse_pos(
		(int32_t)(ox + nx * content_w),
		(int32_t)(oy + ny * content_h)
	);
	if (down) {
		tap_release_pending = false;
		input_set_button_state(INPUT_MOUSE_LEFT, 1.0f);
		// Gamepad A is bound to menu select and restart, so a tap also works on
		// the title screen, the pause menu and the game-over screen.
		input_set_button_state(INPUT_GAMEPAD_A, 1.0f);
	}
	else {
		// A tap can be pressed and released between two frames. Keep the
		// press alive until the game has sampled it once, otherwise the
		// game-over screen (which uses input_down) never sees the tap.
		tap_release_pending = true;
	}
}

void touch_input_update(void) {
	if (tap_release_pending) {
		tap_release_pending = false;
		input_set_button_state(INPUT_MOUSE_LEFT, 0);
		input_set_button_state(INPUT_GAMEPAD_A, 0);
	}
}

TOUCH_EXPORT void shapewars_touch_pause(int down) {
	touch_used = true;
	input_set_button_state(INPUT_GAMEPAD_START, down ? 1.0f : 0.0f);
}

TOUCH_EXPORT void shapewars_touch_reset(void) {
	tap_release_pending = false;
	shapewars_touch_move(0, 0);
	shapewars_touch_aim(0, 0);
	input_set_button_state(INPUT_MOUSE_LEFT, 0);
	input_set_button_state(INPUT_GAMEPAD_A, 0);
	input_set_button_state(INPUT_GAMEPAD_START, 0);
}

bool touch_controls_active(void) {
	return touch_used;
}

void mobile_set_ui_mode(int mode) {
#if defined(__EMSCRIPTEN__)
	EM_ASM({
		if (typeof window.__shapewarsSetTouchMode === "function") {
			window.__shapewarsSetTouchMode($0);
		}
	}, mode);
#else
	(void)mode;
#endif
}
