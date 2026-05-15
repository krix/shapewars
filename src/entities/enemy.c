#include "../main.h"
#include "../sounds.h"

typedef struct {
    entity_t entity;
    float speed;
    float timer;
    float phase;
    int generation;
    vec2_t move_dir;
    float rot_speed;
    float max_health;
} enemy_t;

static entity_t *closest_player(vec2_t from) {
    entity_type_t types[] = {ENTITY_PLAYER, ENTITY_PLAYER2};
    entity_list_t players = entities_by_types(types, 2);
    if (players.len == 0) return NULL;
    entity_t *best = players.entities[0];
    float best_d = vec2_dist(best->pos, from);
    for (int i = 1; i < players.len; i++) {
        float d = vec2_dist(players.entities[i]->pos, from);
        if (d < best_d) { best = players.entities[i]; best_d = d; }
    }
    return best;
}

static void update(entity_t *self) {
    enemy_t *enemy = (enemy_t *)self;
    
    self->invuln_timer -= engine.tick;
    if (self->invuln_timer < 0) self->invuln_timer = 0;

    entity_t *player = closest_player(self->pos);
    if (!player) return;
    
    // Apply knockback velocity with friction
    if (vec2_len(self->vel) > 1.0f) {
        self->pos = vec2_add(self->pos, vec2_mulf(self->vel, engine.tick));
        self->vel = vec2_mulf(self->vel, 0.85f);
    } else {
        self->vel = vec2(0, 0);
    }

    vec2_t to_player = vec2_sub(player->pos, self->pos);
    float dist = vec2_len(to_player);
    vec2_t dir = dist > 0 ? vec2_normalize(to_player) : vec2(0, 0);

    switch (self->type) {
        case ENTITY_ENEMY_TRIANGLE: {
            // Move toward player along a sine curve
            enemy->timer += engine.tick;
            float sine_amp = 12.0f;
            float sine_freq = 3.0f;
            vec2_t perp = vec2(-dir.y, dir.x);
            float prev_off = sinf((enemy->timer - engine.tick) * sine_freq) * sine_amp;
            float cur_off = sinf(enemy->timer * sine_freq) * sine_amp;
            self->pos = vec2_add(self->pos, vec2_mulf(dir, enemy->speed * engine.tick));
            self->pos = vec2_add(self->pos, vec2_mulf(perp, cur_off - prev_off));
            break;
        }
        case ENTITY_ENEMY_SQUARE: {
            // Orbit in a circle while the orbit center moves in the stored direction
            enemy->timer += engine.tick;
            float orbit_radius = 1.0f;
            float orbit_speed = 2.0f;
            vec2_t center = vec2_add(self->pos, vec2_mulf(enemy->move_dir, enemy->speed * engine.tick));
            float ox = cosf(enemy->timer * orbit_speed) * orbit_radius;
            float oy = sinf(enemy->timer * orbit_speed) * orbit_radius;
            self->pos = vec2_add(center, vec2(ox, oy));
            break;
        }
        case ENTITY_ENEMY_HEXAGON: {
            // Predict where the player is heading and move toward that point
            vec2_t target = player->pos;
            if (vec2_len(player->vel) > 1.0f) {
                target = vec2_add(target, vec2_mulf(player->vel, 0.5f));
            }
            vec2_t pred_dir = vec2_sub(target, self->pos);
            if (vec2_len(pred_dir) > 0) pred_dir = vec2_normalize(pred_dir);
            self->pos = vec2_add(self->pos, vec2_mulf(pred_dir, enemy->speed * engine.tick));
            // Grow in size as health decreases
            float t = 1.0f - self->health / enemy->max_health;
            float radius = 5.0f + 9.0f * t;
            self->size = vec2(radius * 2, radius * 2);
            break;
        }
        case ENTITY_ENEMY_CIRCLE: {
            // Move toward player, but periodically stop and pause
            float move_time = 1.5f;
            float stop_time = 0.6f;
            enemy->timer -= engine.tick;
            if (enemy->timer <= 0) {
                enemy->phase = !enemy->phase;
                enemy->timer = enemy->phase ? move_time : stop_time;
            }
            if (enemy->phase) {
                self->pos = vec2_add(self->pos, vec2_mulf(dir, enemy->speed * engine.tick));
            }
            break;
        }
        default:
            self->pos = vec2_add(self->pos, vec2_mulf(dir, enemy->speed * engine.tick));
            break;
    }
    
    self->angle += enemy->rot_speed * engine.tick;
}

