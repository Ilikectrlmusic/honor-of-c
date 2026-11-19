#ifndef RIFT_UI_H
#define RIFT_UI_H
#include "game.h"
enum { HOME, DRAFT, MATCH, VERSUS, FINALE, REPORT };
enum { NONE, GUIDE, PAUSED, SHOP, SCORE, LEAVE, REHEARSAL };
typedef struct {
    Game g;
    int screen, overlay, guideBack, selected, difficulty, muted, fullscreen, requestFull, quit, hoverUnit,
        view, held[256], guidePage, pending, panConsumed;
    float mx, my, clock, selectedAt, zoom, hoverTime, deathShade;
    V cam, endFrom;
    float sceneTime, endZoom;
    int lineup[10]; /* display slots map to game hero ids, never reorder the simulation */
} App;
extern App app;
void ui_reset(void);
void ui_start(uint32_t seed);
void ui_versus(uint32_t seed);
void ui_tick(float dt);
void ui_draw(void);
void ui_click(int right);
void ui_key(int key, int down);
void ui_wheel(int delta);
void ui_blur(void);
#endif
