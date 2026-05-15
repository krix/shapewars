#include "particle.h"

static particle_t particles[MAX_PARTICLES];

void particles_init(void) {
    for (int i = 0; i < MAX_PARTICLES; i++)
        particles[i].active = false;
}

void particles_update(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        particles[i].pos = vec2_add(particles[i].pos, vec2_mulf(particles[i].vel, engine.tick));
        particles[i].vel = vec2_mulf(particles[i].vel, 0.96f);
        particles[i].lifetime -= engine.tick;
        if (particles[i].lifetime <= 0)
            particles[i].active = false;
    }
}

void particles_draw(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        float t = particles[i].lifetime / particles[i].max_lifetime;
        float radius = t * 1.5f + 0.5f;
        render_draw_circle(
            particles[i].pos.x,
            particles[i].pos.y,
            radius,
            particles[i].r,
            particles[i].g,
            particles[i].b
        );
    }
}

void particles_clear(void) {
    for (int i = 0; i < MAX_PARTICLES; i++)
        particles[i].active = false;
}

static int find_free_particle(void) {
    for (int i = 0; i < MAX_PARTICLES; i++)
        if (!particles[i].active) return i;
    return -1;
}

void particle_spawn(vec2_t pos, vec2_t vel, float lifetime, uint8_t r, uint8_t g, uint8_t b) {
    int idx = find_free_particle();
    if (idx < 0) return;
    particles[idx].pos = pos;
    particles[idx].vel = vel;
    particles[idx].lifetime = lifetime;
    particles[idx].max_lifetime = lifetime;
    particles[idx].r = r;
    particles[idx].g = g;
    particles[idx].b = b;
    particles[idx].active = true;
}

void particle_burst(vec2_t pos, int count, float speed, float lifetime, uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < count; i++) {
        float angle = rand_float(0, M_PI * 2);
        float v = rand_float(speed * 0.5f, speed);
        vec2_t vel = vec2(cosf(angle) * v, sinf(angle) * v);
        particle_spawn(pos, vel, lifetime, r, g, b);
    }
}

void particle_explosion(vec2_t pos, int count, float speed_min, float speed_max, float lifetime_min, float lifetime_max, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) {
    for (int i = 0; i < count; i++) {
        float angle = rand_float(0, M_PI * 2);
        float speed = rand_float(speed_min, speed_max);
        float lifetime = rand_float(lifetime_min, lifetime_max);
        vec2_t vel = vec2(cosf(angle) * speed, sinf(angle) * speed);
        float mix = rand_float(0, 1);
        uint8_t r = (uint8_t)(r1 * (1 - mix) + r2 * mix);
        uint8_t g = (uint8_t)(g1 * (1 - mix) + g2 * mix);
        uint8_t b = (uint8_t)(b1 * (1 - mix) + b2 * mix);
        particle_spawn(vec2(pos.x + rand_float(-2, 2), pos.y + rand_float(-2, 2)), vel, lifetime, r, g, b);
    }
}