static void draw(entity_t *self) {
    uint8_t flash_r = 0, flash_g = 0, flash_b = 0;
    if (self->invuln_timer > 0) {
        float t = self->invuln_timer / 0.12f;
        uint8_t a = (uint8_t)(t * 180);
        flash_r = a; flash_g = a; flash_b = a;
    }

    render_push_matrix();
    render_translate(self->pos);
    render_rotate(self->angle);
    
    switch (self->type) {
        case ENTITY_ENEMY_SQUARE: {
            enemy_t *enemy = (enemy_t *)self;
            float h = 10.0f / (enemy->generation + 1);
            float w = 10.0f / (enemy->generation + 1);
            uint8_t r = (uint8_t)min(255, 255 + flash_r);
            uint8_t g = (uint8_t)min(255, 0 + flash_g);
            uint8_t b = (uint8_t)min(255, 0 + flash_b);
            render_draw_line(-w, -h, w, -h, r, g, b);
            render_draw_line(w, -h, w, h, r, g, b);
            render_draw_line(w, h, -w, h, r, g, b);
            render_draw_line(-w, h, -w, -h, r, g, b);
            break;
        }
        case ENTITY_ENEMY_CIRCLE: {
            int segments = 16;
            float angle_step = 2.0f * M_PI / segments;
            uint8_t r = (uint8_t)min(255, 0 + flash_r);
            uint8_t g = (uint8_t)min(255, 255 + flash_g);
            uint8_t b = (uint8_t)min(255, 0 + flash_b);
            for (int j = 0; j < segments; j++) {
                float a1 = j * angle_step;
                float a2 = (j + 1) * angle_step;
                float x1 = cosf(a1) * 10;
                float y1 = sinf(a1) * 10;
                float x2 = cosf(a2) * 10;
                float y2 = sinf(a2) * 10;
                render_draw_line(x1, y1, x2, y2, r, g, b);
            }
            break;
        }
        case ENTITY_ENEMY_TRIANGLE: {
            uint8_t r = (uint8_t)min(255, 0 + flash_r);
            uint8_t g = (uint8_t)min(255, 0 + flash_g);
            uint8_t b = (uint8_t)min(255, 255 + flash_b);
            render_draw_triangle(0, -6, -6, 6, 6, 6, r, g, b);
            break;
        }
        case ENTITY_ENEMY_HEXAGON: {
            float radius = fmaxf(self->size.x, self->size.y) * 0.5f;
            float angle_step = 2.0f * M_PI / 6.0f;
            uint8_t r = (uint8_t)min(255, 255 + flash_r);
            uint8_t g = (uint8_t)min(255, 255 + flash_g);
            uint8_t b = (uint8_t)min(255, 0 + flash_b);
            for (int j = 0; j < 6; j++) {
                float angle1 = j * angle_step;
                float angle2 = (j + 1) * angle_step;
                float x1 = cosf(angle1) * radius;
                float y1 = sinf(angle1) * radius;
                float x2 = cosf(angle2) * radius;
                float y2 = sinf(angle2) * radius;
                render_draw_line(x1, y1, x2, y2, r, g, b);
            }
            break;
        }
        default:
            break;
    }
    
    render_pop_matrix();
}

