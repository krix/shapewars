#ifndef SOUNDS_H
#define SOUNDS_H

#include "main.h"

void sounds_init(void);

extern sound_source_t *sound_shoot;
extern sound_source_t *sound_explosion;
extern sound_source_t *sound_explosion_big;
extern sound_source_t *sound_player_hit;
extern sound_source_t *sound_menu_select;
extern sound_source_t *sound_collect;

#endif
