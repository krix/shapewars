#include "../main.h"

#define MINE_RADIUS 4.0f
#define MINE_TRIGGER_RADIUS 15.0f
#define MINE_ARM_TIME 2.0f
#define MINE_EXPLOSION_DAMAGE 2.0f

typedef struct {
    entity_t entity;
    float arm_timer;
    bool arming;
} mine_t;

static void update(entity_t *self) {
    mine_t *mine = (mine_t *)self;

    if (!mine->arming) {
        entity_type_t ptypes[] = {ENTITY_PLAYER, ENTITY_PLAYER2};
        entity_list_t players = entities_by_types(ptypes, 2);
        for (int i = 0; i < players.len; i++) {
            float dx = self->pos.x - players.entities[i]->pos.x;
            float dy = self->pos.y - players.entities[i]->pos.y;
            if (dx * dx + dy * dy < MINE_TRIGGER_RADIUS * MINE_TRIGGER_RADIUS) {
                mine->arming = true;
                mine->arm_timer = MINE_ARM_TIME;
                break;
            }
        }
    } else {
        mine->arm_timer -= engine.tick;
        if (mine->arm_timer <= 0) {
            entity_explosion_create(self->pos, 5, MINE_EXPLOSION_DAMAGE, true, false);
            entity_remove(self);
        }
    }
}

static void mine_kill(entity_t *self) {
    entity_explosion_create(self->pos, 5, MINE_EXPLOSION_DAMAGE, true, false);
}

static void draw(entity_t *self) {
    mine_t *mine = (mine_t *)self;
    if (mine->arming && fmodf(mine->arm_timer * 4, 1) > 0.5f) return;

    self->angle += engine.tick * 3.0f;
    float pulse = 0.5f + 0.5f * sinf(self->angle);

    render_draw_circle(self->pos.x, self->pos.y, MINE_RADIUS + 3.0f + pulse * 3.0f, 0, 80, 0);

    float s = MINE_RADIUS * 2;
    render_draw(vec2(self->pos.x - MINE_RADIUS, self->pos.y - MINE_RADIUS), vec2(s, s), g.circle_tex, vec2(0, 0), g.circle_tex_size, rgba(40, 255, 40, 160));
}

entity_t *entity_mine_create(vec2_t pos) {
    mine_t *m = entity_alloc(sizeof(mine_t));
    m->entity.type = ENTITY_MINE;
    m->entity.pos = pos;
    m->entity.size = vec2(MINE_RADIUS * 2, MINE_RADIUS * 2);
    m->entity.update = update;
    m->entity.draw = draw;
    m->entity.kill = mine_kill;
    m->entity.health = 0.3f;
    m->arming = false;
    m->arm_timer = 0;
    return (entity_t *)m;
}