static int health_for_type(entity_type_t type) {
    switch (type) {
        case ENTITY_ENEMY_TRIANGLE: return 1;
        case ENTITY_ENEMY_SQUARE:   return 2;
        case ENTITY_ENEMY_HEXAGON:  return 3;
        case ENTITY_ENEMY_CIRCLE:   return 4;
        default: return 1;
    }
}

static void kill(entity_t *self) {
    enemy_t *enemy = (enemy_t *)self;
    int hp = health_for_type(self->type);
    entity_explosion_create(self->pos, hp + 3, hp * 0.2f + 0.5f, false, false);
    if (!g.game_over) {
        if (self->type != ENTITY_ENEMY_SQUARE || enemy->generation >= 2) {
            entity_gambo_create(self->pos);
        }
        if (self->type == ENTITY_ENEMY_CIRCLE) {
            entity_mine_create(self->pos);
        }
        if (self->type == ENTITY_ENEMY_SQUARE && enemy->generation < 2) {
            int ng = enemy->generation + 1;
            float offsets[] = {-12.0f / ng, 12.0f / ng};
            for (int x = 0; x < 2; x++) {
                for (int y = 0; y < 2; y++) {
                    vec2_t spos = vec2(self->pos.x + offsets[x], self->pos.y + offsets[y]);
                    enemy_t *child = (enemy_t *)entity_enemy_create(ENTITY_ENEMY_SQUARE, spos);
                    child->generation = ng;
                    child->entity.size = vec2(20.0f / (ng + 1), 20.0f / (ng + 1));
                    child->entity.health = (ng >= 2) ? 2.0f : 5.0f;
                    vec2_t push_dir = vec2_normalize(vec2_sub(spos, self->pos));
                    child->entity.vel = vec2_mulf(push_dir, 150.0f);
                }
            }
            g.score += 50 * hp;
        }
        g.score += 100 * hp;
        g.total_kills++;
    }
    entity_remove(self);
}

entity_t *entity_enemy_create(entity_type_t type, vec2_t pos) {
    enemy_t *e = entity_alloc(sizeof(enemy_t));
    e->entity.type = type;
    e->entity.pos = pos;
    e->entity.size = vec2(20, 20);
    e->entity.invuln_timer = 0.0f;
    e->entity.update = update;
    e->entity.draw = draw;
    e->entity.kill = kill;
    
    // Set speed and health based on enemy type
    switch (type) {
        case ENTITY_ENEMY_TRIANGLE: // 3 corners - fastest, weakest, sine wave
            e->speed = 50.0f * 1.5f;
            e->entity.health = 2;
            e->entity.size = vec2(12, 12);
            e->timer = 0.0f;
            e->rot_speed = rand_float(4, 7);
            break;
        case ENTITY_ENEMY_SQUARE: // 4 corners, orbits while moving in random direction
            e->speed = 50.0f * 0.5f;
            e->entity.health = 16;
        {
            float a = rand_float(0, M_PI * 2);
            e->move_dir = vec2(cosf(a), sinf(a));
        }
            e->timer = rand_float(0, M_PI * 2);
            e->rot_speed = rand_float(1, 3);
            break;
        case ENTITY_ENEMY_HEXAGON: // 6 corners, predicts player movement
            e->speed = 50.0f * 1.6f;
            e->entity.health = 6;
            e->max_health = 6;
            e->entity.size = vec2(10, 10);
            e->rot_speed = rand_float(2, 4);
            break;
        case ENTITY_ENEMY_CIRCLE: // 16 segments - toughest, stop-and-go
            e->speed = 50.0f * 1.2f;
            e->entity.health = 16;
            e->timer = 0.6f;
            e->phase = 0.0f;
            e->rot_speed = rand_float(2, 4);
            break;
        default:
            e->speed = 50.0f;
            e->entity.health = 1;
            e->rot_speed = 0;
    }
    
    e->generation = 0;
    
    return (entity_t *)e;
}