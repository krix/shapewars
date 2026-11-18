#include "../main.h"
#include "../particle.h"
#include "../spacetime.h"
#include "../high_impact/src/platform.h"
#include "../sounds.h"
#include "../touch_input.h"

#define NUM_STARS 120
#define INPUT_ACTION_PAUSE 10
#define PAUSE_RETURN     0
#define PAUSE_MAIN_MENU  1

typedef struct {
    float x, y;
    float depth;
} star_t;

static entity_t *player;
static entity_t *player2;
static bool game_running;
static bool paused;
static int pause_selected;
static float p2_shoot_timer;
static float powerup_spawn_timer;
static star_t stars[NUM_STARS];

static void init(void) {
    entity_clear_all();
    render_reset_textures();
    mobile_set_ui_mode(1);

    g.score = 0;
    g.lives = 3;
    g.gambo_count = 0;
    g.total_kills = 0;
    g.last_bh_spawn_kills = 0;
    g.spreadshot_timer = 0;
    g.shield_timer = 0;
    g.magnet_timer = 0;
    g.game_over = false;
    g.aim_vec = vec2(0, 0);
    g.p2_aim = vec2(0, 0);
    game_running = true;
    paused = false;
    p2_shoot_timer = 0;
    powerup_spawn_timer = 10.0f;

    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = rand_float(0, PLAY_W);
        stars[i].y = rand_float(0, PLAY_H);
        stars[i].depth = rand_float(0.1f, 1.0f);
    }
    
    player = entity_player_create(vec2(PLAY_LEFT + PLAY_W / 2, PLAY_TOP + PLAY_H / 2));
    player2 = NULL;
    
    input_unbind_all();
    input_bind(INPUT_KEY_LEFT, INPUT_ACTION_LEFT);
    input_bind(INPUT_KEY_RIGHT, INPUT_ACTION_RIGHT);
    input_bind(INPUT_KEY_UP, INPUT_ACTION_UP);
    input_bind(INPUT_KEY_DOWN, INPUT_ACTION_DOWN);
    input_bind(INPUT_KEY_A, INPUT_ACTION_LEFT);
    input_bind(INPUT_KEY_D, INPUT_ACTION_RIGHT);
    input_bind(INPUT_KEY_W, INPUT_ACTION_UP);
    input_bind(INPUT_KEY_S, INPUT_ACTION_DOWN);
    input_bind(INPUT_MOUSE_LEFT, INPUT_ACTION_SHOOT);
    input_bind(INPUT_KEY_RETURN, INPUT_ACTION_RESTART);
    input_bind(INPUT_KEY_KP_ENTER, INPUT_ACTION_RESTART);
    input_bind(INPUT_KEY_ESCAPE, INPUT_ACTION_PAUSE);

    // Gamepad 1 always controls player 1
    input_bind(INPUT_GAMEPAD_L_STICK_LEFT, INPUT_ACTION_LEFT);
    input_bind(INPUT_GAMEPAD_L_STICK_RIGHT, INPUT_ACTION_RIGHT);
    input_bind(INPUT_GAMEPAD_L_STICK_UP, INPUT_ACTION_UP);
    input_bind(INPUT_GAMEPAD_L_STICK_DOWN, INPUT_ACTION_DOWN);
    input_bind(INPUT_GAMEPAD_DPAD_LEFT, INPUT_ACTION_LEFT);
    input_bind(INPUT_GAMEPAD_DPAD_RIGHT, INPUT_ACTION_RIGHT);
    input_bind(INPUT_GAMEPAD_DPAD_UP, INPUT_ACTION_UP);
    input_bind(INPUT_GAMEPAD_DPAD_DOWN, INPUT_ACTION_DOWN);
    input_bind(INPUT_GAMEPAD_R_STICK_LEFT, INPUT_ACTION_AIM_LEFT);
    input_bind(INPUT_GAMEPAD_R_STICK_RIGHT, INPUT_ACTION_AIM_RIGHT);
    input_bind(INPUT_GAMEPAD_R_STICK_UP, INPUT_ACTION_AIM_UP);
    input_bind(INPUT_GAMEPAD_R_STICK_DOWN, INPUT_ACTION_AIM_DOWN);
    input_bind(INPUT_GAMEPAD_R_TRIGGER, INPUT_ACTION_SHOOT);
    input_bind(INPUT_GAMEPAD_A, INPUT_ACTION_RESTART);
    input_bind(INPUT_GAMEPAD_START, INPUT_ACTION_PAUSE);

    if (g.game_mode == 2) {
        // 2-player mode: second gamepad controls player 2
        input_bind(INPUT_GAMEPAD2_L_STICK_LEFT, INPUT_ACTION_P2_LEFT);
        input_bind(INPUT_GAMEPAD2_L_STICK_RIGHT, INPUT_ACTION_P2_RIGHT);
        input_bind(INPUT_GAMEPAD2_L_STICK_UP, INPUT_ACTION_P2_UP);
        input_bind(INPUT_GAMEPAD2_L_STICK_DOWN, INPUT_ACTION_P2_DOWN);
        input_bind(INPUT_GAMEPAD2_R_STICK_LEFT, INPUT_ACTION_P2_AIM_LEFT);
        input_bind(INPUT_GAMEPAD2_R_STICK_RIGHT, INPUT_ACTION_P2_AIM_RIGHT);
        input_bind(INPUT_GAMEPAD2_R_STICK_UP, INPUT_ACTION_P2_AIM_UP);
        input_bind(INPUT_GAMEPAD2_R_STICK_DOWN, INPUT_ACTION_P2_AIM_DOWN);
        input_bind(INPUT_GAMEPAD2_R_TRIGGER, INPUT_ACTION_P2_SHOOT);
        input_bind(INPUT_GAMEPAD2_A, INPUT_ACTION_RESTART);
        input_bind(INPUT_GAMEPAD2_START, INPUT_ACTION_PAUSE);

        player2 = entity_player2_create(vec2(PLAY_LEFT + PLAY_W / 2 - 40, PLAY_TOP + PLAY_H / 2));
    }
    
    g.font = font("font_sans-serif.qoi", "font_sans-serif.json");
    if (g.font) {
        g.font->color = rgba(255, 255, 255, 255);
    }
    
    sounds_init();
    spacetime_init();
    particles_init();
    render_set_clear_color(rgba(20, 24, 40, 255));
}

