#include "../main.h"
#include "../spacetime.h"

#define BH_RADIUS 15.0f
#define BH_HEALTH 30.0f
#define BH_GRAVITY_RADIUS 200.0f
#define BH_GRAVITY_FORCE 500.0f
#define BH_PARTICLES_MAX 128
#define BH_SPAWN_INTERVAL 0.2f

typedef struct {
    vec2_t pos;
    vec2_t vel;
    float lifetime;
} bh_particle_t;

typedef struct {
    entity_t entity;
    bh_particle_t particles[BH_PARTICLES_MAX];
    int particle_count;
    float spawn_timer;
} blackhole_t;

static void update(entity_t *self) {
    blackhole_t *bh = (blackhole_t *)self;
    float tick = (float)engine.tick;

    self->angle += tick * 3.0f;

    bh->spawn_timer -= tick;
    if (bh->spawn_timer <= 0 && bh->particle_count < BH_PARTICLES_MAX) {
        float a = rand_float(0, M_PI * 2);
        float spawn_r = BH_RADIUS * 4.0f;
        vec2_t pos = vec2(
            self->pos.x + cosf(a) * spawn_r,
            self->pos.y + sinf(a) * spawn_r
        );
        vec2_t dir = vec2_normalize(vec2_sub(self->pos, pos));

        bh_particle_t *p = &bh->particles[bh->particle_count++];
        p->pos = pos;
        p->vel = vec2_mulf(dir, rand_float(40, 80));
        p->lifetime = 2.0f;
        bh->spawn_timer = BH_SPAWN_INTERVAL + rand_float(-0.05f, 0.05f);
    }

    for (int i = 0; i < bh->particle_count; ) {
        bh_particle_t *p = &bh->particles[i];
        vec2_t dir = vec2_sub(self->pos, p->pos);
        float dist = vec2_len(dir);
        if (dist > 0) {
            dir = vec2_normalize(dir);
            float pull = 160.0f * (1.0f - dist / BH_GRAVITY_RADIUS);
            p->vel = vec2_add(p->vel, vec2_mulf(dir, pull * tick));
        }
        p->vel = vec2_mulf(p->vel, 0.99f);
        p->pos = vec2_add(p->pos, vec2_mulf(p->vel, tick));
        p->lifetime -= tick;

        if (dist < 4.0f || p->lifetime <= 0) {
            bh->particles[i] = bh->particles[--bh->particle_count];
        } else {
            i++;
        }
    }

    // Gravity pull on all nearby entities
    entity_type_t grav_types[] = {ENTITY_PLAYER};
    entity_list_t targets = entities_by_types(grav_types, 1);
    for (int i = 0; i < targets.len; i++) {
        entity_t *e = targets.entities[i];
        vec2_t d = vec2_sub(self->pos, e->pos);
        float dist = vec2_len(d);
        if (dist < BH_GRAVITY_RADIUS && dist > 4.0f) {
            if (e->type == ENTITY_PLAYER && e->invuln_timer > 0) continue;
            d = vec2_normalize(d);
            float t = dist / BH_GRAVITY_RADIUS;
            float pull = BH_GRAVITY_FORCE * (1.0f - t * t);
            if (e->type == ENTITY_PLAYER) {
                e->pos = vec2_add(e->pos, vec2_mulf(d, pull * 0.55f * engine.tick));
            } else {
                e->vel = vec2_add(e->vel, vec2_mulf(d, pull * engine.tick));
            }
        }
    }
}

static void draw(entity_t *self) {
    float halo_radius = BH_RADIUS * 3.0f;
    float hs = halo_radius * 2;
    render_flush();
    render_set_texture_filter(RENDER_FILTER_LINEAR);
    render_push();
    render_translate(self->pos);
    render_draw(vec2(-halo_radius, -halo_radius), vec2(hs, hs), g.light_tex, vec2(0, 0), vec2(256, 256), rgba(0, 0, 0, 255));
    render_pop();
    render_flush();
    render_set_texture_filter(RENDER_FILTER_NEAREST);

    float a_off = self->angle;
    for (int arm = 0; arm < 3; arm++) {
        float arm_angle = a_off + arm * (2.0f * (float)M_PI / 3.0f);
        float prev_x = self->pos.x + cosf(arm_angle) * BH_RADIUS * 0.15f;
        float prev_y = self->pos.y + sinf(arm_angle) * BH_RADIUS * 0.15f;
        for (int seg = 2; seg <= 20; seg++) {
            float t = seg / 20.0f;
            float r = t * BH_RADIUS;
            float a = arm_angle + t * 2.0f * (float)M_PI;
            float x = self->pos.x + cosf(a) * r;
            float y = self->pos.y + sinf(a) * r;
            uint8_t b = (uint8_t)(200 * t + 55);
            render_draw_line(prev_x, prev_y, x, y, b / 2, b / 5, b);
            prev_x = x;
            prev_y = y;
        }
    }

    render_draw_circle(self->pos.x, self->pos.y, BH_RADIUS * 0.35f, 0, 0, 0);

    for (int i = 0; i < ((blackhole_t *)self)->particle_count; i++) {
        bh_particle_t *p = &((blackhole_t *)self)->particles[i];
        vec2_t dir = vec2_sub(self->pos, p->pos);
        float dist = vec2_len(dir);
        float t = dist / (BH_RADIUS * 4.0f);
        if (t > 1.0f) t = 1.0f;
        uint8_t b = (uint8_t)(200 * t + 55);
        render_draw_circle(p->pos.x, p->pos.y, 2, b, b, b);
    }
}

static void kill(entity_t *self) {
    entity_explosion_create(self->pos, 15, 10, true);
}

entity_t *entity_blackhole_create(vec2_t pos) {
    blackhole_t *bh = entity_alloc(sizeof(blackhole_t));
    bh->entity.type = ENTITY_BLACKHOLE;
    bh->entity.pos = pos;
    bh->entity.size = vec2(BH_RADIUS * 2, BH_RADIUS * 2);
    bh->entity.health = BH_HEALTH;
    bh->entity.update = update;
    bh->entity.draw = draw;
    bh->entity.kill = kill;
    bh->particle_count = 0;
    bh->spawn_timer = 0;
    return (entity_t *)bh;
}
