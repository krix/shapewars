#ifndef SHAPEWARS_TOUCH_INPUT_H
#define SHAPEWARS_TOUCH_INPUT_H

#include <stdbool.h>

// Touch controls for the web build. The wasm shell calls these directly
// through the exported functions below. On other platforms they are no-ops
// unless the game explicitly feeds them.
void shapewars_touch_move(float x, float y);
void shapewars_touch_aim(float x, float y);
void shapewars_touch_tap(float nx, float ny, int down);
void shapewars_touch_pause(int down);
void shapewars_touch_reset(void);

// Called once per engine frame after input has been sampled. Finishes the
// synthetic release of a touch tap that was pressed and released between
// frames.
void touch_input_update(void);

// True after the player has used touch controls. Used to hide the mouse
// crosshair on devices without a pointer.
bool touch_controls_active(void);

// Tell the web shell which kind of interface to show:
//   0 = menu (taps only), 1 = game (two virtual sticks)
void mobile_set_ui_mode(int mode);

#endif