static void update(void) {
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x -= 15.0f * stars[i].depth * engine.tick;
        if (stars[i].x < 0) stars[i].x += PLAY_W;
    }

    if (!g.game_over && input_pressed(INPUT_ACTION_PAUSE)) {
        paused = !paused;
    }

    if (paused) {
        if (input_pressed(INPUT_ACTION_UP)) {
            pause_selected = (pause_selected - 1 + 2) % 2;
        }
        if (input_pressed(INPUT_ACTION_DOWN)) {
            pause_selected = (pause_selected + 1) % 2;
        }
        if (input_pressed(INPUT_ACTION_RESTART)) {
            if (pause_selected == PAUSE_MAIN_MENU) {
                engine_set_scene(&scene_start_screen);
            } else {
                paused = false;
            }
        }
        return;
    }

    if (g.game_over) {
        if (g.shake_timer > 0) g.shake_timer -= engine.tick;
        spacetime_update();
        entity_update_all();
        particles_update();
        entity_cleanup();
        scene_base_update();
        if (input_down(INPUT_ACTION_RESTART)) {
            engine_set_scene(&scene_game);
        }
        return;
    }

    if (g.shake_timer > 0) g.shake_timer -= engine.tick;
    if (g.spreadshot_timer > 0) g.spreadshot_timer -= engine.tick;

    g.mouse_pos = input_mouse_pos();

    {
        float rx = platform_gamepad_right_stick.x;
        float ry = platform_gamepad_right_stick.y;
        if (fabsf(rx) > 0.15f || fabsf(ry) > 0.15f) {
            g.aim_vec = vec2_normalize(vec2(rx, ry));
        } else {
            g.aim_vec = vec2(0, 0);
        }
    }

    if (g.game_mode == 2) {
        float rx = platform_gamepad2_right_stick.x;
        float ry = platform_gamepad2_right_stick.y;
        if (fabsf(rx) > 0.15f || fabsf(ry) > 0.15f) {
            g.p2_aim = vec2_normalize(vec2(rx, ry));
        } else {
            g.p2_aim = vec2(0, 0);
        }
    }

    g.enemy_spawn_timer -= engine.tick;
    if (g.enemy_spawn_timer <= 0) {
        vec2_t spawn_pos;
        int edge = rand_int(0, 3);
        switch (edge) {
            case 0:
                spawn_pos = vec2(rand_float(PLAY_LEFT, PLAY_RIGHT), PLAY_TOP - 20);
                break;
            case 1:
                spawn_pos = vec2(PLAY_RIGHT + 20, rand_float(PLAY_TOP, PLAY_BOTTOM));
                break;
            case 2:
                spawn_pos = vec2(rand_float(PLAY_LEFT, PLAY_RIGHT), PLAY_BOTTOM + 20);
                break;
            case 3:
                spawn_pos = vec2(PLAY_LEFT - 20, rand_float(PLAY_TOP, PLAY_BOTTOM));
                break;
        }
        
        int avail = 1;
        if (g.total_kills >= 50) avail = 2;
        if (g.total_kills >= 250) avail = 3;
        if (g.total_kills >= 1000) avail = 4;
        entity_type_t types[] = {ENTITY_ENEMY_TRIANGLE, ENTITY_ENEMY_SQUARE, ENTITY_ENEMY_HEXAGON, ENTITY_ENEMY_CIRCLE};
        int weights[] = {5, 3, 2, 1};
        int total = 0;
        for (int i = 0; i < avail; i++) total += weights[i];
        int r = rand_int(0, total - 1);
        int picked = 0;
        for (int i = 0; i < avail; i++) {
            r -= weights[i];
            if (r < 0) { picked = i; break; }
        }
        entity_enemy_create(types[picked], spawn_pos);
        
        g.enemy_spawn_timer = 0.4f - (g.total_kills / 2000.0f);
        if (g.enemy_spawn_timer < 0.04f) g.enemy_spawn_timer = 0.04f;
    }
    
    if (g.shield_timer > 0) g.shield_timer -= engine.tick;
    if (g.magnet_timer > 0) g.magnet_timer -= engine.tick;

    powerup_spawn_timer -= engine.tick;
    if (powerup_spawn_timer <= 0) {
        int ptype = rand_int(0, 1);
        entity_powerup_create(vec2(
            rand_float(PLAY_LEFT + 60, PLAY_RIGHT - 60),
            rand_float(PLAY_TOP + 60, PLAY_BOTTOM - 60)
        ), ptype);
        powerup_spawn_timer = rand_float(8.0f, 18.0f);
    }

    if (g.total_kills >= g.last_bh_spawn_kills + 100) {
        g.last_bh_spawn_kills += 100;
        entity_type_t bh_check[] = {ENTITY_BLACKHOLE};
        entity_list_t existing = entities_by_types(bh_check, 1);
        if (existing.len < 5) {
            entity_blackhole_create(vec2(
                rand_float(PLAY_LEFT + 60, PLAY_RIGHT - 60),
                rand_float(PLAY_TOP + 60, PLAY_BOTTOM - 60)
            ));
        }
    }
    
    if (input_state(INPUT_ACTION_SHOOT) > 0.5f || vec2_len(g.aim_vec) > 0) {
        g.shoot_timer -= engine.tick;
        if (g.shoot_timer <= 0) {
            vec2_t dir;
            if (vec2_len(g.aim_vec) > 0) {
                dir = g.aim_vec;
            } else {
                dir = vec2_sub(g.mouse_pos, player->pos);
                dir = vec2_normalize(dir);
            }
            
            for (int i = 0; i < 3; i++)
                entity_bullet_create(player->pos, vec2_rotate(dir, rand_float(-0.08f, 0.08f)), rand_float(1200, 2400));
            
            sound_play(sound_shoot);
            if (g.spreadshot_timer > 0) {
                for (int i = 0; i < 16; i++)
                    entity_bullet_create(player->pos, vec2_rotate(dir, rand_float(-0.35f, 0.35f)), rand_float(1200, 2400));
                g.shoot_timer = 0.025f;
            } else {
                g.shoot_timer = 0.07f;
            }
        }
    } else {
        g.shoot_timer = 0;
    }

    if (g.game_mode == 2 && player2 && player2->is_alive) {
        if (input_state(INPUT_ACTION_P2_SHOOT) > 0.5f || vec2_len(g.p2_aim) > 0) {
            p2_shoot_timer -= engine.tick;
            if (p2_shoot_timer <= 0) {
                vec2_t dir;
                if (vec2_len(g.p2_aim) > 0) {
                    dir = g.p2_aim;
                } else {
                    dir = vec2(1, 0);
                }
                for (int i = 0; i < 3; i++)
                    entity_bullet_create(player2->pos, vec2_rotate(dir, rand_float(-0.08f, 0.08f)), rand_float(1200, 2400));
                sound_play(sound_shoot);
                if (g.spreadshot_timer > 0) {
                    for (int i = 0; i < 16; i++)
                        entity_bullet_create(player2->pos, vec2_rotate(dir, rand_float(-0.35f, 0.35f)), rand_float(1200, 2400));
                    p2_shoot_timer = 0.025f;
                } else {
                    p2_shoot_timer = 0.07f;
                }
            }
        } else {
            p2_shoot_timer = 0;
        }
    }

    spacetime_update();
    entity_update_all();
    particles_update();
    entity_cleanup();
    scene_base_update();
}

