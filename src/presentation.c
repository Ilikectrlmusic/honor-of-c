/* Match bookends: all positions below are in the same 1280x720 virtual viewport as the HUD. */
#include "presentation.h"
#include "draw.h"
#include <math.h>
#define TAU 6.283185307f
#define GOLD 0xf2cd87
#define PAPER 0xfff2d7
static float sat(float v) {
    return fminf(1, fmaxf(0, v));
}
static float ease(float v) {
    v = sat(v);
    return v * v * (3 - 2 * v);
}
static float hash(int i) {
    unsigned n = (unsigned)(i + 71) * 747796405u + 2891336453u;
    n = ((n >> ((n >> 28) + 4)) ^ n) * 277803737u;
    return ((n >> 22) ^ n) / 4294967296.f;
}
static void star(float x, float y, float r, unsigned c, float a) {
    float p[] = {x, y - r * 1.4f, x + r * .21f, y - r * .20f, x + r, y, x + r * .2f, y + r * .2f,
                 x, y + r * 1.4f, x - r * .2f,  y + r * .2f,  x - r, y, x - r * .2f, y - r * .2f};
    poly(p, 8, c, a);
}
static void corners(float x, float y, float w, float h, unsigned color, float a) {
    for (int j = 0; j < 4; j++) {
        float dx = j % 2 ? -1 : 1, dy = j / 2 ? -1 : 1, px = x + (dx < 0 ? w : 0), py = y + (dy < 0 ? h : 0);
        float pts[] = {px,          py,          px + dx * 18, py, px + dx * 8,
                       py + 2 * dy, px + dx * 2, py + 8 * dy,  px, py + 18 * dy};
        poly(pts, 5, color, a);
    }
}
static V along(float u, float x, float y, float w, float h) {
    float d = fmodf(u + 4 * (w + h), 2 * (w + h));
    if (d < w)
        return vec(x + d, y);
    if ((d -= w) < h)
        return vec(x + w, y + d);
    if ((d -= h) < w)
        return vec(x + w - d, y + h);
    return vec(x, y + h - (d - w));
}
static void liquid_gold(float x, float y, float w, float h, float t) {
    for (int k = 0; k < 2; k++) {
        float head = t * 245 + (w + h) * k;
        for (int j = 0; j < 32; j++) {
            float a = 1 - j / 32.f;
            V p = along(head - j * 2.5f, x, y, w, h), q = along(head - (j + 1) * 2.5f, x, y, w, h);
            line(p.x, p.y, q.x, q.y, 0xffa030, a * .28f, 7);
            line(p.x, p.y, q.x, q.y, 0xffe797, a, 1.2f + 2 * a);
        }
        V p = along(head, x, y, w, h);
        star(p.x, p.y, 5, 0xfff7d3, 1);
    }
}
static void card(const Unit *u, int id, int col, int team, float t) {
    float e = ease((t - col * .075f - team * .15f) / .62f),
          x = 151 + col * 197 + (team ? 1 : -1) * (1 - e) * 68;
    float y = (team ? 422 : 59) + (team ? 1 : -1) * (1 - e) * 122, w = 190, h = 222;
    unsigned accent = id == 0 ? GOLD : team ? 0xd3908a : 0x80d2dc;
    if (!e)
        return;
    box(x - 3, y + 4, w + 6, h + 2, 0x020b12, e * .72f);
    box(x, y, w, h, 0x182731, e);
    portrait(u->role, x + 2, y + 2, w - 4, h - 4, .21f, e);
    gradient(x + 2, y + 2, w - 4, 46, 0x09111c, .50f * e, 0x09111c, 0, 1);
    gradient(x + 2, y + h - 102, w - 4, 100, 0x07121d, 0, team ? 0x291824 : 0x081e29, .99f * e, 1);
    frame(x + .5f, y + .5f, w - 1, h - 1, accent, id == 0 ? 1 : e * .67f, id == 0 ? 2.6f : 1.2f);
    frame(x + 4, y + 4, w - 8, h - 8, accent, e * .32f, 1);
    corners(x + 2, y + 2, w - 4, h - 4, accent, e);
    if (!id) {
        float tag[] = {x + 7, y + 7, x + 61, y + 7, x + 54, y + 29, x + 7, y + 29};
        poly(tag, 4, 0xf1d59b, e);
        label(L"玩家", x + 9, y + 7, 43, 21, 13, 0x332317, e, F_BOLD, 1);
        liquid_gold(x, y, w, h, t);
    }
    label(heroes[u->role].name, x + 14, y + h - 55, w - 28, 35, 27, PAPER, e, F_SERIF, 0);
    label(heroes[u->role].role, x + 16, y + h - 24, w - 30, 17, 12, accent, e, F_UI, 0);
    for (int j = 0; j < 3; j++)
        line(x + w - 22 - j * 5, y + h - 15, x + w - 18 - j * 5, y + h - 21, accent, e * (.8f - j * .18f),
             1.4f);
}
void present_versus(const Game *g, const int slots[10], float t) {
    cover(ART_LANDSCAPE, 0, 0, 1280, 720, .22f, 1);
    box(0, 0, 1280, 720, 0x091827, .87f);
    glow(310, 99, 830, 290, 0x29677e, .60f);
    glow(970, 643, 840, 280, 0x813748, .57f);
    for (int j = 0; j < 45; j++) {
        float k = fmodf(hash(j) * 2 + t * .06f, 1), x = hash(j + 77) * 1320 - 20 + k * 15, y = (1 - k) * 720;
        star(x, y, 1 + hash(j + 134) * 1.2f, j % 3 ? 0x84b8cd : GOLD, .7f * sinf(k * 3.14159f));
    }
    label(L"我 方 阵 容", 151, 13, 250, 35, 20, 0xb4edf2, 1, F_SERIF, 0);
    label(L"苍 澜  /  ALLIES", 871, 17, 258, 28, 13, 0xa6d3d9, 1, F_BOLD, 2);
    label(L"敌 方 阵 容", 151, 655, 258, 35, 20, 0xf3c2b4, 1, F_SERIF, 0);
    label(L"赤 霄  /  RIVALS", 871, 657, 258, 30, 13, 0xdbb3a8, 1, F_BOLD, 2);
    for (int i = 0; i < 10; i++)
        card(&g->u[slots[i]], slots[i], i % 5, i / 5, t);
    float a = ease((t - .34f) / .52f);
    for (int i = 0; i < 2; i++) {
        float side = i ? 1 : -1;
        float dx = 640 + side * 86, edge = dx + side * 407 * a;
        line(dx, 351, edge, 351, GOLD, .64f * a, 1);
        line(dx + side * 11, 355, edge - side * 55, 355, i ? 0xc88376 : 0x74bbce, .5f * a, 1);
        star(dx, 351, 4, GOLD, a);
    }
    float size = 100 + 4 * floorf((1 - a) * 10);
    label(L"VS", 512, 278, 256, 133, size, 0x000b13, a, F_NUM, 1);
    label(L"VS", 510, 274, 256, 133, size, 0xf6d491, a, F_NUM, 1);
    label(L"峡   谷   对   决", 486, 375, 308, 27, 14, 0xf2d7a8, a, F_SERIF, 1);
    box(0, 715, 1280 * sat(t / VS_DURATION), 2, GOLD, .74f);
    box(0, 0, 1280, 720, 0x071321, ease((t - VS_DURATION + .31f) / .31f));
}
static void shard(float x, float y, float r, float angle, unsigned light, unsigned dark, float alpha) {
    float dx = cosf(angle) * r, dy = sinf(angle) * r, nx = -dy * .34f, ny = dx * .34f;
    float p[] = {x - dx, y - dy, x + nx, y + ny, x + dx, y + dy, x - nx * .8f, y - ny * .8f};
    poly(p, 4, dark, alpha);
    float f[] = {x - dx, y - dy, x + nx, y + ny, x + dx * .77f, y + dy * .77f};
    poly(f, 3, light, alpha);
    line(x - dx, y - dy, x + nx, y + ny, 0xfff4da, alpha * .85f, 1);
}
void present_core(V feet, float z, int team, float t) {
    unsigned bright = team ? 0xff917d : 0x86ebff, mid = team ? 0xf34454 : 0x50aeff,
             dark = team ? 0x8b2339 : 0x275ba0;
    float sz = 151 * z, x = feet.x, y = feet.y - sz * .966f, cc = y + sz * .34f, b = t - CORE_BREAK;
    int id = team ? 11 : 6;
    if (b < 0) {
        float charge = ease((t - .48f) / .97f);
        float shake = charge * charge * 3.5f * z;
        x += shake * sinf(t * 87);
        oval(x, feet.y - sz * .09f, sz * .39f, sz * .12f, mid, .17f + .23f * charge, 1, 0);
        sprite(id, x, feet.y, sz, 1, 0);
        for (int k = 0; k < 12; k++) {
            float q = TAU * k / 12.f, r = (15 + 31 * hash(k)) * z * charge;
            line(x + cosf(q) * 4 * z, cc + sinf(q) * 7 * z, x + cosf(q) * r, cc + sinf(q) * r, bright,
                 charge * .76f, 1.2f * z);
        }
        float crack[] = {-.09f, .14f, -.031f, .23f, -.053f, .27f, .003f, .33f, -.019f, .37f, .07f, .47f};
        for (int i = 0; i < 5; i++)
            line(x + crack[i * 2] * sz, y + crack[i * 2 + 1] * sz, x + crack[i * 2 + 2] * sz,
                 y + crack[i * 2 + 3] * sz, 0xfff9dc, charge, 2.35f * z);
        for (int i = 0; i < 18; i++) {
            float k = fmodf(t * .95f + hash(i + 23), 1), ang = i * 2.399f;
            star(x + cosf(ang) * (1 - k) * 105 * z, cc + sinf(ang) * (1 - k) * 63 * z, 2.2f * z * charge,
                 bright, charge * sinf(k * 3.14159f));
        }
        return;
    }
    /* The base is cut from the actual destroyed tower, not a generic blast sprite. */
    float settle = ease(b / .52f);
    for (int i = 0; i < 3; i++) {
        float sx = i * 85.f, sw = i == 2 ? 86 : 85, h = 136 * sz / 256;
        art(ART_UNITS, x - sz * .5f + sx * sz / 256 + (i - 1) * 4 * z * settle,
            y + sz * 120 / 256 + (i == 1 ? 12 : 4) * z * settle, sw * sz / 256, h, (id % 6) * 256 + sx,
            (id / 6) * 256 + 120, sw, 136, 1, 0);
    }
    /* Jagged low splinters still embedded in the split socket. */
    for (int i = 0; i < 7; i++) {
        float ox = (-.205f + i * .067f) * sz,
              ground = y + sz * (.49f + .012f * hash(120 + i)) + 2 * z * settle;
        float top = ground - sz * (.023f + .052f * hash(141 + i));
        float facet[] = {x + ox - 4 * z, ground,    x + ox + z * (hash(i) - .5f) * 6, top,
                         x + ox + 5 * z, ground + z};
        poly(facet, 3, dark, .98f);
        line(facet[0], facet[1], facet[2], facet[3], bright, .9f, 1.4f * z);
    }
    if (b < .7f)
        glow(x, cc, 135 * z, 100 * z, bright, .83f * (1 - ease(b / .7f)));
    for (int i = 0; i < 28; i++) {
        float ang = TAU * hash(i + 4), life = .72f + .57f * hash(i + 91), u = sat(b / life);
        float dx = cosf(ang) * (43 + 93 * hash(i + 39)), dy = sinf(ang) * (18 + 55 * hash(i + 67));
        float xx = x + dx * z * (1 - (1 - u) * (1 - u)), ground = feet.y + dy * z;
        float yy = cc + (ground - cc) * u - (42 + 94 * hash(i + 18)) * z * 4 * u * (1 - u);
        shard(xx, yy, (i < 9 ? 8.8f : 3.9f) * z * (.68f + hash(i + 31)) * (1 - .38f * u),
              ang + u * (i % 2 ? 5 : -4), bright, dark, i < 14 ? 1 : 1 - .72f * u);
    }
    for (int i = 0; i < 3; i++) {
        float u = sat((b - i * .072f) / (.78f + i * .06f));
        if (u > 0 && u < 1)
            oval(x, feet.y - 15 * z, (15 + 156 * u) * z, (8 + 50 * u) * z, i == 0 ? 0xffead6 : bright,
                 (1 - u) * .91f, 0, (1 - u) * 5 * z + .8f);
    }
    for (int i = 0; i < 38; i++) {
        float u = sat(b / (.45f + hash(i + 74) * .9f)), ang = i * 2.399f,
              dist = (25 + 116 * hash(i + 50)) * z;
        float ex = x + cosf(ang) * dist * u, ey = cc + sinf(ang) * dist * u + 18 * z * u * u;
        line(ex, ey, ex - cosf(ang) * 11 * z * (1 - u), ey - sinf(ang) * 11 * z * (1 - u),
             i % 3 ? bright : 0xffedb0, (1 - u), 1.3f * z);
    }
    if (b < .35f) {
        float a = 1 - sat(b / .35f);
        star(x, cc, (26 + 31 * (1 - a)) * z, 0xfff1d4, a);
        box(0, 0, 1280, 720, 0xffedd6, .6f * a * a);
    }
}
static void gilt_text(const wchar_t *s, float cy, float scale, int won, float a) {
    float size = 4 * roundf(98 * scale / 4), h = 124 * scale, y = cy - h / 2;
    label(s, 324, y + 4 * scale, 632, h, size, won ? 0x62431f : 0x55202a, a, F_BOLD, 1);
    label(s, 322, y, 636, h, size, won ? 0xf5d071 : 0xe3aeb2, a, F_BOLD, 1);
    cursor_clip(320, y, 640, h * .53f);
    label(s, 322, y, 636, h, size, won ? 0xfff7ce : 0xffe9df, a, F_BOLD, 1);
    clip_end();
}
void present_ending(int won, float t) {
    float age = t - CREST_AT;
    if (age < 0)
        return;
    float f = ease(age / .44f), cx = 640, cy = 349,
          sc = (won ? 1.f : .92f) * (.84f + .16f * f + .033f * sinf(sat(age / .52f) * 3.14159f));
    unsigned c = won ? 0x68c9ff : 0xec5263;
    box(0, 0, 1280, 720, 0x071022, .52f * f);
    gradient(0, 0, 1280, 280, won ? 0x143452 : 0x521e2a, .55f * f, 0x10121b, 0, 1);
    gradient(0, 466, 1280, 254, 0x09151e, 0, won ? 0x0c2b53 : 0x501f2b, .62f * f, 1);
    glow(cx, cy + 45, 610 * sc, 177 * sc, c, (.41f + .09f * cosf(age * 2)) * f);
    glow(cx, cy + (won ? 148 : 86) * sc, won ? 160 : 427, won ? 150 : 63, won ? 0x3298ff : 0xff3546,
         .62f * f);
    for (int i = 0; i < 18; i++) {
        float side = i % 2 ? -1 : 1, extent = side * (565 + 110 * hash(i + 5)), yoff = -109 + 215 * hash(i);
        float flare[] = {cx,          cy + 24,      cx + extent, cy + yoff - 4 - 7 * hash(i + 3),
                         cx + extent, cy + yoff + 3};
        poly(flare, 3, c, (.045f + .038f * hash(i + 9)) * f);
    }
    for (int i = 0; i < 42; i++) {
        float u = fmodf(hash(i) + age * (.13f + hash(i + 45) * .09f), 1);
        float xx = cx + (i % 2 ? -1 : 1) * (142 + u * (299 + hash(i + 1) * 200)),
              yy = cy + (hash(i + 21) - .5f) * 225 - u * u * 77;
        star(xx, yy, (1.2f + hash(i + 51) * 2.4f), i % 3 ? c : 0xffdb95, f * sinf(u * 3.14159f));
    }
    float w = 1130 * sc, h = (won ? 417 : 393) * sc;
    art(ART_CRESTS, cx - w / 2, cy - h / 2, w, h, 0, won ? 0 : 512, 1536, 512, f, 0);
    gilt_text(won ? L"胜利" : L"失败", cy + 6 * sc, sc, won, f);
    label(won ? L"V I C T O R Y" : L"D E F E A T", 415, cy + 59 * sc, 450, 28 * sc, 19,
          won ? 0xffe6a6 : 0xffcfc4, f, F_NUM, 1);
    if (t >= CONTINUE_AT) {
        float a = (.84f + .14f * sinf(age * 2)) * ease((t - CONTINUE_AT) / .22f);
        label(L"点击屏幕继续", 399, 643, 482, 40, 23, PAPER, a, F_BOLD, 1);
    }
}
