#include "../main.h"
#include "../spacetime.h"

#define MAX_EXPLOSION_PARTICLES 256
#define EXPLOSION_PUSH_FORCE 200.0f
#define EXPLOSION_WAVE_SPEED 400.0f

typedef struct {
    vec2_t pos;
    vec2_t vel;
    float lifetime;
    float max_lifetime;
    uint8_t r, g, b;
} particle_t;

typedef struct {
    entity_t entity;
    particle_t particles[MAX_EXPLOSION_PARTICLES];
    int count;
    vec2_t origin;
    float max_radius;
    float current_radius;
    float max_damage;
    bool from_mine;
} explosion_t;

static void update(entity_t *self) {
    explosion_t *ex = (explosion_t *)self;
    bool any_alive = false;

    float prev_radius = ex->current_radius;
    ex->current_radius += EXPLOSION_WAVE_SPEED * engine.tick;
    if (ex->current_radius > ex->max_radius)
        ex->current_radius = ex->max_radius;

    entity_type_t target_types[5];
    int target_count = 4;
    target_types[0] = ENTITY_ENEMY_SQUARE;
    target_types[1] = ENTITY_ENEMY_CIRCLE;
    target_types[2] = ENTITY_ENEMY_TRIANGLE;
    target_types[3] = ENTITY_ENEMY_HEXAGON;
    if (ex->from_mine) {
        target_types[4] = ENTITY_MINE;
        target_count = 5;
    }
    entity_list_t targets = entities_by_types(target_types, target_count);
    for (int i = 0; i < targets.len; i++) {
        entity_t *t = targets.entities[i];
        if (t->invuln_timer > 0) continue;
        vec2_t dir = vec2_sub(t->pos, ex->origin);
        float dist = vec2_len(dir);
        if (dist >= prev_radius && dist < ex->current_radius && dist > 0) {
            float falloff = 1.0f - dist / ex->max_radius;
            t->health -= ex->max_damage * falloff;
            if (t->health <= 0) {
                entity_kill(t);
            }
        }
    }

    for (int i = 0; i < ex->count; i++) {
        if (ex->particles[i].lifetime <= 0) continue;
        ex->particles[i].pos = vec2_add(ex->particles[i].pos, vec2_mulf(ex->particles[i].vel, engine.tick));
        ex->particles[i].vel = vec2_mulf(ex->particles[i].vel, 0.98f);
        ex->particles[i].lifetime -= engine.tick;
        if (ex->particles[i].lifetime > 0) any_alive = true;
    }

    if (!any_alive && ex->current_radius >= ex->max_radius)
        entity_remove(self);
}

static void draw(entity_t *self) {
    explosion_t *ex = (explosion_t *)self;

    if (ex->from_mine) {
        float ring_r = ex->current_radius;
        if (ring_r > 4 && ring_r < ex->max_radius) {
            int segs = 36;
            for (int ring = 0; ring < 3; ring++) {
                float rr = ring_r - ring * 15.0f;
                if (rr < 4) break;
                float rt = rr / ex->max_radius;
                int brightness = (int)(220 * (1.0f - rt));
                if (brightness < 60) brightness = 60;
                uint8_t cr = brightness * 2 / (ring + 2);
                uint8_t cg = brightness;
                uint8_t cb = brightness / 2;
                for (int j = 0; j < segs; j++) {
                    float a1 = j * (2.0f * (float)M_PI / segs);
                    float a2 = (j + 1) * (2.0f * (float)M_PI / segs);
                    float x1 = ex->origin.x + cosf(a1) * rr;
                    float y1 = ex->origin.y + sinf(a1) * rr;
                    float x2 = ex->origin.x + cosf(a2) * rr;
                    float y2 = ex->origin.y + sinf(a2) * rr;
                    render_draw_line(x1, y1, x2, y2, cr, cg, cb);
                }
            }
        }
    }

    for (int i = 0; i < ex->count; i++) {
        if (ex->particles[i].lifetime <= 0) continue;
        float t = ex->particles[i].lifetime / ex->particles[i].max_lifetime;
        float radius = t * 1.0f + 0.3f;
        render_draw_circle(
            ex->particles[i].pos.x,
            ex->particles[i].pos.y,
            radius,
            ex->particles[i].r,
            ex->particles[i].g,
            ex->particles[i].b
        );
    }
}

entity_t *entity_explosion_create(vec2_t pos, float size, float damage, bool from_mine) {
    explosion_t *ex = entity_alloc(sizeof(explosion_t));
    ex->entity.type = ENTITY_EXPLOSION;
    ex->entity.pos = pos;
    ex->entity.update = update;
    ex->entity.draw = draw;

    ex->origin = pos;
    ex->max_radius = size * 25.0f;
    ex->entity.size = vec2(ex->max_radius, ex->max_radius);
    ex->current_radius = 0;
    ex->max_damage = damage;
    ex->from_mine = from_mine;

    g.shake_timer = 0.15f;
    g.shake_intensity = from_mine ? 7.0f : 1.5f;

    entity_type_t push_types[] = {ENTITY_ENEMY_SQUARE, ENTITY_ENEMY_CIRCLE, ENTITY_ENEMY_TRIANGLE, ENTITY_ENEMY_HEXAGON, ENTITY_GAMBO};
    entity_list_t targets = entities_by_types(push_types, 5);
    for (int i = 0; i < targets.len; i++) {
        vec2_t dir = vec2_sub(targets.entities[i]->pos, pos);
        float dist = vec2_len(dir);
        if (dist < ex->max_radius && dist > 0) {
            dir = vec2_normalize(dir);
            float falloff = 1.0f - dist / ex->max_radius;
            targets.entities[i]->vel = vec2_add(
                targets.entities[i]->vel,
                vec2_mulf(dir, EXPLOSION_PUSH_FORCE * falloff)
            );
        }
    }

    spacetime_explosion_push(pos, ex->max_radius, EXPLOSION_PUSH_FORCE * 1.5f);

    uint8_t colors[][3] = {
        {255, 255, 100},
        {255, 200, 50},
        {255, 150, 0},
        {255, 100, 0}
    };

    int num = (int)size * 8;
    if (num > MAX_EXPLOSION_PARTICLES) num = MAX_EXPLOSION_PARTICLES;
    ex->count = num;

    for (int i = 0; i < num; i++) {
        float angle = rand_float(0, M_PI * 2);
        float speed = rand_float(80, 200) * size;
        ex->particles[i].pos = vec2(
            pos.x + rand_float(-4, 4),
            pos.y + rand_float(-4, 4)
        );
        ex->particles[i].vel = vec2(cosf(angle) * speed, sinf(angle) * speed);
        ex->particles[i].lifetime = rand_float(0.4f, 0.9f);
        ex->particles[i].max_lifetime = ex->particles[i].lifetime;
        int ci = rand_int(0, 3);
        ex->particles[i].r = colors[ci][0];
        ex->particles[i].g = colors[ci][1];
        ex->particles[i].b = colors[ci][2];
    }

    return (entity_t *)ex;
}
