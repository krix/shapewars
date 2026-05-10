#include "main.h"

global_t g;

void main_init(void) {
    g.score = 0;
    g.enemy_spawn_timer = 0;
    g.shoot_timer = 0;
    g.mouse_pos = vec2(0, 0);
    g.game_mode = 1;
    
    engine_set_scene(&scene_start_screen);
}

void main_cleanup(void) {
    // Cleanup code if needed
}