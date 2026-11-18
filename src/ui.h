#ifndef RIFT_UI_H
#define RIFT_UI_H
#include "game.h"
enum { HOME, DRAFT, MATCH };
enum { NONE, GUIDE, PAUSED, SHOP, SCORE, LEAVE };
typedef struct {
    Game g;
    int screen, overlay, guideBack, selected, difficulty, muted, fullscreen, requestFull, quit, hoverUnit,
        view, held[256], guidePage, pending;
    float mx, my, clock, zoom, hoverTime;
    V cam;
} App;
extern App app;
void ui_reset(void);
void ui_start(uint32_t seed);
void ui_tick(float dt);
void ui_draw(void);
void ui_click(int right);
void ui_key(int key, int down);
void ui_wheel(int delta);
void ui_blur(void);
#endif
