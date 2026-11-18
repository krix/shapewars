#include "main.h"
#include "touch_input.h"
#include "../high_impact/src/platform.h"
#include "../high_impact/src/image.h"

engine_t engine = {
    .time_real = 0,
    .time_scale = 1.0,
    .time = 0,
    .tick = 0,
    .frame = 0,
    .collision_map = NULL,
    .gravity = 1.0,
};

static scene_t *scene = NULL;
static scene_t *scene_next = NULL;
static bool is_running = false;

static texture_mark_t init_textures_mark;
static image_mark_t init_images_mark;
static bump_mark_t init_bump_mark;
static sound_mark_t init_sounds_mark;

extern void main_init(void);
extern void main_cleanup(void);

void engine_init(void) {
    engine.time_real = platform_now();
    render_init(platform_screen_size());
    sound_init(platform_samplerate());
    sound_init_synth();
    platform_set_audio_mix_cb(sound_mix_stereo);
    input_init();
    main_init();


    init_bump_mark = bump_mark();
    init_images_mark = images_mark();
    init_sounds_mark = sound_mark();
    init_textures_mark = textures_mark();
}

void engine_cleanup(void) {
    main_cleanup();
    input_cleanup();
    sound_cleanup();
    render_cleanup();
}

void engine_set_scene(scene_t *s) {
    scene_next = s;
}

void engine_update(void) {
    double time_frame_start = platform_now();

    if (scene_next) {
        is_running = false;
        if (scene && scene->cleanup) scene->cleanup();

        textures_reset(init_textures_mark);
        images_reset(init_images_mark);
        sound_reset(init_sounds_mark);
        bump_reset(init_bump_mark);

        engine.background_maps_len = 0;
        engine.collision_map = NULL;
        engine.time = 0;
        engine.frame = 0;
        engine.viewport = vec2(0, 0);

        scene = scene_next;
        if (scene->init) scene->init();
        scene_next = NULL;
    }
    is_running = true;

    double time_real_now = platform_now();
    double real_delta = time_real_now - engine.time_real;
    engine.time_real = time_real_now;
    engine.tick = min(real_delta * engine.time_scale, ENGINE_MAX_TICK);
    engine.time += engine.tick;
    engine.frame++;

    platform_poll_gamepad();

    alloc_pool() {
        if (scene->update) scene->update();

        engine.perf.update = platform_now() - time_real_now;

        render_frame_prepare();

        if (scene->draw) scene->draw();

        render_frame_end();
        engine.perf.draw = (platform_now() - time_real_now) - engine.perf.update;
    }

    input_clear();
    touch_input_update();
    temp_alloc_check();

    engine.perf.draw_calls = render_draw_calls();
    engine.perf.total = platform_now() - time_frame_start;
}

bool engine_is_running(void) {
    return is_running;
}

void engine_resize(vec2i_t size) {
    render_resize(size);
}

void engine_add_background_map(map_t *map) {
    (void)map;
}

void engine_set_collision_map(map_t *map) {
    engine.collision_map = map;
}

void scene_base_update(void) {}

void scene_base_draw(void) {}
