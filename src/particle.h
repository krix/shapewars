#ifndef PARTICLE_H
#define PARTICLE_H

#include "main.h"

#define MAX_PARTICLES 2048

typedef struct {
    vec2_t pos;
    vec2_t vel;
    float lifetime;
    float max_lifetime;
    uint8_t r, g, b;
    bool active;
} particle_t;

void particles_init(void);
void particles_update(void);
void particles_draw(void);
void particles_clear(void);

void particle_spawn(vec2_t pos, vec2_t vel, float lifetime, uint8_t r, uint8_t g, uint8_t b);
void particle_burst(vec2_t pos, int count, float speed, float lifetime, uint8_t r, uint8_t g, uint8_t b);
void particle_explosion(vec2_t pos, int count, float speed_min, float speed_max, float lifetime_min, float lifetime_max, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2);

#endif
