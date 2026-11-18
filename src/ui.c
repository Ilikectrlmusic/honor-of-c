#include "ui.h"
#include "draw.h"
#include "audio.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
App app;
#define A app
#define G app.g
#define P app.g.u[0]
#define INK 0x0b1b20
#define DEEP 0x071318
#define PAPER 0xf4efdc
#define GOLD 0xe7c98d
#define MUTED 0xa4bab4
#define JADE 0x69d9c3
#define RED 0xff9378
static const wchar_t *diffs[] = {L"入门", L"标准", L"精锐"};
static int hit(float x, float y, float w, float h) {
    return A.mx >= x && A.my >= y && A.mx < x + w && A.my < y + h;
}
static void txt(const wchar_t *s, float x, float y, float w, float h, float size, uint32_t c, int face,
                int al) {
    label(s, x, y, w, h, size, c, 1, face, al);
}
static void fmt(float x, float y, float w, float h, float size, uint32_t c, int face, int al,
                const wchar_t *format, ...) {
    wchar_t b[200];
    va_list ap;
    va_start(ap, format);
    vswprintf(b, 200, format, ap);
    va_end(ap);
    txt(b, x, y, w, h, size, c, face, al);
}
static void diamond(float x, float y, float r, uint32_t c, float alpha) {
    float p[] = {x, y - r, x + r, y, x, y + r, x - r, y};
    poly(p, 4, c, alpha);
}
static void crest(float x, float y, float s, uint32_t c) {
    float p[] = {x - 17 * s, y + 12 * s, x,         y - 18 * s, x + 17 * s, y + 12 * s,
                 x + 5 * s,  y + 5 * s,  x - 1 * s, y - 5 * s,  x - 8 * s,  y + 7 * s};
    poly(p, 6, c, 1);
    float b[] = {x - 12 * s, y + 16 * s, x, y + 7 * s, x + 11 * s, y + 15 * s, x, y + 12 * s};
    poly(b, 4, c, .85f);
}
static void button(float x, float y, float w, float h, const wchar_t *name, int primary) {
    int over = hit(x, y, w, h);
    box(x, y, w, h, primary ? (over ? 0xf7dfab : GOLD) : (over ? 0x314540 : INK), primary ? 1 : .93f);
    frame(x + .5f, y + .5f, w - 1, h - 1, primary ? 0xf7dfab : GOLD, primary ? .9f : .45f, 1);
    txt(name, x, y, w, h, 16, primary ? INK : PAPER, F_BOLD, 1);
}
static void chip(float x, float y, float w, const wchar_t *s, uint32_t c) {
    box(x, y, w, 23, INK, .87f);
    frame(x + .5f, y + .5f, w - 1, 22, c, .55f, 1);
    txt(s, x, y, w, 23, 11, c, F_BOLD, 1);
}
static void rule(float x, float y, float w) {
    line(x, y, x + w, y, GOLD, .32f, 1);
}
static void mast(void) {
    crest(65, 46, .78f, GOLD);
    txt(L"岚境", 90, 23, 80, 32, 23, PAPER, F_SERIF, 0);
    txt(L"R I F T W A R D", 91, 52, 143, 17, 10, GOLD, F_NUM, 0);
    chip(1036, 33, 90, A.muted ? L"M  ·  静音" : L"M  ·  声音", A.muted ? MUTED : GOLD);
    chip(1137, 33, 90, L"F11  ·  全屏", GOLD);
}
static void home(void) {
    cover(ART_TITLE, 0, 0, 1280, 720, .15f, 1);
    gradient(0, 0, 795, 720, 0x071a1d, .93f, 0x081619, 0, 0);
    gradient(0, 556, 1280, 164, DEEP, 0, DEEP, .91f, 1);
    mast();
    box(66, 176, 37, 2, GOLD, 1);
    txt(L"O F F L I N E   C H R O N I C L E S", 115, 164, 410, 25, 13, GOLD, F_NUM, 0);
    txt(L"岚 境", 57, 182, 560, 143, 113, PAPER, F_SERIF, 0);
    txt(L"R I F T W A R D", 68, 320, 490, 49, 36, GOLD, F_NUM, 0);
    txt(L"五人结阵，三路争锋。", 68, 391, 500, 42, 28, PAPER, F_SERIF, 0);
    txt(L"集结四位人机队友，率领兵线攻破敌方水晶。", 69, 444, 520, 29, 16, 0xc5d2c7, F_UI, 0);
    button(69, 510, 213, 52, L"选择英雄     →", 1);
    button(296, 510, 146, 52, L"战场指南", 0);
    rule(68, 637, 1143);
    oval(75, 665, 4, 4, JADE, 1, 1, 1);
    txt(L"离线就绪", 88, 650, 102, 30, 13, PAPER, F_BOLD, 0);
    txt(L"无需登录  /  本地模拟", 185, 650, 270, 30, 13, MUTED, F_UI, 0);
    txt(L"5 V 5     /     单 人 战 役", 865, 650, 346, 30, 13, GOLD, F_UI, 2);
}
static void glyph(int type, float x, float y, float sz, uint32_t c) {
    float s = sz / 24;
    switch (type % 8) {
    case 0: {
        float p[] = {x - 9 * s, y + 7 * s, x + 9 * s, y - 11 * s, x + 6 * s, y - 2 * s, x - 5 * s, y + 8 * s};
        poly(p, 4, c, 1);
        line(x - 10 * s, y + 2 * s, x - 1 * s, y + 11 * s, c, 1, 2 * s);
        break;
    }
    case 1:
        oval(x, y, 9 * s, 9 * s, c, .9f, 0, 1.8f * s);
        diamond(x, y, 5 * s, c, 1);
        break;
    case 2:
        for (int j = -1; j <= 1; j++) {
            line(x + (j * 5 - 5) * s, y + 8 * s, x + (j * 5 + 4) * s, y - 7 * s, c, 1, 2 * s);
            diamond(x + (j * 5 + 4) * s, y - 7 * s, 2.2f * s, c, 1);
        }
        break;
    case 3:
        line(x - 9 * s, y + 9 * s, x + 8 * s, y - 8 * s, c, 1, 2 * s);
        line(x + 8 * s, y - 8 * s, x + 2 * s, y - 8 * s, c, 1, 2 * s);
        line(x + 8 * s, y - 8 * s, x + 8 * s, y - 2 * s, c, 1, 2 * s);
        break;
    case 4:
        for (int j = 0; j < 3; j++)
            line(x - 10 * s + j * 3 * s, y + (j - 1) * 5 * s, x + 5 * s + j * 2 * s, y + (j - 2) * 5 * s, c,
                 1, 2 * s);
        break;
    case 5:
        diamond(x, y, 10 * s, c, .7f);
        diamond(x, y, 5 * s, INK, 1);
        break;
    case 6: {
        float p[] = {x - 8 * s, y - 8 * s, x, y - 10 * s, x + 8 * s, y - 8 * s,
                     x + 7 * s, y + 3 * s, x, y + 11 * s, x - 7 * s, y + 3 * s};
        poly(p, 6, c, .9f);
        break;
    }
    case 7:
        oval(x, y, 9 * s, 9 * s, c, 1, 0, 2 * s);
        line(x, y - 6 * s, x, y + 5 * s, c, 1, 2 * s);
        line(x - 5 * s, y, x + 5 * s, y, c, 1, 2 * s);
        break;
    }
}
static void skillart(int role, int slot, float x, float y, float w, float h, float alpha) {
    float sy = (float[]){0, 160, 310}[slot];
    float sh = slot == 0 ? 159 : slot == 1 ? 149 : 169;
    float sw = 156, ratio = w / h;
    if (sw / sh > ratio)
        sw = sh * ratio;
    else
        sh = sw / ratio;
    art(ART_SKILLS, x, y, w, h, 160 * role + (160 - sw) * .5f,
        sy + ((slot == 0   ? 159
               : slot == 1 ? 149
                           : 169) -
              sh) *
                 .5f,
        sw, sh, alpha, 0);
}
static void draft(void) {
    cover(ART_TITLE, 0, 0, 1280, 720, .3f, .3f);
    box(0, 0, 1280, 720, DEEP, .77f);
    mast();
    txt(L"战前集结", 60, 87, 365, 65, 47, PAPER, F_SERIF, 0);
    txt(L"H E R O   S E L E C T", 63, 148, 350, 18, 12, GOLD, F_NUM, 0);
    txt(L"选择你的英雄   /   其余九名英雄由本地 AI 操控", 692, 107, 528, 43, 16, MUTED, F_UI, 2);
    for (int i = 0; i < 5; i++) {
        float x = 61 + i * 234;
        int sel = i == A.selected;
        uint32_t c = heroes[i].color;
        int over = hit(x, 191, 222, 280);
        portrait(i, x, 191, 222, 280, .24f, sel || over ? 1 : .74f);
        gradient(x, 321, 222, 150, DEEP, 0, DEEP, .97f, 1);
        frame(x, 191, 222, 280, sel ? GOLD : over ? PAPER : 0x54625d, sel ? 1 : .7f, sel ? 3 : 1);
        if (sel) {
            box(x, 191, 222, 4, GOLD, 1);
            chip(x + 10, 202, 56, L"已选择", GOLD);
        } else
            fmt(x + 12, 198, 60, 22, 13, PAPER, F_NUM, 0, L"0%d", i + 1);
        txt(heroes[i].name, x + 16, 389, 133, 48, 34, PAPER, F_SERIF, 0);
        txt(heroes[i].role, x + 17, 436, 63, 22, 14, c, F_BOLD, 0);
        txt(heroes[i].tag, x + 80, 436, 127, 22, 13, 0xd0d8ca, F_UI, 2);
    }
    const HeroDef *h = &heroes[A.selected];
    txt(h->bio, 63, 487, 690, 34, 20, PAPER, F_SERIF, 0);
    for (int j = 0; j < 3; j++) {
        float x = 64 + j * 230;
        skillart(A.selected, j, x, 531, 37, 37, 1);
        frame(x, 531, 37, 37, h->color, .75f, 1);
        fmt(x + 46, 535, 174, 30, 15, PAPER, F_BOLD, 0, L"%lc   %ls", L"QER"[j], h->skill[j]);
    }
    txt(L"敌方强度", 828, 495, 115, 32, 15, MUTED, F_UI, 0);
    for (int i = 0; i < 3; i++) {
        float x = 925 + i * 96;
        box(x, 493, 87, 35, A.difficulty == i ? GOLD : INK, 1);
        frame(x, 493, 87, 35, GOLD, .46f, 1);
        txt(diffs[i], x, 493, 87, 35, 14, A.difficulty == i ? INK : PAPER, F_BOLD, 1);
    }
    for (int j = 0; j < 3; j++)
        if (hit(64 + j * 230, 526, 220, 51))
            txt(h->desc[j], 64, 566, 738, 24, 13, GOLD, F_UI, 0);
    rule(62, 597, 1157);
    button(62, 621, 133, 48, L"←  返回", 0);
    txt(L"1–5  选英雄     ·     Enter  出战", 222, 625, 506, 43, 14, MUTED, F_UI, 0);
    button(977, 615, 243, 59, L"进入峡谷     →", 1);
}
static V world(V p) {
    return vec((p.x - A.cam.x) * A.zoom + 640, (p.y - A.cam.y) * A.zoom + 360);
}
static V mouseworld(void) {
    return vec((A.mx - 640) / A.zoom + A.cam.x, (A.my - 360) / A.zoom + A.cam.y);
}
static void camera(V target, float dt) {
    float hw = 640 / A.zoom, hh = 360 / A.zoom;
    target.x = clampf(target.x, hw, WW - hw);
    target.y = clampf(target.y, hh, WH - hh);
    A.cam = add(A.cam, mul(sub(target, A.cam), dt < 0 ? 1 : 1 - expf(-dt * 8)));
}
static float unitSize(const Unit *u) {
    return (float[]){88, 47, 48, 57, 115, 151, 88}[u->kind];
}
static int unitArt(const Unit *u) {
    return u->kind == HERO ? u->role : (int[]){0, 7, 8, 9, 5, 6, 10}[u->kind];
}
static int target_under(void) {
    int best = -1;
    float score = 1e8f;
    for (int i = 0; i < MAXU; i++) {
        Unit *u = &G.u[i];
        if (!alive(u) || u->team == 0 || (!visible(&G, 0, u->pos) && u->kind != TOWER && u->kind != CORE))
            continue;
        V p = world(u->pos);
        float s = unitSize(u) * A.zoom;
        float dx = A.mx - p.x, dy = A.my - (p.y - s * .43f);
        if (fabsf(dx) > s * .45f || fabsf(dy) > s * .55f)
            continue;
        float d = dx * dx + dy * dy + (u->kind == HERO ? 0 : 240);
        if (d < score) {
            best = i;
            score = d;
        }
    }
    return best;
}
static void world_fx(int layer) {
    float z = A.zoom;
    for (int i = 0; i < MAXFX; i++) {
        Fx *f = &G.fx[i];
        if (!f->live || !visible(&G, 0, f->a) || (layer == 0) != (f->kind == FX_ZONE || f->kind == FX_HEAL))
            continue;
        V a = world(f->a), b = world(f->b);
        float k = clampf(1 - f->life / f->total, 0, 1), alpha = 1 - k, s = f->size * z;
        uint32_t c = f->color;
        if (a.x < -300 || a.y < -300 || a.x > 1580 || a.y > 1020)
            continue;
        if (f->kind == FX_ZONE) {
            if (f->n == 1) {
                for (int j = 0; j < 15; j++) {
                    float q = fmodf(k * 1.5f + j * .137f, 1);
                    float xx = a.x + cosf(j * 2.4f) * s * .78f + (1 - q) * 50 * z,
                          yy = a.y + sinf(j * 2.4f) * s * .65f - (1 - q) * 95 * z;
                    line(xx + 7 * z, yy - 15 * z, xx, yy, c, .5f + .4f * q, 2 * z);
                }
            } else if (f->n == 2) {
                float xx = a.x + (1 - k) * 100 * z, yy = a.y - (1 - k) * 205 * z;
                line(xx + 26 * z, yy - 48 * z, xx, yy, c, .75f, 9 * z);
                diamond(xx, yy, (10 + 6 * k) * z, PAPER, .94f);
                oval(a.x, a.y, s * .3f * k, s * .2f * k, c, .23f, 1, 0);
            } else if (f->n == 3) {
                for (int j = 0; j < 5; j++) {
                    float t = j * 1.256637f + k, tt = t + 2.513274f;
                    line(a.x + cosf(t) * s * .8f, a.y + sinf(t) * s * .68f, a.x + cosf(tt) * s * .8f,
                         a.y + sinf(tt) * s * .68f, c, .55f, 1.7f * z);
                }
            }
            oval(a.x, a.y, s, s * .88f, c, .11f + .15f * k, 1, 0);
            oval(a.x, a.y, s, s * .88f, c, .8f, 0, 2);
            oval(a.x, a.y, s * (1 - k), s * .88f * (1 - k), c, .9f, 0, 2);
            for (int j = 0; j < 4; j++) {
                float a1 = j * 1.570796f;
                diamond(a.x + cosf(a1) * s, a.y + sinf(a1) * s * .88f, 4, c, .9f);
            }
        } else if (f->kind == FX_NUMBER) {
            fmt(a.x - 51, a.y - 30 - k * 34, 102, 27, f->size, 0x202720, F_BOLD, 1, L"%d", f->n);
            fmt(a.x - 52, a.y - 32 - k * 34, 102, 27, f->size, c, F_BOLD, 1, L"%d", f->n);
        } else if (f->kind == FX_LINE) {
            line(a.x, a.y - 16 * z, b.x, b.y - 16 * z, c, alpha * .35f, s);
            line(a.x, a.y - 16 * z, b.x, b.y - 16 * z, PAPER, alpha, 2.3f * z);
        } else if (f->kind == FX_SLASH) {
            V dir = norm(sub(b, a));
            float angle = atan2f(dir.y, dir.x);
            V last = {0};
            for (int j = 0; j <= 8; j++) {
                float th = angle - .97f + j * .245f + k * .24f;
                V p = vec(a.x + cosf(th) * s, a.y - 14 * z + sinf(th) * s * .82f);
                if (j)
                    line(last.x, last.y, p.x, p.y, c, alpha, (j < 5 ? j + 1 : 10 - j) * z);
                last = p;
            }
        } else {
            float rr = s * (.12f + .88f * k);
            oval(a.x, a.y - 5, rr, rr * .7f, c, alpha, .0f, 3 * z);
            if (f->kind == FX_BURST) {
                for (int j = 0; j < 8; j++) {
                    float t = j * .785398f;
                    line(a.x + cosf(t) * rr * .52f, a.y - 10 + sinf(t) * rr * .65f, a.x + cosf(t) * rr,
                         a.y - 10 + sinf(t) * rr * .96f, c, alpha, 2 * z);
                }
            }
        }
    }
}
static void battlefield(void) {
    float z = A.zoom;
    V org = world(vec(0, 0));
    art(ART_MAP, org.x, org.y, WW * z, WH * z, 0, 0, 1464, 1074, 1, 0);
    box(0, 0, 1280, 720, 0x10282a, .10f);
    shroud(org.x, org.y, WW * z, WH * z, G.sight[0], GW, GH, G.sightFrame);
    int order[MAXU], n = 0;
    for (int i = 0; i < MAXU; i++) {
        Unit *u = &G.u[i];
        if (!u->used || (u->team != 0 && u->kind != CORE && u->kind != TOWER && !visible(&G, 0, u->pos)))
            continue;
        V p = world(u->pos);
        if (p.x < -150 || p.y < -50 || p.x > 1430 || p.y > 940)
            continue;
        if (!alive(u)) {
            if (u->kind == TOWER) {
                oval(p.x, p.y, 29 * z, 16 * z, 0x302d29, .72f, 1, 0);
                for (int k = 0; k < 5; k++)
                    diamond(p.x + cosf(k * 3.f) * 17 * z, p.y + sinf(k * 3.f) * 10 * z, 7 * z, 0x7f8070,
                            .98f);
            }
            continue;
        }
        if (u->kind == TOWER || u->kind == CORE) {
            int danger = u->team == 1 && dist(P.pos, u->pos) < u->reach + 125;
            if (danger || A.hoverUnit == i) {
                uint32_t c = teamColor[u->team];
                int lockPlayer = u->target == 0 && u->targetSerial == P.serial;
                oval(p.x, p.y, u->reach * z, u->reach * z, c, .042f, 1, 0);
                oval(p.x, p.y, u->reach * z, u->reach * z, c, lockPlayer ? .93f : .48f, 0,
                     lockPlayer ? 2.8f : 1.3f);
                if (lockPlayer)
                    line(p.x, p.y - 46 * z, world(P.pos).x, world(P.pos).y - 20 * z, RED, .72f, 1.6f);
            }
        }
        int j = n;
        while (j > 0 && G.u[order[j - 1]].pos.y > u->pos.y) {
            order[j] = order[j - 1];
            j--;
        }
        order[j] = i;
        n++;
    }
    world_fx(0);
    if (G.rallyTime > 0) {
        V p = world(G.rally);
        float k = fmodf(G.time, 1);
        oval(p.x, p.y, (22 + 25 * k) * z, (12 + 14 * k) * z, GOLD, 1 - k, 0, 3);
        line(p.x, p.y, p.x, p.y - 53 * z, GOLD, .96f, 2);
        float flag[] = {p.x, p.y - 53 * z, p.x + 28 * z, p.y - 46 * z, p.x, p.y - 37 * z};
        poly(flag, 3, GOLD, 1);
    }
    if (G.markTime > 0) {
        V p = world(G.mark);
        float k = 1 - G.markTime / .8f;
        oval(p.x, p.y, (12 + 14 * k) * z, (7 + 7 * k) * z, GOLD, 1 - k * .7f, 0, 2);
        line(p.x - 6, p.y, p.x, p.y + 4, GOLD, 1, 2);
        line(p.x, p.y + 4, p.x + 7, p.y - 3, GOLD, 1, 2);
    }
    for (int j = 0; j < n; j++) {
        int id = order[j];
        Unit *u = &G.u[id];
        V p = world(u->pos);
        float sz = unitSize(u) * z;
        uint32_t c = teamColor[u->team];
        int hero = u->kind == HERO, building = u->kind == CORE || u->kind == TOWER;
        oval(p.x, p.y, sz * .30f, sz * .11f, 0x14211c, .52f, 1, 0);
        if (hero || building || A.hoverUnit == id || u->kind == GOLEM) {
            oval(p.x, p.y, sz * .40f, sz * .157f, id == 0 ? GOLD : c, hero ? .98f : .76f, 0, id == 0 ? 3 : 2);
        } else {
            oval(p.x, p.y, sz * .31f, sz * .12f, c, .84f, 0, 1.7f);
        }
        if (id == 0) {
            float q[] = {p.x - 4, p.y + sz * .23f, p.x, p.y + sz * .29f, p.x + 4, p.y + sz * .23f};
            poly(q, 3, GOLD, 1);
        }
        if (u->recall > 0) {
            float a = 1 - u->recall / 4;
            oval(p.x, p.y, sz * (.35f + a * .22f), sz * .23f, JADE, 1, 0, 3);
            line(p.x - 16, p.y, p.x - 16, p.y - 75, JADE, .5f, 2);
            line(p.x + 16, p.y, p.x + 16, p.y - 75, JADE, .5f, 2);
        }
        float bob = u->moving > 0 && !building ? fabsf(sinf(G.time * 9 + id)) * 3.6f * z : 0,
              lunge = u->anim > 0 && !building ? sinf(clampf(u->anim / .3f, 0, 1) * 3.14159f) * 6 * z : 0;
        float opacity = 1;
        if (building && alive(&P) && P.pos.y <= u->pos.y && u->pos.y - P.pos.y < unitSize(u) * .8f &&
            fabsf(P.pos.x - u->pos.x) < unitSize(u) * .43f)
            opacity = .47f;
        sprite(u->team == 1 ? (u->kind == CORE ? 11 : 12 + unitArt(u)) : unitArt(u), p.x + u->face.x * lunge,
               p.y - bob + u->face.y * lunge, sz, opacity, !building && u->dir < 0);
        if (u->shield > 4 || u->grace > 0) {
            oval(p.x, p.y - sz * .42f, sz * .43f, sz * .49f, u->grace > 0 ? PAPER : GOLD, .58f, 0, 2);
        }
        if (A.hoverUnit == id) {
            float q[] = {p.x - 6, p.y - sz - 23, p.x + 6, p.y - sz - 23, p.x, p.y - sz - 16};
            poly(q, 3, RED, 1);
        }
        if (u->stun > 0)
            txt(L"✦", p.x - 20, p.y - sz - 27, 40, 20, 17, GOLD, F_BOLD, 1);
        float bw = (hero               ? 54
                    : building         ? 76
                    : u->kind == GOLEM ? 45
                                       : 26) *
                   z,
              bh = (hero || building ? 5 : 3) * z, by = p.y - sz * .96f - 4;
        box(p.x - bw * .5f - 1, by - 1, bw + 2, bh + 2, DEEP, .97f);
        box(p.x - bw * .5f, by, bw * clampf(u->hp / u->maxhp, 0, 1), bh, id == 0 ? 0xb8ef86 : c, 1);
        if (hero) {
            if (id == 0 || A.hoverUnit == id) {
                txt(id == 0 ? L"你" : heroes[u->role].name, p.x - 49, by - 20, 98, 18, 12, PAPER, F_BOLD, 1);
            }
            box(p.x - bw * .5f - 14, by - 6, 14, 15, DEEP, .95f);
            fmt(p.x - bw * .5f - 14, by - 6, 14, 15, 10, id == 0 ? GOLD : c, F_NUM, 1, L"%d", u->level);
        }
        if (building && protected_building(&G, id)) {
            glyph(6, p.x - bw * .5f - 9, by + 2, 12, GOLD);
        }
    }
    for (int i = 0; i < MAXP; i++) {
        Shot *s = &G.shot[i];
        if (!s->live || s->kind == 3 || !visible(&G, 0, s->p))
            continue;
        V p = world(s->p);
        p.y -= 17 * z;
        V v = norm(s->v);
        float sz = (s->kind == 0 ? s->radius : s->radius > 40 ? 16 : 8) * z;
        line(p.x - v.x * sz * 4, p.y - v.y * sz * 4, p.x, p.y, s->color, .57f, sz * 1.23f);
        oval(p.x, p.y, sz, sz, s->color, .93f, 1, 0);
        oval(p.x, p.y, sz * .42f, sz * .42f, PAPER, 1, 1, 0);
    }
    world_fx(1);
}
enum { MAPX = 25, MAPY = 25, MAPW = 174, MAPH = 128 };
static void minimap(void) {
    float x = MAPX, y = MAPY, w = MAPW, h = MAPH;
    box(x - 5, y - 5, w + 10, h + 10, DEEP, .96f);
    cover(ART_MAP, x, y, w, h, .5f, 1);
    box(x, y, w, h, DEEP, .37f);
    shroud(x, y, w, h, G.sight[0], GW, GH, G.sightFrame);
    frame(x - 1, y - 1, w + 2, h + 2, GOLD, .65f, 1);
    for (int phase = 0; phase < 2; phase++)
        for (int i = phase ? 0 : 10; i < (phase ? 10 : MAXU); i++) {
            Unit *u = &G.u[i];
            if (!alive(u) || (u->team != 0 && u->kind != CORE && u->kind != TOWER && !visible(&G, 0, u->pos)))
                continue;
            float xx = x + u->pos.x / WW * w, yy = y + u->pos.y / WH * h;
            uint32_t c = teamColor[u->team];
            if (i < 10) {
                oval(xx, yy, i == 0 ? 4.4f : 3.2f, i == 0 ? 4.4f : 3.2f, DEEP, 1, 1, 0);
                oval(xx, yy, i == 0 ? 3.5f : 2.6f, i == 0 ? 3.5f : 2.6f, i == 0 ? PAPER : c, 1, 1, 0);
            } else if (u->kind == TOWER || u->kind == CORE)
                diamond(xx, yy, u->kind == CORE ? 4 : 2.8f, c, 1);
            else
                box(xx - 1, yy - 1, 2, 2, c, .9f);
        }
    if (G.rallyTime > 0) {
        float xx = x + G.rally.x / WW * w, yy = y + G.rally.y / WH * h;
        diamond(xx, yy, 4, GOLD, 1);
        oval(xx, yy, 5 + fmodf(G.time, 1) * 5, 5 + fmodf(G.time, 1) * 5, GOLD, .78f, 0, 1);
    }
    frame(x + (A.cam.x - 640 / A.zoom) / WW * w, y + (A.cam.y - 360 / A.zoom) / WH * h,
          1280 / A.zoom / WW * w, 720 / A.zoom / WH * h, PAPER, .82f, 1);
    box(x - 5, 158, w + 10, 22, DEEP, .91f);
    txt(L"岚境峡谷   /   战术地图", x, 158, w, 21, 11, MUTED, F_UI, 1);
}
static void panel(float x, float y, float w, float h) {
    box(x, y, w, h, DEEP, .94f);
    frame(x + .5f, y + .5f, w - 1, h - 1, GOLD, .4f, 1);
}
static void hud(void) {
    Unit *u = &P;
    const HeroDef *h = &heroes[u->role];
    minimap();
    panel(477, 15, 326, 54);
    fmt(485, 16, 79, 47, 30, JADE, F_NUM, 1, L"%02d", G.kills[0]);
    fmt(718, 16, 77, 47, 30, RED, F_NUM, 1, L"%02d", G.kills[1]);
    line(568, 28, 568, 55, GOLD, .36f, 1);
    line(712, 28, 712, 55, GOLD, .36f, 1);
    fmt(570, 18, 140, 28, 23, PAPER, F_NUM, 1, L"%02d:%02d", (int)G.time / 60, (int)G.time % 60);
    txt(L"峡 谷 对 决", 570, 45, 140, 19, 10, GOLD, F_UI, 1);
    panel(1032, 21, 152, 39);
    diamond(1051, 40, 6, GOLD, 1);
    fmt(1065, 22, 106, 36, 16, GOLD, F_BOLD, 0, L"%d   商店 P", u->gold);
    panel(1193, 21, 59, 39);
    txt(L"Ⅱ", 1193, 20, 59, 38, 20, PAPER, F_BOLD, 1);
    for (int i = 0; i < 3; i++) {
        Feed *f = &G.feed[i];
        if (!f->msg[0] || G.time - f->time > 7)
            continue;
        float y = 74 + i * 27;
        box(1020, y, 232, 24, DEEP, .79f);
        box(1020, y, 3, 24, f->color, 1);
        txt(f->msg, 1028, y, 216, 24, 13, PAPER, F_UI, 2);
    }
    /* Personal status and persistent controls. */
    gradient(0, 572, 1280, 148, DEEP, 0, DEEP, .84f, 1);
    panel(24, 616, 304, 81);
    portrait(u->role, 30, 622, 65, 69, .23f, 1);
    frame(30, 622, 65, 69, h->color, .83f, 1);
    fmt(106, 620, 204, 26, 17, PAPER, F_BOLD, 0, L"%ls   Lv.%d", h->name, u->level);
    box(106, 652, 199, 8, 0x334443, 1);
    box(106, 652, 199 * clampf(u->hp / u->maxhp, 0, 1), 8, JADE, 1);
    box(106, 664, 199, 3, 0x334443, 1);
    box(106, 664, 199 * clampf((float)u->xp / xp_next(u), 0, 1), 3, GOLD, 1);
    fmt(106, 671, 204, 21, 12, MUTED, F_NUM, 0, L"HP  %d / %d     KDA  %d/%d/%d", (int)fmaxf(0, u->hp),
        (int)u->maxhp, u->kills, u->deaths, u->assists);
    for (int i = 0; i < 4; i++) {
        float x = 455 + i * 94;
        uint32_t c = i == 3 ? JADE : h->color;
        float cd = i < 3 ? u->cd[i] : u->recall;
        int hot = hit(x, 612, 85, 86);
        panel(x, 612, 85, 85);
        if (hot)
            box(x + 1, 613, 83, 83, c, .14f);
        box(x + 1, 613, 83, 2, c, 1);
        if (i == 3)
            glyph(7, x + 43, 641, 32, c);
        else
            skillart(u->role, i, x + 3, 616, 79, 44, cd > .04f ? .64f : 1.f);
        if (cd > .04f) {
            box(x + 2, 615, 81, 46, DEEP, .44f);
            fmt(x, 617, 85, 43, 24, PAPER, F_NUM, 1, L"%.1f", cd);
        }
        txt(i == 3 ? L"回城" : h->skill[i], x, 658, 85, 23, 14, PAPER, F_BOLD, 1);
        wchar_t key[] = {L"QERB"[i], 0};
        txt(key, x, 679, 85, 16, 12, GOLD, F_NUM, 1);
        if (hot && A.overlay == NONE) {
            panel(413, 550, 433, 47);
            txt(i == 3 ? L"引导 4 秒后返回泉水，受击或行动会中断。" : h->desc[i], 423, 551, 414, 45, 14,
                PAPER, F_UI, 1);
        }
    }
    txt(L"SPACE  普攻     G  集结     TAB  战绩", 880, 633, 369, 26, 15, PAPER, F_UI, 2);
    txt(L"滚轮  缩放     F1  指南     ESC  暂停", 884, 664, 365, 24, 13, MUTED, F_UI, 2);
    if (G.time < 19) {
        panel(20, 190, 184, 82);
        fmt(30, 195, 165, 23, 14, GOLD, F_BOLD, 0, L"%ls  ·  推荐%ls路", h->role,
            (const wchar_t *[]){L"上", L"中", L"下"}[u->lane]);
        txt(L"WASD 移动  /  空格普攻", 30, 222, 165, 20, 12, PAPER, F_UI, 0);
        txt(L"鼠标瞄准后按 Q、E、R", 30, 244, 165, 20, 12, MUTED, F_UI, 0);
    }
    if (G.time < 17 && G.noticeTime <= 0) {
        panel(401, 82, 478, 40);
        txt(L"随兵线推进  ·  先拆同路外塔、内塔，再攻水晶", 408, 82, 464, 40, 15, GOLD, F_BOLD, 1);
    }
    if (G.noticeTime > 0) {
        panel(402, 83, 476, 39);
        txt(G.notice, 409, 83, 462, 39, 15, GOLD, F_BOLD, 1);
    }
    if (!alive(u)) {
        panel(489, 464, 302, 66);
        txt(L"等待重生", 493, 466, 294, 28, 18, PAPER, F_SERIF, 1);
        fmt(493, 494, 294, 32, 19, GOLD, F_NUM, 1, L"%.1f s", fmaxf(0, u->respawn));
    } else if (u->recall > 0) {
        panel(505, 511, 270, 38);
        fmt(507, 511, 266, 30, 15, JADE, F_BOLD, 1, L"回城中   %.1f", u->recall);
        box(512, 542, 256 * (1 - u->recall / 4), 3, JADE, 1);
    }
    if (G.rallyCd > 0)
        fmt(964, 601, 285, 25, 13, GOLD, F_UI, 2, L"集结冷却  %.0f s", G.rallyCd);
    if (u->buff > 0)
        fmt(25, 580, 290, 28, 14, GOLD, F_BOLD, 0, L"山灵祝福   %.0f s", u->buff);
}
static void modal(float x, float y, float w, float h, const wchar_t *eyebrow, const wchar_t *title) {
    box(0, 0, 1280, 720, DEEP, .73f);
    panel(x, y, w, h);
    box(x, y, 52, 3, GOLD, 1);
    txt(eyebrow, x + 31, y + 23, w - 64, 20, 12, GOLD, F_NUM, 0);
    txt(title, x + 29, y + 46, w - 58, 57, 36, PAPER, F_SERIF, 0);
}
static void guide(void) {
    modal(194, 66, 892, 586, L"F I E L D   M A N U A L", L"战场指南");
    const wchar_t *keys[] = {L"W A S D", L"鼠标右键", L"左键 / 空格", L"Q  E  R", L"B  /  P", L"Tab / Esc"};
    const wchar_t *act[] = {
        L"移动英雄；键盘移动优先于寻路指令。",        L"点击地面自动寻路；点击敌人追击并普攻。",
        L"左键下达攻击移动；按住空格主动索敌。",      L"朝鼠标指针瞬发技能；点击技能格可先瞄准。",
        L"回泉水补血 / 打开装备商店（商店内暂停）。", L"按住查看双方战绩 / 暂停或关闭当前面板。"};
    for (int i = 0; i < 6; i++) {
        float y = 186 + i * 39;
        box(226, y, 134, 30, 0x263a38, 1);
        txt(keys[i], 228, y, 130, 30, 14, GOLD, F_BOLD, 1);
        txt(act[i], 376, y, 675, 30, 15, PAPER, F_UI, 0);
    }
    rule(226, 431, 824);
    txt(L"推塔规则", 227, 443, 145, 28, 18, JADE, F_BOLD, 0);
    txt(L"跟随小兵进塔，依次拆掉同一路外塔、内塔，即可解除敌方水晶封印。", 227, 478, 822, 28, 15, PAPER, F_UI,
        0);
    txt(L"塔优先攻击小兵；在塔下攻击敌方英雄会引来塔火。击败山灵可获得金币和攻击祝福。", 227, 509, 825, 29,
        14, MUTED, F_UI, 0);
    txt(L"右键小地图可寻路  ·  G 朝指针集结队友  ·  滚轮缩放  ·  F11 全屏  ·  M 静音", 227, 549, 833, 28, 13,
        MUTED, F_UI, 0);
    button(846, 594, 204, 38, L"明白了    ↵", 1);
}
static void shop(void) {
    Unit *u = &P;
    modal(206, 111, 868, 495, L"Q U A R T E R M A S T E R", L"行军商店");
    fmt(732, 157, 302, 37, 18, GOLD, F_BOLD, 2, L"持有金币   %d", u->gold);
    for (int i = 0; i < 3; i++) {
        float x = 239 + i * 268;
        int max = u->items[i] >= 4, cost = item_cost(u, i), can = !max && u->gold >= cost;
        panel(x, 239, 247, 223);
        box(x, 239, 247, 3, i == 0 ? RED : i == 1 ? GOLD : JADE, 1);
        glyph((int[]){0, 6, 4}[i], x + 33, 280, 40, GOLD);
        fmt(x + 60, 257, 171, 44, 25, PAPER, F_SERIF, 0, L"%ls", itemNames[i]);
        fmt(x + 16, 305, 218, 30, 14, MUTED, F_UI, 0, L"品阶   %d / 4", u->items[i]);
        txt(itemDesc[i], x + 15, 341, 220, 36, 13, PAPER, F_UI, 0);
        wchar_t b[48];
        if (max)
            swprintf(b, 48, L"已满阶");
        else
            swprintf(b, 48, L"%d   金币%s", cost, can ? L" · 购买" : L" · 不足");
        button(x + 15, 395, 217, 45, b, can);
    }
    txt(L"装备立即生效；每件可提升四阶。人机也会使用金币购装。", 239, 478, 780, 31, 15, MUTED, F_UI, 0);
    if (G.noticeTime > 0)
        txt(G.notice, 239, 514, 553, 32, 14, GOLD, F_UI, 0);
    button(855, 540, 183, 42, L"返回战场   P", 1);
}
static void pause_ui(void) {
    modal(419, 132, 442, 456, L"T A C T I C A L   P A U S E", L"战局已暂停");
    button(453, 260, 374, 48, L"继续战斗    Esc", 1);
    button(453, 324, 179, 44, L"战场指南   F1", 0);
    button(646, 324, 181, 44, A.muted ? L"打开声音   M" : L"关闭声音   M", 0);
    button(453, 384, 374, 44, L"重新选择英雄", 0);
    button(453, 444, 374, 44, L"返回主界面", 0);
    txt(L"暂停时所有单位和技能计时都会冻结。", 451, 524, 378, 32, 14, MUTED, F_UI, 1);
}
static void confirm_leave(void) {
    modal(407, 186, 466, 314, L"L E A V E   M A T C H", L"结束当前对局？");
    txt(L"本局进度不会保留。", 441, 302, 394, 35, 17, MUTED, F_UI, 0);
    button(442, 396, 178, 49, L"继续对局", 0);
    button(637, 396, 198, 49, L"确认离开", 1);
}
static void scoreboard(void) {
    modal(100, 68, 1080, 584, L"M A T C H   S T A T I S T I C S", L"对局战绩");
    fmt(709, 112, 435, 44, 16, MUTED, F_UI, 2, L"对局时间  %02d:%02d     /     第 %d 波兵线",
        (int)G.time / 60, (int)G.time % 60, G.waves);
    for (int t = 0; t < 2; t++) {
        float x = 132 + t * 523;
        uint32_t c = teamColor[t];
        fmt(x, 184, 494, 38, 21, c, F_BOLD, 0, L"%ls   %d 击败  ·  %d 推塔", t ? L"敌方" : L"我方",
            G.kills[t], G.towers[t]);
        rule(x, 232, 491);
        txt(L"英雄 / 等级", x + 8, 238, 185, 22, 12, MUTED, F_UI, 0);
        txt(L"击败 / 阵亡 / 助攻", x + 190, 238, 153, 22, 12, MUTED, F_UI, 1);
        txt(L"补刀", x + 390, 238, 84, 22, 12, MUTED, F_UI, 2);
        for (int i = 0; i < 5; i++) {
            int id = t * 5 + i;
            Unit *u = &G.u[id];
            float y = 270 + i * 58;
            box(x, y, 490, 51, id == 0 ? 0x264541 : 0x1b2c2e, .95f);
            portrait(u->role, x + 4, y + 4, 43, 43, .24f, alive(u) ? 1 : .4f);
            fmt(x + 57, y + 2, 165, 27, 15, PAPER, F_BOLD, 0, L"%ls  %ls", heroes[u->role].name,
                id ? L"" : L"· 你");
            fmt(x + 57, y + 27, 160, 20, 12, MUTED, F_UI, 0, L"Lv.%d   %ls", u->level, id ? L"AI" : L"玩家");
            fmt(x + 185, y + 2, 161, 47, 18, PAPER, F_NUM, 1, L"%d  /  %d  /  %d", u->kills, u->deaths,
                u->assists);
            fmt(x + 390, y + 2, 84, 47, 18, c, F_NUM, 2, L"%d", u->cs);
        }
    }
    txt(L"松开 Tab 返回战场", 123, 592, 1033, 30, 14, GOLD, F_UI, 1);
}
static void result(void) {
    int won = G.winner == 0;
    box(0, 0, 1280, 720, DEEP, .78f);
    modal(312, 97, 656, 521,
          won ? L"V I C T O R Y    /    A F T E R   A C T I O N"
              : L"D E F E A T    /    A F T E R   A C T I O N",
          won ? L"敌方水晶已破" : L"我方水晶失守");
    txt(won ? L"胜  利" : L"战  败", 341, 209, 596, 99, 70, won ? GOLD : RED, F_SERIF, 1);
    fmt(332, 314, 616, 37, 18, PAPER, F_UI, 1, L"%ls  ·  %ls难度  ·  用时 %02d:%02d", heroes[P.role].name,
        diffs[A.difficulty], (int)G.time / 60, (int)G.time % 60);
    rule(352, 371, 576);
    fmt(347, 387, 586, 34, 19, PAPER, F_BOLD, 1, L"个人战绩    %d / %d / %d       补刀    %d", P.kills,
        P.deaths, P.assists, P.cs);
    fmt(347, 430, 586, 29, 15, MUTED, F_UI, 1, L"双方击败   %d : %d      我方推塔   %d / 6", G.kills[0],
        G.kills[1], G.towers[0]);
    button(358, 516, 208, 54, L"返回主界面", 0);
    button(590, 516, 332, 54, L"再来一局 · 重新选将", 1);
}
/* Mouse-click casting has an explicit aiming phase; keyboard casting is immediate. */
static void aim_overlay(void) {
    int r = P.role, i = A.pending;
    V pos = world(P.pos), m = mouseworld(), dir = norm(sub(m, P.pos));
    float z = A.zoom;
    float range = 0, radius = 0;
    if (r == 0) {
        range = i == 0 ? 156 : i == 2 ? 440 : 0;
        radius = i == 1 ? 160 : 0;
    }
    if (r == 1) {
        range = i == 0 ? 548 : i == 1 ? 130 : 475;
        radius = i == 2 ? 153 : 0;
    }
    if (r == 2) {
        range = i == 0 ? 479 : 475;
        radius = i == 1 ? 112 : i == 2 ? 170 : 0;
    }
    if (r == 3) {
        range = i == 2 ? 232 : 0;
        radius = i == 0 ? 143 : i == 1 ? 242 : 162;
    }
    if (r == 4) {
        range = i == 0 ? 198 : i == 1 ? 547 : 295;
        radius = i == 2 ? 128 : 0;
    }
    V target = world(add(P.pos, mul(dir, fminf(range, dist(m, P.pos)))));
    uint32_t c = heroes[r].color;
    if (range > 0)
        oval(pos.x, pos.y, range * z, range * z, c, .37f, 0, 1);
    if (radius > 0) {
        oval(target.x, target.y, radius * z, radius * z, c, .14f, 1, 0);
        oval(target.x, target.y, radius * z, radius * z, c, .85f, 0, 2);
    }
    line(pos.x, pos.y, target.x, target.y, c, .9f, 2);
    diamond(target.x, target.y, 5, PAPER, .95f);
}
void ui_draw(void) {
    draw_begin();
    if (A.screen == HOME)
        home();
    else if (A.screen == DRAFT)
        draft();
    else {
        battlefield();
        if (A.pending >= 0)
            aim_overlay();
        hud();
    }
    if (A.overlay == GUIDE)
        guide();
    else if (A.overlay == SHOP)
        shop();
    else if (A.overlay == PAUSED)
        pause_ui();
    else if (A.overlay == LEAVE)
        confirm_leave();
    else if (A.overlay == SCORE)
        scoreboard();
    else if (A.screen == MATCH && G.winner >= 0)
        result();
}
void ui_reset(void) {
    memset(&A, 0, sizeof A);
    A.screen = HOME;
    A.selected = 0;
    A.difficulty = 1;
    A.zoom = .95f;
    A.hoverUnit = A.pending = -1;
}
void ui_start(uint32_t seed) {
    game_init(&G, A.selected, A.difficulty, seed);
    A.screen = MATCH;
    A.overlay = NONE;
    A.hoverUnit = A.pending = -1;
    A.zoom = .95f;
    memset(A.held, 0, sizeof A.held);
    camera(P.pos, -1);
    audio_event(S_UI);
}
static void mute_toggle(void) {
    A.muted = !A.muted;
    audio_mute(A.muted);
}
static void showguide(void) {
    A.guideBack = A.overlay;
    A.overlay = GUIDE;
}
static void cast(int s) {
    if (!game_cast(&G, 0, s, mouseworld()) && alive(&P)) {
        if (P.stun > 0)
            game_notice(&G, L"受到控制，暂时无法施法");
        else
            game_notice(&G, L"技能尚在冷却");
    }
    A.pending = -1;
}
void ui_click(int right) {
    if (A.mx < 0 || A.my < 0 || A.mx >= 1280 || A.my >= 720)
        return;
    if (A.overlay == GUIDE) {
        if (!right && hit(846, 594, 204, 38))
            A.overlay = A.guideBack;
        return;
    }
    if (A.overlay == SHOP) {
        if (right)
            return;
        if (hit(855, 540, 183, 42)) {
            A.overlay = NONE;
            return;
        }
        for (int i = 0; i < 3; i++)
            if (hit(254 + i * 268, 395, 217, 45)) {
                if (game_buy(&G, 0, i))
                    game_notice(&G, L"购买成功 · 装备属性已生效");
                else
                    game_notice(&G, P.items[i] >= 4 ? L"此装备已达满阶" : L"金币不足");
            }
        return;
    }
    if (A.overlay == PAUSED) {
        if (right)
            return;
        if (hit(453, 260, 374, 48))
            A.overlay = NONE;
        else if (hit(453, 324, 179, 44))
            showguide();
        else if (hit(646, 324, 181, 44))
            mute_toggle();
        else if (hit(453, 384, 374, 44) || hit(453, 444, 374, 44)) {
            A.view = A.my < 440 ? DRAFT : HOME;
            A.overlay = LEAVE;
        }
        return;
    }
    if (A.overlay == LEAVE) {
        if (!right && hit(442, 396, 178, 49))
            A.overlay = NONE;
        else if (!right && hit(637, 396, 198, 49)) {
            A.overlay = NONE;
            A.screen = A.view;
        }
        return;
    }
    if (A.overlay != NONE)
        return;
    if (A.screen != MATCH && !right) {
        if (hit(1036, 33, 90, 23)) {
            mute_toggle();
            return;
        }
        if (hit(1137, 33, 90, 23)) {
            A.requestFull = 1;
            return;
        }
    }
    if (A.screen == HOME) {
        if (right)
            return;
        if (hit(69, 510, 213, 52)) {
            A.screen = DRAFT;
            audio_event(S_UI);
        } else if (hit(296, 510, 146, 52))
            showguide();
        return;
    }
    if (A.screen == DRAFT) {
        if (right)
            return;
        for (int i = 0; i < 5; i++)
            if (hit(61 + i * 234, 191, 222, 280)) {
                A.selected = i;
                audio_event(S_UI);
                return;
            }
        for (int i = 0; i < 3; i++)
            if (hit(925 + i * 96, 493, 87, 35)) {
                A.difficulty = i;
                audio_event(S_UI);
                return;
            }
        if (hit(62, 621, 133, 48))
            A.screen = HOME;
        else if (hit(977, 615, 243, 59))
            ui_start(GetTickCount());
        return;
    }
    if (G.winner >= 0) {
        if (!right && hit(358, 516, 208, 54))
            A.screen = HOME;
        else if (!right && hit(590, 516, 332, 54))
            A.screen = DRAFT;
        return;
    }
    if (A.pending >= 0) {
        if (right)
            A.pending = -1;
        else
            cast(A.pending);
        return;
    }
    if (!right && hit(1032, 21, 152, 39)) {
        A.overlay = SHOP;
        return;
    }
    if (!right && hit(1193, 21, 59, 39)) {
        A.overlay = PAUSED;
        return;
    }
    for (int i = 0; i < 4; i++)
        if (hit(455 + i * 94, 612, 85, 85)) {
            if (!right && alive(&P)) {
                if (i == 3)
                    game_recall(&G);
                else if (P.cd[i] <= 0) {
                    A.pending = i;
                    game_notice(&G, L"选择施法位置 · 左键释放 / 右键取消");
                } else
                    game_notice(&G, L"技能尚在冷却");
            }
            return;
        }
    if (hit(MAPX, MAPY, MAPW, MAPH)) {
        game_order(&G, vec((A.mx - MAPX) / MAPW * WW, (A.my - MAPY) / MAPH * WH), -1, !right);
        return;
    }
    if (hit(24, 616, 304, 81) || hit(477, 15, 326, 54) || hit(20, 158, 184, 22))
        return;
    game_order(&G, mouseworld(), target_under(), !right);
}
void ui_key(int k, int down) {
    if (k < 0 || k > 255)
        return;
    int was = A.held[k];
    A.held[k] = down;
    if (!down) {
        if (k == VK_TAB && A.overlay == SCORE)
            A.overlay = NONE;
        return;
    }
    if (was)
        return;
    if (k == VK_F11) {
        A.requestFull = 1;
        return;
    }
    if (k == 'M') {
        mute_toggle();
        return;
    }
    if (A.overlay == GUIDE) {
        if (k == VK_F1 || k == VK_ESCAPE || k == VK_RETURN)
            A.overlay = A.guideBack;
        return;
    }
    if (A.overlay == LEAVE) {
        if (k == VK_ESCAPE)
            A.overlay = PAUSED;
        return;
    }
    if (A.overlay == PAUSED) {
        if (k == VK_ESCAPE)
            A.overlay = NONE;
        else if (k == VK_F1)
            showguide();
        return;
    }
    if (A.overlay == SHOP) {
        if (k == 'P' || k == VK_ESCAPE)
            A.overlay = NONE;
        return;
    }
    if (A.overlay != NONE)
        return;
    if (k == VK_F1) {
        showguide();
        return;
    }
    if (A.screen == HOME) {
        if (k == VK_RETURN) {
            A.screen = DRAFT;
            audio_event(S_UI);
        }
        return;
    }
    if (A.screen == DRAFT) {
        if (k >= '1' && k <= '5')
            A.selected = k - '1';
        else if (k == VK_LEFT)
            A.selected = (A.selected + 4) % 5;
        else if (k == VK_RIGHT)
            A.selected = (A.selected + 1) % 5;
        else if (k == VK_RETURN)
            ui_start(GetTickCount());
        else if (k == VK_ESCAPE)
            A.screen = HOME;
        return;
    }
    if (G.winner >= 0) {
        if (k == VK_RETURN)
            A.screen = DRAFT;
        else if (k == VK_ESCAPE)
            A.screen = HOME;
        return;
    }
    if (k == VK_ESCAPE) {
        if (A.pending >= 0)
            A.pending = -1;
        else
            A.overlay = PAUSED;
    } else if (k == VK_TAB)
        A.overlay = SCORE;
    else if (k == 'P')
        A.overlay = SHOP;
    else if (k == 'B')
        game_recall(&G);
    else if (k == 'G')
        game_rally(&G, hit(MAPX, MAPY, MAPW, MAPH) ? vec((A.mx - MAPX) / MAPW * WW, (A.my - MAPY) / MAPH * WH)
                                                   : mouseworld());
    else if (k == 'Q' || k == 'E' || k == 'R')
        cast(k == 'Q' ? 0 : k == 'E' ? 1 : 2);
}
void ui_wheel(int d) {
    if (A.screen == MATCH && A.overlay == NONE) {
        A.zoom = clampf(A.zoom + d / 120.f * .09f, .68f, 1.35f);
        camera(P.pos, -1);
    }
}
void ui_blur(void) {
    memset(A.held, 0, sizeof A.held);
    A.pending = -1;
    if (A.screen == MATCH && G.winner < 0 && (A.overlay == NONE || A.overlay == SCORE))
        A.overlay = PAUSED;
}
void ui_tick(float dt) {
    A.clock += dt;
    if (A.screen != MATCH || A.overlay != NONE)
        return;
    Input in = {(float)(A.held['D'] - A.held['A']), (float)(A.held['S'] - A.held['W']), A.held[VK_SPACE]};
    game_step(&G, dt, in);
    camera(P.pos, dt);
    A.hoverUnit = target_under();
    for (int i = 0; i < G.soundN; i++)
        audio_event(G.sound[i]);
    G.soundN = 0;
}
