#include "main.h"
#include "spacetime.h"

typedef struct {
    vec2_t rest;
    vec2_t pos;
    vec2_t vel;
} spacetime_point_t;

static spacetime_point_t points[SPACETIME_COLS * SPACETIME_ROWS];
static bool initted = false;

void spacetime_init(void) {
    float spacing_x = PLAY_W / (float)(SPACETIME_COLS - 1);
    float spacing_y = PLAY_H / (float)(SPACETIME_ROWS - 1);
    float start_x = PLAY_LEFT;
    float start_y = PLAY_TOP;

    for (int row = 0; row < SPACETIME_ROWS; row++) {
        for (int col = 0; col < SPACETIME_COLS; col++) {
            int i = row * SPACETIME_COLS + col;
            points[i].rest = vec2(
                start_x + col * spacing_x,
                start_y + row * spacing_y
            );
            points[i].pos = points[i].rest;
            points[i].vel = vec2(0, 0);
        }
    }
    initted = true;
}

void spacetime_update(void) {
    if (!initted) return;
    float tick = (float)engine.tick;
    float damping = powf(SPACETIME_DAMPING, tick * 60.0f);

    for (int row = 0; row < SPACETIME_ROWS; row++) {
        for (int col = 0; col < SPACETIME_COLS; col++) {
            bool is_edge = row == 0 || row == SPACETIME_ROWS - 1 || col == 0 || col == SPACETIME_COLS - 1;
            if (is_edge) continue;

            int i = row * SPACETIME_COLS + col;
            spacetime_point_t *p = &points[i];

            vec2_t diff = vec2_sub(p->rest, p->pos);
            p->pos = vec2_add(p->pos, vec2_mulf(diff, SPACETIME_STIFFNESS * tick));

            p->pos = vec2_add(p->pos, vec2_mulf(p->vel, tick));
            p->vel = vec2_mulf(p->vel, damping);
        }
    }

    entity_type_t bh_types[] = {ENTITY_BLACKHOLE};
    entity_list_t bhs = entities_by_types(bh_types, 1);
    if (bhs.len > 0) {
        for (int row = 0; row < SPACETIME_ROWS; row++) {
            for (int col = 0; col < SPACETIME_COLS; col++) {
                bool is_edge = row == 0 || row == SPACETIME_ROWS - 1 || col == 0 || col == SPACETIME_COLS - 1;
                if (is_edge) continue;

                int i = row * SPACETIME_COLS + col;
                spacetime_point_t *p = &points[i];
                for (int j = 0; j < bhs.len; j++) {
                    vec2_t d = vec2_sub(bhs.entities[j]->pos, p->pos);
                    float dist = vec2_len(d);
                    if (dist < SPACETIME_PULL_RADIUS && dist > 0) {
                        d = vec2_normalize(d);
                        float pull = SPACETIME_PULL_FORCE * (1.0f - dist / SPACETIME_PULL_RADIUS);
                        p->vel = vec2_add(p->vel, vec2_mulf(d, pull * tick));
                    }
                }
            }
        }
    }
}

void spacetime_draw(void) {
    if (!initted) return;

    for (int row = 0; row < SPACETIME_ROWS; row++) {
        for (int col = 0; col < SPACETIME_COLS; col++) {
            int i = row * SPACETIME_COLS + col;

            if (col < SPACETIME_COLS - 1 && row > 0 && row < SPACETIME_ROWS - 1) {
                int ri = row * SPACETIME_COLS + (col + 1);
                render_draw_line(points[i].pos.x, points[i].pos.y, points[ri].pos.x, points[ri].pos.y, 20, 20, 45);
            }
            if (row < SPACETIME_ROWS - 1 && col > 0 && col < SPACETIME_COLS - 1) {
                int bi = (row + 1) * SPACETIME_COLS + col;
                render_draw_line(points[i].pos.x, points[i].pos.y, points[bi].pos.x, points[bi].pos.y, 20, 20, 45);
            }
        }
    }

    for (int i = 0; i < SPACETIME_COLS * SPACETIME_ROWS; i++) {
        int row = i / SPACETIME_COLS;
        int col = i % SPACETIME_COLS;
        bool is_edge = row == 0 || row == SPACETIME_ROWS - 1 || col == 0 || col == SPACETIME_COLS - 1;
        if (is_edge) continue;

        spacetime_point_t *p = &points[i];
        vec2_t offset = vec2_abs(vec2_sub(p->pos, p->rest));
        float dist = offset.x + offset.y;
        uint8_t intensity = (uint8_t)min(100.0f + dist * 1.5f, 180.0f);
        bool is_big = (row % 8 == 0) && (col % 8 == 0);
        bool is_edge_line = (row % 8 == 0) || (col % 8 == 0);
        if (is_big) {
            render_draw_rect(p->pos.x - 1.0f, p->pos.y - 1.0f, 2.0f, 2.0f, intensity / 3, intensity / 3, intensity);
        } else if (is_edge_line) {
            render_draw_rect(p->pos.x - 1.0f, p->pos.y - 1.0f, 2.0f, 2.0f, intensity / 4, intensity / 4, intensity / 1.5f);
        } else {
            render_draw_rect(p->pos.x - 1.0f, p->pos.y - 1.0f, 2.0f, 2.0f, intensity / 5, intensity / 5, intensity / 2);
        }
    }
}

void spacetime_explosion_push(vec2_t pos, float radius, float force) {
    if (!initted) return;
    for (int i = 0; i < SPACETIME_COLS * SPACETIME_ROWS; i++) {
        spacetime_point_t *p = &points[i];
        vec2_t dir = vec2_sub(p->pos, pos);
        float dist = vec2_len(dir);
        if (dist < radius && dist > 0) {
            dir = vec2_normalize(dir);
            float falloff = 1.0f - dist / radius;
            p->vel = vec2_add(p->vel, vec2_mulf(dir, force * falloff));
        }
    }
}