static void draw_lives(void) {
    for (int i = 0; i < 3; i++) {
        float px = 640.0f + (i - 1) * 20.0f;
        uint8_t b = (i < g.lives) ? 255 : 40;
        render_push_matrix();
        render_translate(vec2(px, 16));
        render_rotate(-(float)M_PI / 2.0f);
        render_draw_line(10, 0, 2, -6, b, b, b);
        render_draw_line(2, -6, -4, -3, b, b, b);
        render_draw_line(-4, -3, -5, -1, b, b, b);
        render_draw_line(-5, -1, -5, 1, b, b, b);
        render_draw_line(-5, 1, -4, 3, b, b, b);
        render_draw_line(-4, 3, 2, 6, b, b, b);
        render_draw_line(2, 6, 10, 0, b, b, b);
        render_pop_matrix();
    }
}

static void draw_hud(void) {
    if (!g.font) return;
    render_push();
    render_translate(vec2(PLAY_LEFT, 4));
    font_draw(g.font, vec2(0, 0), str_format("S:%d", g.score), FONT_ALIGN_LEFT);
    render_pop();
    render_push();
    int tw = font_line_width(g.font, str_format("G:%d/200", g.gambo_count));
    render_translate(vec2(PLAY_RIGHT - tw, 4));
    font_draw(g.font, vec2(0, 0), str_format("G:%d/200", g.gambo_count), FONT_ALIGN_LEFT);
    render_pop();
}

