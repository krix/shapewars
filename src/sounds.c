#include "sounds.h"

sound_source_t *sound_shoot;
sound_source_t *sound_explosion;
sound_source_t *sound_explosion_big;
sound_source_t *sound_player_hit;
sound_source_t *sound_menu_select;
sound_source_t *sound_collect;

static pl_synth_sound_t _shoot = {
    .synth = {
        9, 0, 0, 0, 220, 0,
        11, 0, 5, 0, 120, 0,
        0,
        40, 180, 350, 200,
        2, 9000, 80, 0, 0,
        0, 0,
        1, 0, 6, 20, 0
    },
    .row_len = 5513,
    .note = 130
};

static pl_synth_sound_t _explosion = {
    .synth = {
        4, 0, 0, 0, 180, 3,
        3, 0, 0, 0, 140, 1,
        200,
        50, 300, 4000, 200,
        2, 1500, 200, 2, 80,
        4, 40,
        0, 1, 6, 80, 0
    },
    .row_len = 5513,
    .note = 105
};

static pl_synth_sound_t _explosion_big = {
    .synth = {
        2, 0, 0, 0, 200, 3,
        1, 0, 0, 0, 160, 1,
        240,
        80, 400, 6000, 220,
        2, 800, 150, 3, 60,
        2, 30,
        0, 1, 4, 90, 0
    },
    .row_len = 5513,
    .note = 80
};

static pl_synth_sound_t _player_hit = {
    .synth = {
        6, 0, 0, 0, 200, 3,
        5, 0, 0, 0, 120, 0,
        100,
        20, 100, 1500, 180,
        1, 3000, 150, 0, 0,
        0, 0,
        0, 0, 0, 0, 0
    },
    .row_len = 5513,
    .note = 115
};

static pl_synth_sound_t _menu_select = {
    .synth = {
        10, 0, 0, 0, 180, 0,
        10, 0, 12, 0, 80, 1,
        0,
        10, 50, 400, 150,
        0, 0, 0, 0, 0,
        0, 0,
        0, 0, 0, 0, 0
    },
    .row_len = 5513,
    .note = 135
};

static pl_synth_sound_t _collect = {
    .synth = {
        12, 0, 0, 1, 200, 0,
        14, 0, 7, 1, 80, 1,
        0,
        100, 100, 500, 180,
        0, 0, 0, 0, 0,
        0, 0,
        1, 0, 12, 50, 0
    },
    .row_len = 5513,
    .note = 140
};

void sounds_init(void) {
    sound_shoot = sound_source_synth_sound(&_shoot);
    sound_explosion = sound_source_synth_sound(&_explosion);
    sound_explosion_big = sound_source_synth_sound(&_explosion_big);
    sound_player_hit = sound_source_synth_sound(&_player_hit);
    sound_menu_select = sound_source_synth_sound(&_menu_select);
    sound_collect = sound_source_synth_sound(&_collect);
}
