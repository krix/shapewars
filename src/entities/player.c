#include "../main.h"
#include "../sounds.h"

#define PLAYER_SPEED 300.0f

static void update(entity_t *self) {
    if (g.game_over) return;

    self->invuln_timer -= engine.tick;
    if (self->invuln_timer < 0) self->invuln_timer = 0;

    // Check collision with enemies (only when not invulnerable and no shield)
    if (self->invuln_timer <= 0 && g.shield_timer <= 0) {
        entity_type_t enemy_types[] = {ENTITY_ENEMY_SQUARE, ENTITY_ENEMY_CIRCLE, ENTITY_ENEMY_TRIANGLE, ENTITY_ENEMY_HEXAGON};
        entity_list_t enemies = entities_by_types(enemy_types, 4);
        for (int i = 0; i < enemies.len; i++) {
            entity_t *enemy = enemies.entities[i];
            if (enemy->invuln_timer > 0) continue;
            float dx = self->pos.x - enemy->pos.x;
            float dy = self->pos.y - enemy->pos.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < 20.0f) {
                g.lives--;
                sound_play(sound_player_hit);
                if (g.lives <= 0) {
                    entity_explosion_create(self->pos, 25, 20, true, true);
                    g.game_over = true;
                    entity_remove(self);
                    return;
                }
                entity_explosion_create(self->pos, 25, 20, true, true);
                self->invuln_timer = 2.0f;
                break;
            }
        }
    }

    // Update player position based on input (analog for controller, digital for keyboard)
    vec2_t dir;
    dir.x = input_state(INPUT_ACTION_RIGHT) - input_state(INPUT_ACTION_LEFT);
    dir.y = input_state(INPUT_ACTION_DOWN) - input_state(INPUT_ACTION_UP);
    float dl = vec2_len(dir);
    vec2_t velocity = (dl > 0) ? vec2_mulf(dir, PLAYER_SPEED / dl) : vec2(0, 0);
    
    // Store velocity for enemy prediction
    self->vel = velocity;
    
    // Update position
    self->pos = vec2_add(self->pos, vec2_mulf(velocity, engine.tick));
    
    // Constrain to play area bounds
    float margin = 10.0f;
    if (self->pos.x < PLAY_LEFT + margin) self->pos.x = PLAY_LEFT + margin;
    if (self->pos.x > PLAY_RIGHT - margin) self->pos.x = PLAY_RIGHT - margin;
    if (self->pos.y < PLAY_TOP + margin) self->pos.y = PLAY_TOP + margin;
    if (self->pos.y > PLAY_BOTTOM - margin) self->pos.y = PLAY_BOTTOM - margin;
    
    if (vec2_len(g.aim_vec) > 0) {
        self->angle = atan2f(g.aim_vec.y, g.aim_vec.x);
    } else {
        vec2_t dir = vec2_sub(g.mouse_pos, self->pos);
        self->angle = atan2f(dir.y, dir.x);
    }
}

static void draw(entity_t *self) {
    if (g.game_over) return;
    if (self->invuln_timer > 0 && fmodf(self->invuln_timer * 6, 1) > 0.5f) return;

    render_push_matrix();
    render_translate(self->pos);
    render_rotate(self->angle);
    
    uint8_t brightness = self->invuln_timer > 0 ? 128 : 255;
    render_draw_line(10, 0, 2, -6, brightness, brightness, brightness);
    render_draw_line(2, -6, -4, -3, brightness, brightness, brightness);
    render_draw_line(-4, -3, -5, -1, brightness, brightness, brightness);
    render_draw_line(-5, -1, -5, 1, brightness, brightness, brightness);
    render_draw_line(-5, 1, -4, 3, brightness, brightness, brightness);
    render_draw_line(-4, 3, 2, 6, brightness, brightness, brightness);
    render_draw_line(2, 6, 10, 0, brightness, brightness, brightness);
    
    render_pop_matrix();

    if (g.shield_timer > 0) {
        bool flash = g.shield_timer <= 2.0f && fmodf(g.shield_timer * 6, 1) > 0.5f;
        if (g.shield_timer > 2.0f || flash) {
            float r = 18.0f;
            int segs = 24;
            for (int pass = 0; pass < 2; pass++) {
                float radius = r + pass * 0.8f;
                for (int j = 0; j < segs; j++) {
                    float a1 = j * (2.0f * (float)M_PI / segs);
                    float a2 = (j + 1) * (2.0f * (float)M_PI / segs);
                    float x1 = self->pos.x + cosf(a1) * radius;
                    float y1 = self->pos.y + sinf(a1) * radius;
                    float x2 = self->pos.x + cosf(a2) * radius;
                    float y2 = self->pos.y + sinf(a2) * radius;
                    render_draw_line(x1, y1, x2, y2, 60, 80, 200);
                }
            }
        }
    }
}

