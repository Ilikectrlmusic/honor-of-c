#include "domain.h"
#include "draw.h"
#include "living.h"
#include <math.h>
#define TAU 6.28318530718f
static float fract(float v) {
    return v - floorf(v);
}
static float seed(int i) {
    unsigned a = 747796405u * (unsigned)(i + 17) + 2891336453u;
    a = ((a >> ((a >> 28) + 4)) ^ a) * 277803737u;
    return (a >> 8) / 16777216.f;
}
static void glimmer(float x, float y, float r, float a) {
    float p[] = {x, y - r, x + .17f * r, y - .16f * r, x + .72f * r, y, x + .16f * r, y + .15f * r,
                 x, y + r, x - .16f * r, y + .16f * r, x - .72f * r, y, x - .16f * r, y - .16f * r};
    poly(p, 8, 0xffeddb, a);
}
static void mist(float x, float y, float w, float h, float a, int flip) {
    art(ART_PROPS, x, y, w, h, 516, 672, 502, 172, a, flip);
}
static void forge(float x, float y, float w, float h, float t) {
    /* Three distances of fire, rooted at the edge braziers and the bottom coals. */
    float ox[] = {-.01f, .96f, .12f, .91f}, oy[] = {.73f, .62f, .99f, .99f};
    float fw[] = {140, 123, 168, 175}, fh[] = {211, 189, 238, 214};
    for (int i = 0; i < 4; i++) {
        float cx = x + w * ox[i], base = y + h * oy[i], f = .98f + .018f * sinf(t * 4.8f + i);
        oval(cx, base - 2, fw[i] * .32f, 11, 0xff802c, .22f, 1, 0);
        living_art(RIG_FLAME, cx - fw[i] * .5f, base - fh[i] * f, fw[i], fh[i] * f, t, .90f);
        if (i < 2) {
            float rim[] = {cx - 34, base - 3, cx + 34, base - 3, cx + 22, base + 11, cx - 22, base + 11};
            poly(rim, 4, 0x523326, .98f);
            line(cx - 33, base - 3, cx + 33, base - 3, 0xffae56, .9f, 2);
            box(cx - 4, base + 11, 8, h * (1 - oy[i]), 0x3a2a27, .95f);
        }
    }
    for (int j = 0; j < 27; j++) {
        float k = fract(seed(j) + t * (.13f + seed(j + 53) * .09f));
        float xx = x + (j % 2 ? w * .88f : w * .13f) + 37 * sinf(5 * k + j) + 17 * k;
        float yy = y + h * (.83f - .79f * k);
        float a = sinf(k * 3.14159f) * (.46f + .46f * seed(j + 75));
        line(xx - 1, yy + 2 + 3 * seed(j + 100), xx + 1, yy, j % 3 ? 0xffb448 : 0xffe599, a, 1.45f);
    }
}
static void grove(float x, float y, float w, float h, float t) {
    /* Root and culm share one deforming cutout; leaves lag behind the trunk. */
    living_art(RIG_BAMBOO, x - 78, y + 13, 202, 339, t, .78f);
    living_art(RIG_BAMBOO, x + w - 113, y - 22, 209, 387, t, .97f);
    living_art(RIG_BAMBOO, x - 75, y + 38, 237, 467, t, .98f);
    for (int i = 0; i < 8; i++) {
        float k = fract(seed(i) + t * (.053f + i * .003f));
        float px = x + fract(seed(i + 11) + k * .28f) * w, py = y + h * k, th = t * (1.2f + i * .06f) + i;
        float dx = cosf(th) * 5, dy = sinf(th) * 4;
        float p[] = {px - dx,        py - dy,        px - dy * .26f, py + dx * .26f,
                     px + dx * 1.6f, py + dy * 1.6f, px + dy * .38f, py - dx * .38f};
        poly(p, 4, i % 2 ? 0x93bf68 : 0xbad581, .8f * sinf(k * 3.14159f));
    }
}
static void heavens(float x, float y, float w, float h, float t) {
    float mx = x + 74, my = y + 83;
    oval(mx, my, 54, 54, 0xbba4ff, .09f, 1, 0);
    oval(mx, my, 48, 48, 0xc9c1ff, .14f, 1, 0);
    art(ART_PROPS, mx - 43, my - 43, 86, 86, 1073, 63, 416, 416, .96f, 0);
    for (int i = 0; i < 32; i++) {
        float xx = x + seed(3 * i + 17) * w, yy = y + 5 + seed(7 * i + 5) * h * .62f;
        float a = .56f + .42f * sinf(t * (1.17f + seed(i)) + .45f * i);
        if (i % 4 == 0)
            glimmer(xx, yy, 2.4f + 1.8f * seed(i + 4), a);
        else
            oval(xx, yy, 1.1f, 1.1f, 0xffefd8, a, 1, 0);
    }
    for (int j = 0; j < 2; j++) {
        float q = t * .29f + j * 3.15f, px = mx + cosf(q) * 57, py = my + sinf(q) * 17 + cosf(q) * 13;
        glimmer(px, py, 3.6f, .89f);
    }
    mist(x + 2 + 16 * sinf(t * .35f), my + 19, 147, 27, .53f, 0);
    float k = fract(t * .15f + .07f);
    if (k < .27f) {
        float s = k / .27f, px = x + w * (.97f - .69f * s), py = y + h * (.056f + .255f * s);
        for (int i = 0; i < 6; i++)
            line(px + i * 4, py - i * 2, px + (i + 1) * 4, py - (i + 1) * 2, 0xe9d6ff,
                 (1 - i / 6.f) * sinf(s * 3.14159f), 1.5f);
        glimmer(px, py, 3.2f, sinf(s * 3.14159f));
    }
}
static void standard(float x, float y, float scale, float t, float a) {
    float size = 147 * scale, polex = x + size * .051f, poley = y + size * .205f;
    line(polex, poley - 8 * scale, polex, poley + size * 2.08f, 0x4c3828, a, 3.2f * scale);
    line(polex - .6f * scale, poley - 8 * scale, polex - .6f * scale, poley + size * 2.08f, 0xd5ad67,
         a * .85f, 1.1f * scale);
    float finial[] = {polex, poley - 18 * scale, polex + 3.4f * scale, poley - 10 * scale,
                      polex, poley - 5 * scale,  polex - 3.4f * scale, poley - 10 * scale};
    poly(finial, 4, 0xffdd85, a);
    living_art(RIG_STANDARD, x, y, size, size, t, a);
}
static void bastion(float x, float y, float w, float h, float t) {
    standard(x + w * .67f, y + h * .16f, .73f, t, .82f);
    standard(x + w * .031f, y + h * .045f, 1.05f, t, 1);
    standard(x + w * .83f, y + h * .33f, .92f, t, .92f);
    for (int j = 0; j < 9; j++) {
        float k = fract(seed(j) + t * .058f);
        float xx = x + w * fract(seed(j + 7) + k * .26f);
        oval(xx, y + h * (.68f - .45f * k), 1.3f, .8f, 0xffdc99, .69f * sinf(k * 3.14159f), 1, 0);
    }
}
static void night(float x, float y, float w, float h, float t) {
    for (int j = 0; j < 3; j++) {
        float k = fract(t * (.078f + j * .009f) + (.14f + j * .34f));
        float sz = 33 + 10 * (j % 2), bx = x - 42 + k * (w + 84),
              by = y + 36 + j * 40 + sinf(k * TAU * 1.3f + j) * 14;
        living_art(RIG_RAVEN, bx, by, sz, sz * .93f, t, .98f);
    }
    mist(x - 62 + 45 * sinf(t * .29f), y + h * .31f, 275, 52, .51f, 0);
    mist(x + w - 193 + 32 * sinf(t * .34f + 1), y + h * .21f, 256, 46, .44f, 1);
    mist(x - 30 + 25 * sinf(t * .21f + 2), y + h * .68f, w + 43, 77, .48f, 0);
}
void domain_draw(int role, float x, float y, float w, float h, float t) {
    /* Atlas columns are fixed 404px wide. Retain skyline and foreground rather
       than cropping away the visual identity at the edges of each card. */
    art(ART_DOMAINS, x, y, w, h, role * 404.f + 1, 0, 402, 778, 1, 0);
    box(x, y, w, h, 0x0a1a24, role == 1 ? .23f : role == 3 ? .28f : .16f);
    switch (role) {
    case 0:
        forge(x, y, w, h, t);
        break;
    case 1:
        grove(x, y, w, h, t);
        break;
    case 2:
        heavens(x, y, w, h, t);
        break;
    case 3:
        bastion(x, y, w, h, t);
        break;
    default:
        night(x, y, w, h, t);
        break;
    }
}
