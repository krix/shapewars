#include "../main.h"

#define PLAYER_SPEED 300.0f

static void update(entity_t *self) {
    if (g.game_over) return;

    self->invuln_timer -= engine.tick;
    if (self->invuln_timer < 0) self->invuln_timer = 0;

    // Check collision with enemies (only when not invulnerable)
    if (self->invuln_timer <= 0) {
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
                if (g.lives <= 0) {
                    entity_explosion_create(self->pos, 25, 20, true);
                    g.game_over = true;
                    entity_remove(self);
                    return;
                }
                entity_explosion_create(self->pos, 25, 20, true);
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