static void p2_update(entity_t *self) {
    if (g.game_over) return;

    self->invuln_timer -= engine.tick;
    if (self->invuln_timer < 0) self->invuln_timer = 0;

    // Check collision with enemies (only when not invulnerable and no shield)
    if (self->invuln_timer <= 0 && g.shield_timer <= 0) {
        entity_type_t enemy_types[] = {ENTITY_ENEMY_SQUARE, ENTITY_ENEMY_CIRCLE, ENTITY_ENEMY_TRIANGLE, ENTITY_ENEMY_HEXAGON};
        entity_list_t enemies = entities_by_types(enemy_types, 4);
        for (int i = 0; i < enemies.len; i++) {
            entity_t *enemy = enemies.entities[i];
            if (enemy->invuln_timer > 0) continue;
            float dx = self->pos.x - enemy->pos.x;
            float dy = self->pos.y - enemy->pos.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < 20.0f) {
                g.lives--;
                sound_play(sound_player_hit);
                if (g.lives <= 0) {
                    entity_explosion_create(self->pos, 25, 20, true, true);
                    g.game_over = true;
                    entity_remove(self);
                    return;
                }
                entity_explosion_create(self->pos, 25, 20, true, true);
                self->invuln_timer = 2.0f;
                break;
            }
        }
    }

    // Movement from left stick / dpad
    vec2_t dir;
    dir.x = input_state(INPUT_ACTION_P2_RIGHT) - input_state(INPUT_ACTION_P2_LEFT);
    dir.y = input_state(INPUT_ACTION_P2_DOWN) - input_state(INPUT_ACTION_P2_UP);
    float dl = vec2_len(dir);
    vec2_t velocity = (dl > 0) ? vec2_mulf(dir, PLAYER_SPEED / dl) : vec2(0, 0);
    self->vel = velocity;
    self->pos = vec2_add(self->pos, vec2_mulf(velocity, engine.tick));

    float margin = 10.0f;
    if (self->pos.x < PLAY_LEFT + margin) self->pos.x = PLAY_LEFT + margin;
    if (self->pos.x > PLAY_RIGHT - margin) self->pos.x = PLAY_RIGHT - margin;
    if (self->pos.y < PLAY_TOP + margin) self->pos.y = PLAY_TOP + margin;
    if (self->pos.y > PLAY_BOTTOM - margin) self->pos.y = PLAY_BOTTOM - margin;

    // Aim from right stick
    vec2_t aim;
    aim.x = input_state(INPUT_ACTION_P2_AIM_RIGHT) - input_state(INPUT_ACTION_P2_AIM_LEFT);
    aim.y = input_state(INPUT_ACTION_P2_AIM_DOWN) - input_state(INPUT_ACTION_P2_AIM_UP);
    float al = vec2_len(aim);
    if (al > 0.5f) {
        g.p2_aim = vec2_normalize(aim);
        self->angle = atan2f(g.p2_aim.y, g.p2_aim.x);
    }
}

static void p2_draw(entity_t *self) {
    if (g.game_over) return;
    if (self->invuln_timer > 0 && fmodf(self->invuln_timer * 6, 1) > 0.5f) return;

    render_push_matrix();
    render_translate(self->pos);
    render_rotate(self->angle);

    uint8_t base = self->invuln_timer > 0 ? 128 : 255;
    render_draw_line(10, 0, 2, -6, 0, base, 0);
    render_draw_line(2, -6, -4, -3, 0, base, 0);
    render_draw_line(-4, -3, -5, -1, 0, base, 0);
    render_draw_line(-5, -1, -5, 1, 0, base, 0);
    render_draw_line(-5, 1, -4, 3, 0, base, 0);
    render_draw_line(-4, 3, 2, 6, 0, base, 0);
    render_draw_line(2, 6, 10, 0, 0, base, 0);

    render_pop_matrix();

    if (g.shield_timer > 0) {
        bool flash = g.shield_timer <= 2.0f && fmodf(g.shield_timer * 6, 1) > 0.5f;
        if (g.shield_timer > 2.0f || flash) {
            float r = 18.0f;
            int segs = 24;
            for (int pass = 0; pass < 2; pass++) {
                float radius = r + pass * 0.8f;
                for (int j = 0; j < segs; j++) {
                    float a1 = j * (2.0f * (float)M_PI / segs);
                    float a2 = (j + 1) * (2.0f * (float)M_PI / segs);
                    float x1 = self->pos.x + cosf(a1) * radius;
                    float y1 = self->pos.y + sinf(a1) * radius;
                    float x2 = self->pos.x + cosf(a2) * radius;
                    float y2 = self->pos.y + sinf(a2) * radius;
                    render_draw_line(x1, y1, x2, y2, 40, 180, 80);
                }
            }
        }
    }
}

entity_t *entity_player_create(vec2_t pos) {
    entity_t *e = entity_create(ENTITY_PLAYER);
    e->type = ENTITY_PLAYER;
    e->pos = pos;
    e->size = vec2(20, 20);
    e->update = update;
    e->draw = draw;
    return e;
}

entity_t *entity_player2_create(vec2_t pos) {
    entity_t *e = entity_create(ENTITY_PLAYER2);
    e->type = ENTITY_PLAYER2;
    e->pos = pos;
    e->size = vec2(20, 20);
    e->update = p2_update;
    e->draw = p2_draw;
    return e;
}