static void draw_pause_menu(void) {
    if (!g.font) return;

    render_push();
    render_draw(vec2(0, 0), vec2(1280, 720), g.white_tex, vec2(0, 0), vec2(1, 1), rgba(0, 0, 0, 160));
    render_pop();

    const char *title = "PAUSED";
    int tw = font_line_width(g.font, (char*)title);
    render_push();
    render_translate(vec2((1280 - tw) / 2, 240));
    g.font->color = rgba(255, 255, 255, 255);
    font_draw(g.font, vec2(0, 0), (char*)title, FONT_ALIGN_LEFT);
    render_pop();

    const char *options[2] = {"RETURN", "MAIN MENU"};
    for (int i = 0; i < 2; i++) {
        char *text;
        if (i == pause_selected) {
            text = str_format("> %s", options[i]);
            g.font->color = rgba(0, 255, 80, 255);
        } else {
            text = str_format("  %s", options[i]);
            g.font->color = rgba(180, 180, 200, 255);
        }
        tw = font_line_width(g.font, text);
        render_push();
        render_translate(vec2((1280 - tw) / 2, 320 + i * 60));
        font_draw(g.font, vec2(0, 0), text, FONT_ALIGN_LEFT);
        render_pop();
    }
}

static void draw(void) {
    render_draw_rect(0, 0, 1280, 720, 8, 10, 20);
    render_draw_rect(PLAY_LEFT, PLAY_TOP, PLAY_W, PLAY_H, 16, 18, 32);
    spacetime_draw();
    entity_draw_lights();

    if (g.shake_timer > 0) {
        float intensity = g.shake_intensity * (g.shake_timer / 0.3f);
        render_push();
        render_translate(vec2(rand_float(-intensity, intensity), rand_float(-intensity, intensity)));
    }

    for (int i = 0; i < NUM_STARS; i++) {
        float sx = PLAY_LEFT + stars[i].x;
        float sy = PLAY_TOP + stars[i].y;
        uint8_t b = (uint8_t)(80 + stars[i].depth * 120);
        float sz = 1.0f + stars[i].depth * 1.0f;
        render_draw_circle(sx, sy, sz * 0.5f, b, b, b + 10);
    }

    entity_type_t bh_type[] = {ENTITY_BLACKHOLE};
    entity_draw_only(bh_type, 1);
    entity_draw_all_except(bh_type, 1);
    particles_draw();

    render_draw_rect(PLAY_LEFT, PLAY_TOP, PLAY_W, 2, 80, 100, 160);
    render_draw_rect(PLAY_LEFT, PLAY_TOP, 2, PLAY_H, 80, 100, 160);
    render_draw_rect(PLAY_RIGHT - 2, PLAY_TOP, 2, PLAY_H, 80, 100, 160);
    render_draw_rect(PLAY_LEFT, PLAY_BOTTOM - 2, PLAY_W, 2, 80, 100, 160);

    scene_base_draw();

    if (g.shake_timer > 0) render_pop();

    if (vec2_len(g.aim_vec) == 0 && !touch_controls_active()) {
        float cx = g.mouse_pos.x, cy = g.mouse_pos.y;
        render_draw_rect(cx - 8, cy - 1, 5, 2, 220, 220, 220);
        render_draw_rect(cx + 3, cy - 1, 5, 2, 220, 220, 220);
        render_draw_rect(cx - 1, cy - 8, 2, 5, 220, 220, 220);
        render_draw_rect(cx - 1, cy + 3, 2, 5, 220, 220, 220);
    }

    draw_lives();

    draw_hud();
    
    if (g.game_over) {
        if (g.font) {
            vec2_t size = vec2_from_vec2i(render_size());
            int tw = font_line_width(g.font, "GAME OVER");
            render_push();
            render_translate(vec2((size.x - tw) / 2, size.y / 2 - 48));
            font_draw(g.font, vec2(0, 0), "GAME OVER", FONT_ALIGN_LEFT);
            render_pop();
            const char *restart_text = touch_controls_active() ? "Tap to restart" : "Press ENTER to restart";
            tw = font_line_width(g.font, (char*)restart_text);
            render_push();
            render_translate(vec2((size.x - tw) / 2, size.y / 2 + 16));
            font_draw(g.font, vec2(0, 0), (char*)restart_text, FONT_ALIGN_LEFT);
            render_pop();
        } else {
            render_draw_rect(200, 280, 400, 40, 160, 0, 0);
            render_draw_line(200, 300, 600, 300, 255, 255, 255);
            render_draw_line(240, 290, 240, 310, 255, 255, 255);
            render_draw_line(260, 290, 260, 310, 255, 255, 255);
        }
    }

    if (paused) draw_pause_menu();

    // Glow pass: draw enemies to glow FBO for selective bloom
    {
        entity_type_t glow_enemy_types[] = {
            ENTITY_PLAYER,
            ENTITY_PLAYER2,
            ENTITY_ENEMY_TRIANGLE,
            ENTITY_ENEMY_SQUARE,
            ENTITY_ENEMY_HEXAGON,
            ENTITY_ENEMY_CIRCLE,
            ENTITY_BULLET,
            ENTITY_MINE,
            ENTITY_EXPLOSION,
            ENTITY_POWERUP_SHIELD,
            ENTITY_POWERUP_MAGNET
        };
        render_flush();
        render_bind_glow_fbo();
        entity_draw_only(glow_enemy_types, 11);
        draw_lives();
        draw_hud();
        if (paused) draw_pause_menu();
        render_flush();
        render_unbind_glow_fbo();
    }
}

static void cleanup(void) {
    entity_clear_all();
    particles_clear();
}

scene_t scene_game = {
    .init = init,
    .update = update,
    .draw = draw,
    .cleanup = cleanup
};
