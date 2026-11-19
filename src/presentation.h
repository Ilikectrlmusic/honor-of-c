#ifndef RIFT_PRESENTATION_H
#define RIFT_PRESENTATION_H
#include "game.h"
#define VS_DURATION 3.8f
#define CORE_BREAK 1.45f
#define CREST_AT 2.95f
#define CONTINUE_AT 3.60f
void present_versus(const Game *g, const int lineup[10], float t);
void present_core(V feet, float zoom, int team, float t);
void present_ending(int won, float t);
#endif
