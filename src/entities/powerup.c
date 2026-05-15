#include "../main.h"
#include "../sounds.h"

#define POWERUP_SIZE 8.0f
#define POWERUP_LIFETIME 20.0f
#define POWERUP_BLINK_TIME 4.0f
#define POWERUP_COLLECT_RADIUS 20.0f
#define POWERUP_BOB_SPEED 3.0f

typedef struct {
    entity_t entity;
    int powerup_type;
    float bob_phase;
    float lifetime;
} powerup_t;

static void update(entity_t *self) {
    powerup_t *p = (powerup_t *)self;
    p->bob_phase += engine.tick * POWERUP_BOB_SPEED;
    p->lifetime -= engine.tick;

    if (p->lifetime <= 0) {
        entity_remove(self);
        return;
    }

    entity_type_t ptypes[] = {ENTITY_PLAYER, ENTITY_PLAYER2};
    entity_list_t players = entities_by_types(ptypes, 2);
    for (int i = 0; i < players.len; i++) {
        float dx = self->pos.x - players.entities[i]->pos.x;
        float dy = self->pos.y - players.entities[i]->pos.y;
        if (dx * dx + dy * dy < POWERUP_COLLECT_RADIUS * POWERUP_COLLECT_RADIUS) {
            sound_play(sound_collect);
            if (p->powerup_type == 0) {
                g.shield_timer = 5.0f;
            } else {
                g.magnet_timer = 5.0f;
            }
            entity_remove(self);
            return;
        }
    }
}

static void draw(entity_t *self) {
    powerup_t *p = (powerup_t *)self;
    if (p->lifetime <= POWERUP_BLINK_TIME && fmodf(p->lifetime * 5, 1) > 0.5f) return;

    float bob = sinf(p->bob_phase) * 3.0f;
    float px = self->pos.x;
    float py = self->pos.y + bob;

    if (p->powerup_type == 0) {
        uint8_t c = (uint8_t)(180 + 75 * sinf(p->bob_phase * 1.5f));
        render_draw_circle(px, py, POWERUP_SIZE, 30, 30, c);
    } else {
        uint8_t r = (uint8_t)(200 + 55 * sinf(p->bob_phase * 2.0f));
        uint8_t g = (uint8_t)(120 + 40 * sinf(p->bob_phase * 2.0f + 1.0f));
        render_draw_circle(px, py, POWERUP_SIZE, r, g, 40);
        render_draw_line(px - 5, py + 3, px - 2, py - 2, r, g, 40);
        render_draw_line(px - 2, py - 2, px + 1, py + 3, r, g, 40);
        render_draw_line(px + 1, py + 3, px + 5, py - 3, r, g, 40);
    }
}

entity_t *entity_powerup_create(vec2_t pos, int powerup_type) {
    powerup_t *p = entity_alloc(sizeof(powerup_t));
    p->entity.type = (powerup_type == 0) ? ENTITY_POWERUP_SHIELD : ENTITY_POWERUP_MAGNET;
    p->entity.pos = pos;
    p->entity.size = vec2(POWERUP_SIZE * 2, POWERUP_SIZE * 2);
    p->entity.update = update;
    p->entity.draw = draw;
    p->powerup_type = powerup_type;
    p->bob_phase = rand_float(0, M_PI * 2);
    p->lifetime = POWERUP_LIFETIME;
    return (entity_t *)p;
}
