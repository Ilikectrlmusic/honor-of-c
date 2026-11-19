#ifndef RIFT_ANIMATION_H
#define RIFT_ANIMATION_H
#include "game.h"
/* Screen coordinates use the same virtual canvas as the unit and HUD passes. */
void motion_frame(int model, int team, int row, int frame, int facing, V feet, float height, float opacity);
void motion_actor(const Unit *u, V feet, float height, float opacity);
void paint_ultimate(const Fx *f, V a, V b, float zoom, int foreground);
void paint_flame_wave(const Shot *s, V p, float zoom);
void paint_gate(V feet, float zoom, float progress, float scale, int front, int channel);
void paint_stone_tell(const Unit *, V feet, V target, float zoom);
void paint_stone_hit(V pos, float zoom, float progress);
void paint_coin_amount(const Fx*,V pos,V target,float zoom);
void paint_coins(const Fx *, V pos, V target, float zoom);
#endif
