#ifndef MAIN_H
#define MAIN_H

#include "../high_impact/src/types.h"
#include "../high_impact/src/utils.h"
#include "../high_impact/src/alloc.h"
#include "../high_impact/src/render.h"
#include "../high_impact/src/input.h"
#include "../high_impact/src/engine.h"
#include "../high_impact/src/font.h"
#include "../high_impact/src/sound.h"

#define PLAY_BORDER 40
#define PLAY_LEFT   40
#define PLAY_TOP    40
#define PLAY_RIGHT  1240
#define PLAY_BOTTOM 680
#define PLAY_W      1200
#define PLAY_H      640

#define MAX_ENTITIES 4096
#define ENTITY_TYPE_NONE 0

typedef enum {
    ENTITY_PLAYER = 1,
    ENTITY_ENEMY_SQUARE,
    ENTITY_ENEMY_CIRCLE,
    ENTITY_ENEMY_TRIANGLE,
    ENTITY_ENEMY_HEXAGON,
    ENTITY_BULLET,
    ENTITY_GAMBO,
    ENTITY_EXPLOSION,
    ENTITY_MINE,
    ENTITY_BLACKHOLE
} entity_type_t;

typedef struct entity_t entity_t;
typedef void (*entity_update_fn)(entity_t *self);
typedef void (*entity_draw_fn)(entity_t *self);
typedef void (*entity_kill_fn)(entity_t *self);

struct entity_t {
    bool is_alive;
    entity_type_t type;
    vec2_t pos;
    vec2_t size;
    vec2_t vel;
    float angle;
    float health;
    float invuln_timer;
    entity_update_fn update;
    entity_draw_fn draw;
    entity_kill_fn kill;
};

typedef struct {
    entity_t *entities[MAX_ENTITIES];
    int len;
} entity_list_t;

#define INPUT_ACTION_LEFT  0
#define INPUT_ACTION_RIGHT 1
#define INPUT_ACTION_UP    2
#define INPUT_ACTION_DOWN  3
#define INPUT_ACTION_SHOOT 4
#define INPUT_ACTION_RESTART 5
#define INPUT_ACTION_AIM_LEFT   6
#define INPUT_ACTION_AIM_RIGHT  7
#define INPUT_ACTION_AIM_UP     8
#define INPUT_ACTION_AIM_DOWN   9

typedef struct {
    font_t *font;
    int score;
    int lives;
    float enemy_spawn_timer;
    float shoot_timer;
    vec2_t mouse_pos;
    bool mouse_buttons[3];
    texture_t white_tex;
    texture_t circle_tex;
    vec2_t circle_tex_size;
    texture_t light_tex;
    bool game_over;
    float shake_timer;
    float shake_intensity;
    int gambo_count;
    int total_kills;
    int last_bh_spawn_kills;
    float spreadshot_timer;
    vec2_t aim_vec;
    int game_mode;
} global_t;

extern global_t g;
extern scene_t scene_game;
extern scene_t scene_start_screen;

entity_t *entity_create(entity_type_t type);
void *entity_alloc(size_t size);
void entity_remove(entity_t *e);
entity_t *entity_first(entity_type_t type);
entity_list_t entities_by_types(entity_type_t *types, int count);
void entity_kill(entity_t *e);
void entity_update_all(void);
void entity_draw_all(void);
void entity_draw_lights(void);
void entity_draw_only(entity_type_t *types, int count);
void entity_draw_all_except(entity_type_t *types, int count);
void entity_clear_all(void);
void entity_cleanup(void);

entity_t *entity_player_create(vec2_t pos);
entity_t *entity_enemy_create(entity_type_t type, vec2_t pos);
entity_t *entity_bullet_create(vec2_t pos, vec2_t direction, float speed);
entity_t *entity_gambo_create(vec2_t pos);
entity_t *entity_explosion_create(vec2_t pos, float size, float damage, bool from_mine);
entity_t *entity_mine_create(vec2_t pos);
entity_t *entity_blackhole_create(vec2_t pos);

void render_push_matrix(void);
void render_pop_matrix(void);
void render_draw_line(float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b);
void render_draw_rect(float x, float y, float w, float h, uint8_t r, uint8_t g, uint8_t b);
void render_draw_circle(float cx, float cy, float radius, uint8_t r, uint8_t g, uint8_t b);
void render_draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, uint8_t r, uint8_t g, uint8_t b);
void render_reset_textures(void);
void render_set_clear_color(rgba_t color);

bool input_down(uint8_t action);
bool input_mouse_down(int button);

static inline vec2_t vec2_normalize(vec2_t v) {
    float len = sqrtf(v.x * v.x + v.y * v.y);
    if (len > 0) return vec2(v.x / len, v.y / len);
    return vec2(0, 0);
}

static inline vec2_t vec2_rotate(vec2_t v, float angle) {
    float c = cosf(angle), s = sinf(angle);
    return vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

#endif
