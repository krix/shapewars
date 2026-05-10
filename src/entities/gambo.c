#include "../main.h"

#define GAMBO_ATTRACT_RANGE 200.0f
#define GAMBO_ATTRACT_MAX_SPEED 300.0f
#define GAMBO_LIFETIME 25.0f
#define GAMBO_BLINK_TIME 5.0f

typedef struct {
    entity_t entity;
    float lifetime;
    float angle;
} gambo_t;

static void update(entity_t *self) {
    gambo_t *gambo = (gambo_t *)self;
    
    if (vec2_len(self->vel) > 1.0f) {
        self->pos = vec2_add(self->pos, vec2_mulf(self->vel, engine.tick));
        self->vel = vec2_mulf(self->vel, 0.85f);
    } else {
        self->vel = vec2(0, 0);
    }

    entity_t *player = entity_first(ENTITY_PLAYER);
    if (player) {
        vec2_t dir = vec2_sub(player->pos, self->pos);
        float distance = vec2_len(dir);
        
        if (distance < GAMBO_ATTRACT_RANGE && distance > 0) {
            dir = vec2_normalize(dir);
            
            float t = 1.0f - (distance / GAMBO_ATTRACT_RANGE);
            float speed = t * t * GAMBO_ATTRACT_MAX_SPEED;
            vec2_t velocity = vec2_mulf(dir, speed);
            
            self->pos = vec2_add(self->pos, vec2_mulf(velocity, engine.tick));
            
            if (distance < 20.0f) {
                g.score += 10;
                g.gambo_count++;
                if (g.gambo_count >= 100) {
                    g.gambo_count = 0;
                    g.spreadshot_timer = 5.0f;
                }
                entity_remove(self);
                return;
            }
        }
    }
    
    gambo->lifetime -= engine.tick;
    if (gambo->lifetime <= 0) {
        entity_remove(self);
    }
}

static void draw(entity_t *self) {
    gambo_t *gambo = (gambo_t *)self;
    if (gambo->lifetime <= GAMBO_BLINK_TIME && fmodf(gambo->lifetime * 5, 1) > 0.5f) return;
    render_push();
    render_translate(self->pos);
    render_rotate(gambo->angle);
    float w = 5, h = 3;
    render_draw(vec2(-w/2, -h/2), vec2(w, h), g.circle_tex, vec2(0, 0), g.circle_tex_size, rgba(255, 255, 0, 255));
    render_pop();
}

entity_t *entity_gambo_create(vec2_t pos) {
    gambo_t *g = entity_alloc(sizeof(gambo_t));
    g->entity.type = ENTITY_GAMBO;
    g->entity.pos = pos;
    g->entity.size = vec2(8, 8);
    g->entity.update = update;
    g->entity.draw = draw;
    g->angle = rand_float(0, M_PI * 2);
    g->lifetime = GAMBO_LIFETIME;
    return (entity_t *)g;
}
