#include "../main.h"
#include "../high_impact/src/platform.h"
#include "../sounds.h"
#include <string.h>

#define ACTION_UP     10
#define ACTION_DOWN   11
#define ACTION_SELECT 12
#define NUM_MENU_ITEMS 3
#define NUM_STARS 120

typedef struct { float x, y; float depth; } star_t;

static star_t stars[NUM_STARS];
static const char *menu_texts[NUM_MENU_ITEMS] = {
    "1 Player Mode",
    "2 Player Mode",
    "Exit"
};

static int selected = 0;
static int hovered = -1;
static float menu_x, menu_start_y;
static float menu_item_w, menu_item_h;

typedef struct { float x1, y1, x2, y2; } seg_t;

// All letters are hollow outlines built from straight lines only

// All letters: clean hollow outlines using only straight lines

static const seg_t seg_S[] = {
    {0, 0, 1, 0},           // top
    {0, 0, 0, 0.5f},        // left upper
    {0, 0.5f, 1, 0.5f},     // middle
    {1, 0.5f, 1, 1},        // right lower
    {0, 1, 1, 1},           // bottom
};
static const seg_t seg_H[] = {
    {0, 0, 0, 1},           // left
    {1, 0, 1, 1},           // right
    {0, 0.5f, 1, 0.5f},     // crossbar
};
static const seg_t seg_A[] = {
    {0.5f, 0, 1, 1},        // right diagonal
    {0, 1, 0.5f, 0},        // left diagonal
    {0.2f, 0.55f, 0.8f, 0.55f}, // crossbar
};
static const seg_t seg_P[] = {
    {0, 0, 0, 1},           // left
    {0, 0, 1, 0},           // top
    {1, 0, 1, 0.5f},        // right of loop
    {0, 0.5f, 1, 0.5f},     // middle
};
static const seg_t seg_E[] = {
    {0, 0, 0, 1},           // left
    {0, 0, 1, 0},           // top
    {0, 0.5f, 0.8f, 0.5f},  // middle
    {0, 1, 1, 1},           // bottom
};
static const seg_t seg_W[] = {
    {0, 0, 0.33f, 1},       // left outer
    {0.33f, 1, 0.5f, 0.5f}, // left inner
    {0.5f, 0.5f, 0.67f, 1}, // right inner
    {0.67f, 1, 1, 0},       // right outer
};
static const seg_t seg_R[] = {
    {0, 0, 0, 1},           // left
    {0, 0, 1, 0},           // top
    {1, 0, 1, 0.5f},        // right of loop
    {0, 0.5f, 1, 0.5f},     // middle
    {0.5f, 0.5f, 1, 1},     // diagonal leg
};

typedef struct {
    const seg_t *segs;
    int count;
} letter_def_t;

static const letter_def_t letters[] = {
    {seg_S, sizeof(seg_S)/sizeof(seg_S[0])},
    {seg_H, sizeof(seg_H)/sizeof(seg_H[0])},
    {seg_A, sizeof(seg_A)/sizeof(seg_A[0])},
    {seg_P, sizeof(seg_P)/sizeof(seg_P[0])},
    {seg_E, sizeof(seg_E)/sizeof(seg_E[0])},
    {seg_W, sizeof(seg_W)/sizeof(seg_W[0])},
    {seg_A, sizeof(seg_A)/sizeof(seg_A[0])},
    {seg_R, sizeof(seg_R)/sizeof(seg_R[0])},
    {seg_S, sizeof(seg_S)/sizeof(seg_S[0])},
};

static void draw_thick_line(float x1, float y1, float x2, float y2, float thickness, uint8_t cr, uint8_t cg, uint8_t cb) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.5f) return;
    float angle = atan2f(dy, dx);
    render_push();
    render_translate(vec2(x1, y1));
    render_rotate(angle);
    render_draw(vec2(0, -thickness * 0.5f), vec2(len, thickness), g.white_tex, vec2(0, 0), vec2(1, 1), rgba(cr, cg, cb, 255));
    render_pop();
}

static void draw_letter(const letter_def_t *l, float x, float y, float w, float h, float thickness, uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < l->count; i++) {
        draw_thick_line(
            x + l->segs[i].x1 * w, y + l->segs[i].y1 * h,
            x + l->segs[i].x2 * w, y + l->segs[i].y2 * h,
            thickness, r, g, b
        );
    }
}

static void draw_title(void) {
    int len = 9;
    float spacing = 130.0f;
    float total_w = (len - 1) * spacing + 110.0f;
    float start_x = (1280.0f - total_w) / 2.0f;
    float y = 90.0f;
    float letter_w = 110.0f;
    float letter_h = 180.0f;
    float cx = start_x + total_w / 2.0f;

    float t = (float)engine.time * 0.65f;
    float shear = sinf(t) * 0.04f;
    for (int i = 0; i < len && i < sizeof(letters)/sizeof(letters[0]); i++) {
        float letter_x = start_x + i * spacing;
        float letter_cx = letter_x + letter_w / 2.0f;
        float dy = (letter_cx - cx) * shear;
        draw_letter(&letters[i], letter_x, y + dy, letter_w, letter_h, 6.0f, 255, 255, 255);
    }
}

static void select_item(int index) {
    switch (index) {
        case 0:
            g.game_mode = 1;
            engine_set_scene(&scene_game);
            break;
        case 1:
            g.game_mode = 2;
            engine_set_scene(&scene_game);
            break;
        case 2:
            platform_exit();
            break;
    }
}

