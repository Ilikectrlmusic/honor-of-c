#ifndef RIFT_DRAW_H
#define RIFT_DRAW_H
#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <stdint.h>
#include <wchar.h>
enum {
    ART_TITLE,
    ART_HEROES,
    ART_MAP,
    ART_UNITS,
    ART_SKILLS,
    ART_MOTION0,
    ART_MOTION1,
    ART_MOTION2,
    ART_ULTIMATE,
    ART_SILK, ART_IRON, ART_LANDSCAPE, ART_SWORDSMAN, ART_DOMAINS, ART_PROPS, ART_CRESTS,
    ART_COUNT
};
enum { F_UI, F_BOLD, F_SERIF, F_NUM };
int draw_init(HWND wnd, int width, int height);
void draw_close(void);
void draw_resize(int w, int h);
void draw_begin(void);
int draw_end(void);
#ifdef RIFT_CAPTURE
int draw_copy_pixels(unsigned char *output,int bytes);
#endif
uint32_t draw_region_hash(int x,int y,int w,int h);  /* WIC gallery verification only */
int draw_save_bmp(const wchar_t *path);
void to_virtual(float *x, float *y);
void box(float x, float y, float w, float h, uint32_t c, float a);
void frame(float x, float y, float w, float h, uint32_t c, float a, float stroke);
void line(float x, float y, float xx, float yy, uint32_t c, float a, float stroke);
void oval(float x, float y, float rx, float ry, uint32_t c, float a, int fill, float stroke);
void poly(const float *points, int n, uint32_t c, float a);
void gradient(float x, float y, float w, float h, uint32_t c, float a, uint32_t d, float b, int vertical);
void glow(float cx,float cy,float rx,float ry,uint32_t color,float alpha);
float text_width(const wchar_t *s,float size,int font);
void label(const wchar_t *s, float x, float y, float w, float h, float size, uint32_t c, float a, int font,
           int align);
void art(int id, float x, float y, float w, float h, float sx, float sy, float sw, float sh, float alpha,
         int flip);
/* Only transparent foreground art and scene props enter the animation rig. */
void living_art(int figure,float x,float y,float w,float h,float seconds,float alpha);
void cover(int id, float x, float y, float w, float h, float topBias, float opacity);
void portrait(int hero, float x, float y, float w, float h, float bias, float alpha);
void sprite(int id, float x, float feet, float size, float alpha, int flip);
void effect_stamp(int tile, float cx, float cy, float w, float h, float radians, float opacity);
void invalidate_shroud(void);
void shroud(float x, float y, float w, float h, const unsigned char *light, int cols, int rows, int revision);
void cursor_clip(float x, float y, float w, float h);
void clip_end(void);
#endif
