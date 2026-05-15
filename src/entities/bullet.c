#include "../main.h"

#define BULLET_SPEED 2000.0f
#define BULLET_LIFETIME 2.0f

typedef struct {
    entity_t entity;
    float lifetime;
    vec2_t prev_pos;
    uint8_t r, g, b;
} bullet_t;

static float dist_to_segment(vec2_t p, vec2_t a, vec2_t b) {
    vec2_t ab = vec2_sub(b, a);
    vec2_t ap = vec2_sub(p, a);
    float len_sq = vec2_dot(ab, ab);
    if (len_sq < 0.0001f) return vec2_len(ap);
    float t = fmaxf(0, fminf(1, vec2_dot(ap, ab) / len_sq));
    vec2_t closest = vec2_add(a, vec2_mulf(ab, t));
    return vec2_len(vec2_sub(p, closest));
}

static void update(entity_t *self) {
    bullet_t *bullet = (bullet_t *)self;
    
    bullet->prev_pos = self->pos;
    self->pos = vec2_add(self->pos, vec2_mulf(self->vel, engine.tick));
    
    bullet->lifetime -= engine.tick;
    if (bullet->lifetime <= 0) {
        entity_remove(self);
        return;
    }
    
    entity_type_t types[] = {ENTITY_ENEMY_SQUARE, ENTITY_ENEMY_CIRCLE, ENTITY_ENEMY_TRIANGLE, ENTITY_ENEMY_HEXAGON, ENTITY_BLACKHOLE};
    entity_list_t enemies = entities_by_types(types, 5);
    for (int i = 0; i < enemies.len; i++) {
        entity_t *enemy = enemies.entities[i];
        if (enemy->invuln_timer > 0) continue;
        
        float hit_r = fmaxf(enemy->size.x, enemy->size.y) * 0.5f + 8.0f;
        float d = dist_to_segment(enemy->pos, bullet->prev_pos, self->pos);
        if (d < hit_r) {
            vec2_t ab = vec2_sub(self->pos, bullet->prev_pos);
            vec2_t ap = vec2_sub(enemy->pos, bullet->prev_pos);
            float len_sq = vec2_dot(ab, ab);
            vec2_t hit_pos = self->pos;
            if (len_sq >= 0.0001f) {
                float t = fmaxf(0, fminf(1, vec2_dot(ap, ab) / len_sq));
                hit_pos = vec2_add(bullet->prev_pos, vec2_mulf(ab, t));
            }
            vec2_t push_dir = vec2_sub(enemy->pos, hit_pos);
            if (vec2_len(push_dir) > 0) push_dir = vec2_normalize(push_dir);
            if (enemy->type != ENTITY_BLACKHOLE) {
                float kb = 800.0f / fmaxf(enemy->health, 1.0f);
                if (enemy->type == ENTITY_ENEMY_TRIANGLE || enemy->type == ENTITY_ENEMY_SQUARE) kb *= 0.3f;
                enemy->vel = vec2_add(enemy->vel, vec2_mulf(push_dir, kb));
                enemy->invuln_timer = 0.12f;
            }
            entity_bullet_hit_create(hit_pos);
            enemy->health--;
            if (enemy->health <= 0) {
                entity_kill(enemy);
            }
            entity_remove(self);
            return;
        }
    }
    
    float margin = 20.0f;
    if (self->pos.x < PLAY_LEFT - margin || self->pos.x > PLAY_RIGHT + margin || self->pos.y < PLAY_TOP - margin || self->pos.y > PLAY_BOTTOM + margin) {
        entity_remove(self);
    }
}

static void draw(entity_t *self) {
    bullet_t *bullet = (bullet_t *)self;
    render_draw_circle(self->pos.x, self->pos.y, 1, bullet->r, bullet->g, bullet->b);
}

entity_t *entity_bullet_create(vec2_t pos, vec2_t direction, float speed) {
    bullet_t *b = entity_alloc(sizeof(bullet_t));
    b->entity.type = ENTITY_BULLET;
    b->entity.pos = pos;
    b->prev_pos = pos;
    b->entity.vel = vec2_mulf(direction, speed);
    b->entity.size = vec2(3, 3);
    b->entity.update = update;
    b->entity.draw = draw;
    b->lifetime = BULLET_LIFETIME;
    b->r = rand_int(0, 40);
    b->g = rand_int(200, 255);
    b->b = rand_int(80, 160);
    return (entity_t *)b;
}