static void init(void) {
    entity_clear_all();
    render_reset_textures();
    selected = 0;
    hovered = -1;
    menu_item_w = 280.0f;
    menu_item_h = 36.0f;
    menu_x = 640.0f;
    menu_start_y = 440.0f;

    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = rand_float(0, 1280.0f);
        stars[i].y = rand_float(0, 720.0f);
        stars[i].depth = rand_float(0.1f, 1.0f);
    }

    g.font = font("font_sans-serif.qoi", "font_sans-serif.json");
    if (g.font) {
        g.font->color = rgba(255, 255, 255, 255);
    }
    input_unbind_all();
    input_bind(INPUT_KEY_UP, ACTION_UP);
    input_bind(INPUT_KEY_DOWN, ACTION_DOWN);
    input_bind(INPUT_KEY_W, ACTION_UP);
    input_bind(INPUT_KEY_S, ACTION_DOWN);
    input_bind(INPUT_KEY_RETURN, ACTION_SELECT);
    input_bind(INPUT_KEY_KP_ENTER, ACTION_SELECT);
    input_bind(INPUT_KEY_ESCAPE, ACTION_SELECT);
    input_bind(INPUT_GAMEPAD_DPAD_UP, ACTION_UP);
    input_bind(INPUT_GAMEPAD_DPAD_DOWN, ACTION_DOWN);
    input_bind(INPUT_GAMEPAD_L_STICK_UP, ACTION_UP);
    input_bind(INPUT_GAMEPAD_L_STICK_DOWN, ACTION_DOWN);
    input_bind(INPUT_GAMEPAD_A, ACTION_SELECT);
    input_bind(INPUT_GAMEPAD_X, ACTION_SELECT);
    input_bind(INPUT_GAMEPAD_START, ACTION_SELECT);
    input_bind(INPUT_GAMEPAD2_DPAD_UP, ACTION_UP);
    input_bind(INPUT_GAMEPAD2_DPAD_DOWN, ACTION_DOWN);
    input_bind(INPUT_GAMEPAD2_L_STICK_UP, ACTION_UP);
    input_bind(INPUT_GAMEPAD2_L_STICK_DOWN, ACTION_DOWN);
    input_bind(INPUT_GAMEPAD2_A, ACTION_SELECT);
    input_bind(INPUT_GAMEPAD2_X, ACTION_SELECT);
    input_bind(INPUT_GAMEPAD2_START, ACTION_SELECT);
    input_bind(INPUT_MOUSE_LEFT, ACTION_SELECT);
    sounds_init();
    render_set_clear_color(rgba(35, 48, 72, 255));
}

static void update(void) {
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x -= 25.0f * stars[i].depth * engine.tick;
        if (stars[i].x < 0) stars[i].x += 1280.0f;
    }

    if (input_pressed(ACTION_UP)) {
        selected = (selected - 1 + NUM_MENU_ITEMS) % NUM_MENU_ITEMS;
    }
    if (input_pressed(ACTION_DOWN)) {
        selected = (selected + 1) % NUM_MENU_ITEMS;
    }
    vec2_t mouse = input_mouse_pos();
    hovered = -1;
    for (int i = 0; i < NUM_MENU_ITEMS; i++) {
        float ix = menu_x - menu_item_w / 2.0f;
        float iy = menu_start_y + i * 60.0f - menu_item_h / 2.0f;
        if (mouse.x >= ix && mouse.x <= ix + menu_item_w &&
            mouse.y >= iy && mouse.y <= iy + menu_item_h) {
            hovered = i;
            break;
        }
    }
    if (input_pressed(ACTION_SELECT)) {
        sound_play(sound_menu_select);
        if (hovered >= 0) select_item(hovered);
        else select_item(selected);
    }
}

static void draw_menu_item(int index, bool is_selected) {
    float y = menu_start_y + index * 60.0f;
    uint8_t cr, cg, cb;
    if (is_selected) {
        cr = 0; cg = 255; cb = 80;
    } else {
        cr = 180; cg = 180; cb = 200;
    }
    if (g.font) {
        g.font->color = rgba(cr, cg, cb, 255);
        int tw = font_line_width(g.font, (char*)menu_texts[index]);
        render_push();
        render_translate(vec2(menu_x - tw / 2.0f, y - 10.0f));
        font_draw(g.font, vec2(0, 0), (char*)menu_texts[index], FONT_ALIGN_LEFT);
        render_pop();
    }
}

static void draw(void) {
    render_draw_rect(0, 0, 1280, 720, 35, 48, 72);

    for (int i = 0; i < NUM_STARS; i++) {
        uint8_t b = (uint8_t)(80 + stars[i].depth * 120);
        float sz = 1.0f + stars[i].depth * 1.0f;
        render_draw_circle(stars[i].x, stars[i].y, sz * 0.5f, b, b, b + 10);
    }

    draw_title();
    for (int i = 0; i < NUM_MENU_ITEMS; i++) {
        draw_menu_item(i, i == hovered || (hovered < 0 && i == selected));
    }
    render_flush();
    render_bind_glow_fbo();
    draw_title();
    for (int i = 0; i < NUM_MENU_ITEMS; i++) {
        draw_menu_item(i, i == hovered || (hovered < 0 && i == selected));
    }
    render_flush();
    render_unbind_glow_fbo();
}

scene_t scene_start_screen = {
    .init = init,
    .update = update,
    .draw = draw
};
