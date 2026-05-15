#include "main.h"
#include <string.h>
#include <stdlib.h>

#define MAX_ENTITIES 4096
static entity_t *entity_pool[MAX_ENTITIES];
static int entity_pool_count = 0;

entity_t *entity_create(entity_type_t type) {
    entity_t *e = calloc(1, sizeof(entity_t));
    if (!e) return NULL;
    if (entity_pool_count >= MAX_ENTITIES) {
        free(e);
        return NULL;
    }
    e->is_alive = true;
    e->type = type;
    e->health = 1.0f;
    entity_pool[entity_pool_count++] = e;
    return e;
}

void *entity_alloc(size_t size) {
    entity_t *e = calloc(1, size);
    if (!e) return NULL;
    if (entity_pool_count >= MAX_ENTITIES) {
        free(e);
        return NULL;
    }
    e->is_alive = true;
    e->health = 1.0f;
    entity_pool[entity_pool_count++] = e;
    return e;
}

void entity_remove(entity_t *e) {
    if (!e || !e->is_alive) return;
    e->is_alive = false;
}

entity_t *entity_first(entity_type_t type) {
    for (int i = 0; i < entity_pool_count; i++) {
        if (entity_pool[i]->is_alive && entity_pool[i]->type == type) {
            return entity_pool[i];
        }
    }
    return NULL;
}

entity_list_t entities_by_types(entity_type_t *types, int count) {
    entity_list_t list = {0};
    for (int i = 0; i < entity_pool_count && list.len < MAX_ENTITIES; i++) {
        if (!entity_pool[i]->is_alive) continue;
        for (int t = 0; t < count; t++) {
            if (entity_pool[i]->type == types[t]) {
                list.entities[list.len++] = entity_pool[i];
                break;
            }
        }
    }
    return list;
}

void entity_kill(entity_t *e) {
    if (!e || !e->is_alive) return;
    e->is_alive = false;
    if (e->kill) e->kill(e);
}

void entity_update_all(void) {
    for (int i = 0; i < entity_pool_count; i++) {
        if (!entity_pool[i]->is_alive) continue;
        if (entity_pool[i]->update) entity_pool[i]->update(entity_pool[i]);
    }
}

void entity_draw_all(void) {
    for (int i = 0; i < entity_pool_count; i++) {
        if (!entity_pool[i]->is_alive) continue;
        if (entity_pool[i]->draw) entity_pool[i]->draw(entity_pool[i]);
    }
}

void entity_draw_only(entity_type_t *types, int count) {
    for (int i = 0; i < entity_pool_count; i++) {
        if (!entity_pool[i]->is_alive) continue;
        for (int t = 0; t < count; t++) {
            if (entity_pool[i]->type == types[t]) {
                if (entity_pool[i]->draw) entity_pool[i]->draw(entity_pool[i]);
                break;
            }
        }
    }
}

void entity_draw_all_except(entity_type_t *types, int count) {
    for (int i = 0; i < entity_pool_count; i++) {
        if (!entity_pool[i]->is_alive) continue;
        bool skip = false;
        for (int t = 0; t < count; t++) {
            if (entity_pool[i]->type == types[t]) {
                skip = true;
                break;
            }
        }
        if (!skip && entity_pool[i]->draw) entity_pool[i]->draw(entity_pool[i]);
    }
}

void entity_clear_all(void) {
    for (int i = 0; i < entity_pool_count; i++) {
        free(entity_pool[i]);
    }
    entity_pool_count = 0;
}

void entity_cleanup(void) {
    int write = 0;
    for (int read = 0; read < entity_pool_count; read++) {
        if (entity_pool[read]->is_alive) {
            entity_pool[write++] = entity_pool[read];
        } else {
            free(entity_pool[read]);
        }
    }
    entity_pool_count = write;
}

static bool render_initted = false;

void render_reset_textures(void) {
    render_initted = false;
}

static void ensure_render_textures(void) {
    if (render_initted) return;

    rgba_t white_pixel = rgba(255, 255, 255, 255);
    g.white_tex = texture_create(vec2i(1, 1), &white_pixel);

    int circle_size = 32;
    rgba_t *circle_pixels = temp_alloc(circle_size * circle_size * sizeof(rgba_t));
    float cx = circle_size / 2.0f;
    float cy = circle_size / 2.0f;
    float radius = circle_size / 2.0f - 0.5f;
    for (int y = 0; y < circle_size; y++) {
        for (int x = 0; x < circle_size; x++) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist <= radius) {
                circle_pixels[y * circle_size + x] = rgba(255, 255, 255, 255);
            } else {
                circle_pixels[y * circle_size + x] = rgba(0, 0, 0, 0);
            }
        }
    }
    g.circle_tex = texture_create(vec2i(circle_size, circle_size), circle_pixels);
    g.circle_tex_size = vec2(circle_size, circle_size);
    temp_free(circle_pixels);

    int light_size = 256;
    rgba_t *light_pixels = temp_alloc(light_size * light_size * sizeof(rgba_t));
    float lcx = (light_size - 1) / 2.0f;
    float lcy = (light_size - 1) / 2.0f;
    float lradius = light_size / 2.0f;
    for (int y = 0; y < light_size; y++) {
        for (int x = 0; x < light_size; x++) {
            float dx = x - lcx;
            float dy = y - lcy;
            float d = sqrtf(dx * dx + dy * dy) / lradius;
            d *= d;
            int alpha = (int)(255.0f * expf(-d * 5.0f));
            light_pixels[y * light_size + x] = rgba(255, 255, 255, alpha);
        }
    }
    g.light_tex = texture_create(vec2i(light_size, light_size), light_pixels);
    temp_free(light_pixels);

    render_initted = true;
}

