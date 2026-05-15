#ifndef SPACETIME_H
#define SPACETIME_H

#define SPACETIME_COLS 81
#define SPACETIME_ROWS 49
#define SPACETIME_STIFFNESS 8.0f
#define SPACETIME_DAMPING 0.92f
#define SPACETIME_PULL_RADIUS 600.0f
#define SPACETIME_PULL_FORCE 2000.0f

void spacetime_init(void);
void spacetime_update(void);
void spacetime_draw(void);
void spacetime_explosion_push(vec2_t pos, float radius, float force);

#endif
