#include "../main.h"

#define BULLET_SPEED 2000.0f
#define BULLET_LIFETIME 2.0f

typedef struct {
    entity_t entity;
    float lifetime;
    uint8_t r, g, b;
} bullet_t;

static void update(entity_t *self) {
    bullet_t *bullet = (bullet_t *)self;
    
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
        float dx = self->pos.x - enemy->pos.x;
        float dy = self->pos.y - enemy->pos.y;
        float distance = sqrtf(dx * dx + dy * dy);
        
        if (distance < 20.0f) {
            vec2_t push_dir = vec2_sub(enemy->pos, self->pos);
            if (vec2_len(push_dir) > 0) push_dir = vec2_normalize(push_dir);
            if (enemy->type != ENTITY_BLACKHOLE) {
                enemy->vel = vec2_add(enemy->vel, vec2_mulf(push_dir, 800.0f / fmaxf(enemy->health, 1.0f)));
            }
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