void entity_draw_lights(void) {
    if (!render_initted) ensure_render_textures();
    render_flush();
    render_set_texture_filter(RENDER_FILTER_LINEAR);
    render_set_blend_mode(RENDER_BLEND_NORMAL);
    vec2_t light_tex_size = vec2(256, 256);
    for (int i = 0; i < entity_pool_count; i++) {
        entity_t *e = entity_pool[i];
        if (!e->is_alive) continue;
        rgba_t color;
        float radius;
        int base_alpha = 0;
        switch (e->type) {
            case ENTITY_PLAYER:          base_alpha = 16; color = rgba( 80, 160, 255, 16); radius = 120; break;
            case ENTITY_PLAYER2:         base_alpha = 16; color = rgba( 80, 255, 80, 16); radius = 120; break;
            case ENTITY_ENEMY_TRIANGLE:  base_alpha = 14; color = rgba(100, 100, 255, 14); radius = 160; break;
            case ENTITY_ENEMY_SQUARE:    base_alpha = 20; color = rgba(255, 100, 100, 20); radius = 160; break;
            case ENTITY_ENEMY_HEXAGON:   base_alpha = 20; color = rgba(255, 255, 100, 20); radius = 160; break;
            case ENTITY_ENEMY_CIRCLE:    base_alpha = 20; color = rgba(100, 255, 100, 20); radius = 160; break;
            case ENTITY_BULLET:          color = rgba(  0, 255, 120, 8); radius = 40; break;
            case ENTITY_MINE:
                base_alpha = 12;
                color = rgba(0, 80, 0, 12);
                radius = 60;
                goto density_alpha;
            case ENTITY_BLACKHOLE:       color = rgba(120,  40, 180, 20); radius = 160; break;
            case ENTITY_POWERUP_SHIELD:  color = rgba( 60,  80, 255, 40); radius = 60; break;
            case ENTITY_POWERUP_MAGNET:  color = rgba(255, 160,  40, 40); radius = 60; break;
            case ENTITY_GAMBO:
                color = rgba(255, 255, 80, 8);
                radius = 60;
                goto density_alpha;
            default: continue;
        }
        if (base_alpha > 0) {
            entity_type_t same_type = e->type;
            entity_list_t nearby_type = entities_by_types(&same_type, 1);
            int nearby = 1;
            for (int j = 0; j < nearby_type.len; j++) {
                entity_t *o = nearby_type.entities[j];
                if (o == e) continue;
                float dx = e->pos.x - o->pos.x;
                float dy = e->pos.y - o->pos.y;
                if (dx * dx + dy * dy < 6400) nearby++;
            }
            int a = (int)(base_alpha / sqrtf((float)nearby));
            if (a < 2) a = 2;
            color = rgba(color.r, color.g, color.b, a);
        }
        density_alpha:;
        float s = radius * 2;
        render_push();
        render_translate(e->pos);
        render_draw(vec2(-radius, -radius), vec2(s, s), g.light_tex, vec2(0, 0), light_tex_size, color);
        render_pop();
    }
    render_flush();
    render_set_texture_filter(RENDER_FILTER_NEAREST);
    render_set_blend_mode(RENDER_BLEND_NORMAL);
}

void render_push_matrix(void) {
    render_push();
}

void render_pop_matrix(void) {
    render_pop();
}

void render_draw_line(float x1, float y1, float x2, float y2, uint8_t cr, uint8_t cg, uint8_t cb) {
    if (!render_initted) ensure_render_textures();
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.5f) return;
    float angle = atan2f(dy, dx);
    render_push();
    render_translate(vec2(x1, y1));
    render_rotate(angle);
    render_draw(vec2(0, -0.5f), vec2(len, 1.0f), g.white_tex, vec2(0, 0), vec2(1, 1), rgba(cr, cg, cb, 255));
    render_pop();
}

void render_draw_rect(float x, float y, float w, float h, uint8_t cr, uint8_t cg, uint8_t cb) {
    if (!render_initted) ensure_render_textures();
    render_draw(vec2(x, y), vec2(w, h), g.white_tex, vec2(0, 0), vec2(1, 1), rgba(cr, cg, cb, 255));
}

void render_draw_circle(float cx, float cy, float radius, uint8_t cr, uint8_t cg, uint8_t cb) {
    if (!render_initted) ensure_render_textures();
    float size = radius * 2.0f;
    render_draw(vec2(cx - radius, cy - radius), vec2(size, size), g.circle_tex, vec2(0, 0), g.circle_tex_size, rgba(cr, cg, cb, 255));
}

void render_draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, uint8_t cr, uint8_t cg, uint8_t cb) {
    render_draw_line(x1, y1, x2, y2, cr, cg, cb);
    render_draw_line(x2, y2, x3, y3, cr, cg, cb);
    render_draw_line(x3, y3, x1, y1, cr, cg, cb);
}

void render_set_clear_color(rgba_t color) {
    (void)color;
}

bool input_down(uint8_t action) {
    return input_state(action) > 0.5f;
}

bool input_mouse_down(int button) {
    return g.mouse_buttons[button];
}
