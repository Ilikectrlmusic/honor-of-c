#include "ui.h"
#include "draw.h"
#include "audio.h"
#include "animation.h"
#include "domain.h"
#include "living.h"
#include "presentation.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
App app;
#define A app
#define G app.g
#define P (G.u[0])
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
typedef struct { float x,y,w,h; } Area;
static int inside(Area a){return hit(a.x,a.y,a.w,a.h);}
static const Area homeChoose={69,439,213,52}, homeGuide={296,439,146,52};
static const Area guideDone = {902, 594, 148, 38};
static Area card_area(int i){return (Area){38+(i%5)*164,143+(i/5)*165,156,156};}
static Area difficulty_area(int i){return (Area){143+i*94,563,84,33};}
static const Area reportHome={350,514,155,50}, reportStats={520,514,200,50},
                  reportAgain={735,514,195,50}, statsBack={535,581,210,43};
static void area_button(Area a,const wchar_t *s,int primary){button(a.x,a.y,a.w,a.h,s,primary);}
static void mast(void) {
    crest(65,46,.78f,GOLD);
    txt(L"岚境",90,30,80,32,23,PAPER,F_SERIF,0);
    chip(1036,33,90,A.muted ? L"M  ·  静音" : L"M  ·  声音",A.muted?MUTED:GOLD);
    chip(1137,33,90,L"F11  ·  全屏",GOLD);
}
static void home(void) {
    /* Fixed reconstructed scenery; only the transparent swordsman can deform. */
    float h = 1280.f * 1024 / 1536;
    cover(ART_LANDSCAPE, 0, 0, 1280, 720, .15f, 1);
    living_art(0, 0, -(h - 720) * .15f, 1280, h, A.clock, 1);
    gradient(0, 0, 795, 720, 0x071a1d, .88f, 0x081619, 0, 0);
    gradient(0, 556, 1280, 164, DEEP, 0, DEEP, .88f, 1);
    mast();
    txt(L"岚 境", 57, 199, 560, 143, 113, PAPER, F_SERIF, 0);
    txt(L"五人结阵，三路争锋。", 68, 349, 500, 42, 28, PAPER, F_SERIF, 0);
    button(homeChoose.x, homeChoose.y, homeChoose.w, homeChoose.h, L"选择英雄     →", 1);
    button(homeGuide.x, homeGuide.y, homeGuide.w, homeGuide.h, L"战场指南", 0);
    rule(68, 637, 1143);
    oval(75, 665, 4, 4, JADE, 1, 1, 1);
    txt(L"离线就绪", 88, 650, 102, 30, 13, PAPER, F_BOLD, 0);
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
    if(role>=5){
        float sw=fminf(158,158*w/h),sh=fminf(158,158*h/w);
        art(ART_ROSTER_SKILLS,x,y,w,h,(role-5)*160+(160-sw)/2,slot*160+(160-sh)/2,sw,sh,alpha,0);
        return;
    }
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
static void select_role(int role) {
    if (role != A.selected) {
        A.selected = role;
        A.selectedAt = A.clock;
        audio_event(S_PICK0 + role);
    }
}
static void showcase(int role) {
    const HeroDef *h = &heroes[role];
    float x = 869, y = 84, w = 370, height = 497;
    float age = clampf((A.clock - A.selectedAt) / .28f, 0, 1), settle = (1 - age) * (1 - age);
    float t = A.clock;
    uint32_t color = h->color;
    cursor_clip(x, y, w, height);
    domain_draw(role, x, y, w, height, t);
    float ph = role<5?484:w*664/400, pw=role<5?
          ph * (float[]){634.f / 822, 634.f / 822, 638.f / 822, 759.f / 1024, 771.f / 1024}[role]:w;
    living_art(role<5?role+1:RIG_STORM+role-5, x + (w - pw) / 2 + settle * 7,
               y + (role<5?height-ph:16) + settle * 2, pw, ph, t,
               1 - settle * .12f);
    gradient(x, y + height - 113, w, 113, DEEP, 0, DEEP, .96f, 1);
    box(x + 19, y + height - 63, 24, 2, color, .95f);
    txt(h->name, x + 17, y + height - 57, 155, 48, 31, PAPER, F_SERIF, 0);
    txt(h->tag, x + 164, y + height - 48, w - 184, 33, 14, color, F_BOLD, 2);
    clip_end();
    frame(x + .5f, y + .5f, w - 1, height - 1, GOLD, .38f, 1);
    line(x, y + 24, x, y, GOLD, .90f, 2);
    line(x, y, x + 24, y, GOLD, .90f, 2);
    line(x + w - 24, y + height, x + w, y + height, GOLD, .87f, 2);
    line(x + w, y + height, x + w, y + height - 24, GOLD, .87f, 2);
}
static void draft(void) {
    cover(ART_TITLE, 0, 0, 1280, 720, .3f, .3f);
    box(0, 0, 1280, 720, DEEP, .77f);
    mast();
    txt(L"战前集结", 37, 71, 460, 60, 45, PAPER, F_SERIF, 0);
    const HeroDef *h = &heroes[A.selected];
    showcase(A.selected);
    for (int i = 0; i < HERO_COUNT; i++) {
        Area r = card_area(i);
        float x = r.x, y = r.y, w = r.w, b = y + r.h;
        int selected = i == A.selected, over = inside(r);
        uint32_t c = heroes[i].color;
        /* The original five paintings have a much taller headpiece than the
           new busts: frame their eyes, not a uniform sliver of each forehead. */
        static const float focus[HERO_COUNT]={.35f,.39f,.30f,.23f,.38f,.08f,.08f,.08f,.08f,.08f};
        portrait(i, x, y, w, r.h, focus[i], selected || over ? 1 : .88f);
        gradient(x, b - 56, w, 56, DEEP, 0, DEEP, .96f, 1);
        frame(x, y, w, r.h,
              selected ? GOLD
              : over   ? PAPER
                       : 0x596a66,
              selected ? 1 : .74f, selected ? 2.5f : 1);
        if (selected) {
            box(x, y, w, 3, GOLD, 1);
            chip(x + 5, y + 5, 48, L"已选择", GOLD);
        } else
            fmt(x + 7, y + 3, 35, 19, 12, PAPER, F_NUM, 0, L"%02d", i + 1);
        txt(heroes[i].name, x + 9, b - 35, w - 58, 31, 22, PAPER, F_SERIF, 0);
        txt(heroes[i].role, x + w - 50, b - 28, 40, 23, 13, c, F_BOLD, 2);
    }
    const Area bio={39,471,801,30};
    const float bioSize=19;
    txt(h->bio,bio.x,bio.y,bio.w,bio.h,bioSize,PAPER,F_SERIF,0);
    /* Center between the visible Chinese text and the button edge, not
       between layout rectangles: the vertically centered bio has padding. */
    const float bioInkBottom=ceilf(bio.y+(bio.h+bioSize)*.5f);
    const float skillY=(bioInkBottom+difficulty_area(0).y-40)*.5f;
    for (int j = 0; j < 3; j++) {
        float x = 40 + j * 269;
        skillart(A.selected, j, x, skillY, 40, 40, 1);
        frame(x, skillY, 40, 40, h->color, .75f, 1);
        fmt(x + 51, skillY+4, 195, 30, 15, PAPER, F_BOLD, 0, L"%lc   %ls", L"QER"[j], h->skill[j]);
        if (hit(x, skillY-1, 239, 42)){
            /* Share the difficulty row's unused right side, rather than
               squeezing the hover text between the spells and buttons. */
            float width=text_width(h->desc[j],13,F_UI);
            float size=13*fminf(1,410/fmaxf(width,1));
            txt(h->desc[j],429,563,418,33,size,GOLD,F_UI,0);
        }
    }
    txt(L"敌方强度", 40, 563, 100, 33, 15, MUTED, F_UI, 0);
    for (int i = 0; i < 3; i++) {
        Area r = difficulty_area(i);
        box(r.x, r.y, r.w, r.h, A.difficulty == i ? GOLD : INK, 1);
        frame(r.x, r.y, r.w, r.h, GOLD, .46f, 1);
        txt(diffs[i], r.x, r.y, r.w, r.h, 14, A.difficulty == i ? INK : PAPER, F_BOLD, 1);
    }
    rule(38, 605, 1201);
    button(62, 621, 133, 48, L"←  返回", 0);
    txt(L"1–9 / 0  选英雄     ·     Enter  出战", 222, 625, 506, 43, 14, MUTED, F_UI, 0);
    button(747, 621, 204, 48, L"练习   V", 0);
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
static V coin_target(const Fx*f,V pos,float z) {
    Unit*u=&G.u[f->team];
    V end=alive(u)&&visible(&G,0,u->pos)&&dist(f->a,u->pos)<560?world(u->pos):pos;
    end.y-=28*z;return end;
}
static void world_fx(int layer) {
    float z = A.zoom;
    for (int i = 0; i < MAXFX; i++) {
        Fx *f = &G.fx[i];
        if (f->live && (f->kind == FX_ARRIVAL || f->kind == FX_COINS || f->kind == FX_EARTH)) {
            V point;
            if (!game_fx_position(&G, f, &point) || !visible(&G, 0, point))
                continue;
            V at = world(point);
            if (at.x < -360 || at.x > 1640 || at.y < -200 || at.y > 1030)
                continue;
            float t = 1 - f->life / f->total;
            if (f->kind == FX_ARRIVAL)
                paint_gate(at, z, t, f->size, layer, 0);
            if (f->kind == FX_EARTH && layer == 1)
                paint_stone_hit(at, z, t);
            if (f->kind == FX_COINS && layer == 1) {
                paint_coins(f, at, coin_target(f, at, z), z);
            }
            continue;
        }
        if (f->live && (f->kind == FX_ULT_CAST || f->kind == FX_ULT_IMPACT)) {
            int seeStart = visible(&G, 0, f->a),
                tankTell = f->n == 3 && f->kind == FX_ULT_CAST && visible(&G, 0, f->b);
            if (seeStart || tankTell) {
                V end = f->b;
                if (f->n == 4 && f->kind == FX_ULT_IMPACT && !visible(&G, 0, end)) {
                    /* Do not leave ghost silhouettes in unrevealed territory. */
                    end = f->a;
                    for (int step = 1; step <= 12; step++) {
                        V probe = add(f->a, mul(sub(f->b, f->a), step / 12.f));
                        if (!visible(&G, 0, probe))
                            break;
                        end = probe;
                    }
                }
                V a = world(f->a), b = world(end);
                if (a.x > -500 && a.x < 1780 && a.y > -330 && a.y < 1110)
                    paint_ultimate(f, a, b, z, layer);
            }
            continue;
        }
        if (!f->live || f->kind==FX_NUMBER || !visible(&G, 0, f->a) || (layer == 0) != (f->kind == FX_ZONE || f->kind == FX_HEAL))
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
/* The world, including all five arrival columns, is already at full opacity.
   Use a separate presentation clock so neither input nor announcer timing waits. */
#define HUD_REVEAL_DELAY .12f
#define HUD_REVEAL_SPAN 1.0f
static float hud_alpha(void){
    if(A.screen!=MATCH)return 1;
    float t=clampf((A.hudBirthAge-HUD_REVEAL_DELAY)/HUD_REVEAL_SPAN,0,1);
    return t*t*(3-2*t);
}
static void battlefield(void) {
    float z = A.zoom;
    V org = world(vec(0, 0));
    if(A.screen==TRAINING)art(ART_DOJO,0,0,1280,720,0,0,1536,1024,1,0);
    else art(ART_MAP, org.x, org.y, WW * z, WH * z, 0, 0, 1464, 1074, 1, 0);
    box(0, 0, 1280, 720, 0x10282a, .10f);
    int cinematic = A.screen == FINALE || A.screen == REPORT || A.screen == OPENING;
    if (!cinematic && !G.training) shroud(org.x, org.y, WW * z, WH * z, G.sight[0], GW, GH, G.sightFrame,1);
    if(A.screen==OPENING && A.sceneTime>2)
        shroud(org.x,org.y,WW*z,WH*z,G.sight[0],GW,GH,G.sightFrame,clampf((A.sceneTime-2)/.95f,0,1));
    int order[MAXU], n = 0;
    for (int i = 0; i < MAXU; i++) {
        Unit *u = &G.u[i];
        if (!u->used || (!cinematic && u->team != 0 && u->kind != CORE && u->kind != TOWER && !visible(&G, 0, u->pos)))
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
            int danger = alive(&P) && u->team == 1 && dist(P.pos, u->pos) < u->reach + 125;
            if (!cinematic && (danger || A.hoverUnit == i)) {
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
    if(!cinematic)world_fx(0);
    for(int j=0;j<n;j++) {
        Unit *u=&G.u[order[j]];
        if(!cinematic && u->kind==GOLEM && u->swingTarget>=0 && u->anim>0 && u->stun<=0)
            paint_stone_tell(u,world(u->pos),world(u->stoneAim),z);
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
        if (u->recall>0) paint_gate(p,z,1-u->recall/4,1,0,1);
        float opacity = 1;
        if (building && alive(&P) && P.pos.y <= u->pos.y && u->pos.y - P.pos.y < unitSize(u) * .8f &&
            fabsf(P.pos.x - u->pos.x) < unitSize(u) * .43f)
            opacity = .47f;
        if (building)
            sprite(u->team == 1 ? (u->kind == CORE ? 11 : 12 + unitArt(u)) : unitArt(u), p.x, p.y, sz,
                   opacity, 0);
        else
            motion_actor(u, p, sz, opacity);
        if (u->recall>0) paint_gate(p,z,1-u->recall/4,1,1,1);
        if (u->shield > 4 || u->grace > 0) {
            oval(p.x, p.y - sz * .42f, sz * .43f, sz * .49f, u->grace > 0 ? PAPER : GOLD, .58f, 0, 2);
        }

    }
    if (cinematic) return; /* End camera has no frozen missiles, HUD or tactical readouts. */
    for (int i = 0; i < MAXP; i++) {
        Shot *s = &G.shot[i];
        if (!s->live || s->kind == 3 || !visible(&G, 0, s->p))
            continue;
        V p = world(s->p);
        if (s->style == SHOT_FIREBLADE) {
            paint_flame_wave(s, p, z);
            continue;
        }
        float lift = 22;
        if (s->kind == 0) {
            const Unit *source = &G.u[s->owner], *end = &G.u[s->target];
            float travel = clampf(dist(s->start, s->p) / fmaxf(1, dist(s->start, end->pos)), 0, 1);
            float from = unitSize(source) * (source->kind == TOWER ? .76f : .49f);
            lift = from + (unitSize(end) * .35f - from) * travel;
        }
        p.y -= lift * z;
        if(s->kind!=0 && G.u[s->owner].kind==HERO && G.u[s->owner].role>=5){
            paint_roster_shot(G.u[s->owner].role,s,p,z);
            continue;
        }
        V v = norm(s->v);
        float sz = (s->kind == 0 ? s->radius : s->radius > 40 ? 16 : 8) * z;
        line(p.x - v.x * sz * 4, p.y - v.y * sz * 4, p.x, p.y, s->color, .57f, sz * 1.23f);
        oval(p.x, p.y, sz, sz, s->color, .93f, 1, 0);
        oval(p.x, p.y, sz * .42f, sz * .42f, PAPER, 1, 1, 0);
    }
    world_fx(1);
    /* Only the world is dimmed. Floating readouts, team pings, health bars
       and the screen HUD/minimap are painted AFTER the observer veil. */
    if (A.deathShade > 0)
        box(0, 0, 1280, 720, 0x0a1322, A.deathShade);
    float labelsAlpha=hud_alpha();
    if(labelsAlpha<=0)return;
    if(labelsAlpha<1)opacity_begin(labelsAlpha);
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
        int hero = u->kind == HERO, building = u->kind == TOWER || u->kind == CORE;
        uint32_t c = teamColor[u->team];
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
            if (id == 0 || A.hoverUnit == id || G.training) {
                txt(id == 0 ? L"你" : G.training ? L"陪练靶" : heroes[u->role].name, p.x - 49, by - 20, 98, 18, 12, PAPER, F_BOLD, 1);
            }
            box(p.x - bw * .5f - 14, by - 6, 14, 15, DEEP, .95f);
            fmt(p.x - bw * .5f - 14, by - 6, 14, 15, 10, id == 0 ? GOLD : c, F_NUM, 1, L"%d", u->level);
        }
        if (building && protected_building(&G, id)) {
            glyph(6, p.x - bw * .5f - 9, by + 2, 12, GOLD);
        }
    }
    for (int i = 0; i < MAXFX; i++) {
        Fx *f = &G.fx[i];
        if (!f->live || (f->kind != FX_NUMBER && (f->kind != FX_COINS || f->team != 0)) ||
            !visible(&G, 0, f->a))
            continue;
        V a = world(f->a);
        if (a.x < -120 || a.y < -100 || a.x > 1400 || a.y > 900)
            continue;
        if (f->kind == FX_COINS)
            paint_coin_amount(f, a, coin_target(f, a, z), z);
        else {
            float k = clampf(1 - f->life / f->total, 0, 1);
            fmt(a.x - 51, a.y - 30 - k * 34, 102, 27, f->size, 0x202720, F_BOLD, 1, L"%d", f->n);
            fmt(a.x - 52, a.y - 32 - k * 34, 102, 27, f->size, f->color, F_BOLD, 1, L"%d", f->n);
        }
    }
    if(labelsAlpha<1)opacity_end();
}
enum { MAPX = 25, MAPY = 25, MAPW = 174, MAPH = 128 };
static void minimap(void) {
    float x = MAPX, y = MAPY, w = MAPW, h = MAPH;
    box(x - 5, y - 5, w + 10, h + 10, DEEP, .96f);
    cover(ART_MAP, x, y, w, h, .5f, 1);
    box(x, y, w, h, DEEP, .16f);
    shroud(x, y, w, h, G.sight[0], GW, GH, G.sightFrame,1);
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
/* Measured width + fixed side bearings, rather than a one-size-fits-all panel. */
static float pill(float y, const wchar_t *s, int size, int face, uint32_t c, float pad, float height) {
    float w = clampf(text_width(s, (float)size, face) + 2 * pad, 2 * pad + 4, 850), x = 640 - w * .5f;
    panel(x, y, w, height);
    txt(s, x + pad - 2, y, w - 2 * pad + 4, height, (float)size, c, face, 1);
    return w;
}
static void card_portrait(int kind, int role, int team, float slash, float x, float y, float s) {
    uint32_t c = teamColor[team];
    box(x - 3, y - 3, s + 6, s + 6, 0x0b161e, .99f);
    frame(x - 2, y - 2, s + 4, s + 4, c, .94f, 1.8f);
    if (kind == HERO)
        portrait(role, x, y, s, s, .20f, 1);
    else {
        box(x, y, s, s, 0x27332f, 1);
        int artId = kind == CORE     ? (team == 1 ? 11 : 6)
                    : kind == TOWER  ? (team == 1 ? 17 : 5)
                    : kind == GOLEM  ? 10
                    : kind == SIEGE  ? 9
                    : kind == RANGED ? 8
                                     : 7;
        sprite(artId, x + s * .5f, y + s, s, .97f, 0);
    }
    if (slash > 0) {
        box(x, y, s, s, 0x241820, .35f);
        /* A pressure-loaded diagonal brush: blunt dark heel, full belly,
           split dry-brush tip. Reveal left to right beyond BOTH frame edges. */
        cursor_clip(x-11,y-12,(s+24)*slash,s+26);
        float edge[]={x-10,y-10, x+2,y-6, x+s*.44f,y+s*.27f, x+s+12,y+s+12,
                      x+s-1,y+s+7, x+s*.31f,y+s*.50f, x-6,y+1};
        float ink[]={x-9,y-10, x-1,y-7, x+s*.42f,y+s*.28f, x+s*.77f,y+s*.68f,
                     x+s+12,y+s+10, x+s-1,y+s+4, x+s+5,y+s+10,
                     x+s*.38f,y+s*.50f, x-5,y};
        poly(edge,7,0x581723,.99f);
        poly(ink,9,0xe83340,1);
        /* Broken, slightly curved hairs rather than a straight specular
           highlight: the wet belly separates into three dry tips. */
        float ferrule[]={x-10,y-7,x-9,y-11,x-7,y-8,x-7,y-12,x-3,y-7,
                         x-4,y-11,x+6,y-3,x+11,y+6,x+4,y+7,x-6,y+1};
        poly(ferrule,10,0xd42b38,1);
        float bristle[]={x-6,y-8,x+4,y-2,x+s*.48f,y+s*.34f,x+s*.32f,y+s*.28f};
        poly(bristle,4,0xf85446,.95f);
        float dry[]={x+s*.52f,y+s*.59f,x+s*.72f,y+s*.74f,x+s+6,y+s+13,
                     x+s*.84f,y+s*.92f};
        poly(dry,4,0xcb2633,.96f);
        line(x+s*.76f,y+s*.66f,x+s+12,y+s+7,0xf04b49,.92f,.95f);
        line(x+s*.22f,y+s*.30f,x+s*.41f,y+s*.46f,0xa12b30,.83f,.7f);
        clip_end();
    }
}
static void battle_dispatch(void) {
    if (G.cardCount) {
        const KillCard *k = &G.cards[0];
        float age = G.time - G.cardStart, enter = 1 - clampf(age / .17f, 0, 1);
        float y = 81 - enter * enter * 10;
        uint32_t rim = k->first ? 0xffd48a : k->victimTeam ? 0x7edbd7 : 0xef988c;
        float shape[] = {462, y + 27, 485, y + 11, 554, y + 7,  570, y,      710, y,
                         726, y + 7,  795, y + 11, 818, y + 27, 796, y + 47, 717, y + 51,
                         703, y + 58, 577, y + 58, 563, y + 51, 484, y + 47};
        poly(shape, 14, 0x10212b, .96f);
        gradient(551, y + 2, 178, 43, k->victimTeam ? 0x286069 : 0x674246, .76f, 0x19252e, .08f, 1);
        line(469, y + 26, 501, y + 10, rim, .65f, 1.3f);
        line(779, y + 10, 811, y + 26, rim, .65f, 1.3f);
        line(560, y + 4, 720, y + 4, rim, .67f, 1);
        line(576, y + 55, 704, y + 55, rim, .53f, 1);
        diamond(494, y + 30, 4, rim, .85f);
        diamond(786, y + 30, 4, rim, .85f);
        card_portrait(k->sourceKind, k->sourceRole, k->sourceTeam, 0, 509 - 15 * enter * enter, y + 5, 46);
        card_portrait(HERO, k->victimRole, k->victimTeam, clampf((age - .05f) / .14f, 0, 1),
                      725 + 15 * enter * enter, y + 5, 46);
        const wchar_t *headline = k->first ? L"第一滴血" : L"击  败";
        if (k->voice >= VO_DOUBLE && k->voice <= VO_ENEMY_PENTA)
            headline = ((const wchar_t *[]){L"二连击破", L"三连决胜", L"四连超凡", L"五连绝世"})[(k->voice-VO_DOUBLE)%4];
        if (k->voice >= VO_SPREE && k->voice <= VO_ENEMY_LEGENDARY)
            headline = ((const wchar_t *[]){L"锋芒毕露", L"无坚不摧", L"无人可挡", L"横扫千军", L"天下无双"})[(k->voice-VO_SPREE)%5];
        if (k->voice == VO_SHUTDOWN) headline = L"终  结";
        if (k->voice == VO_EXECUTED) headline = L"被 击 败";
        /* Portraits already identify both heroes: one centered result, no captions. */
        txt(headline, 554, y + 3, 172, 52, 23, PAPER, F_SERIF, 1);
    }
    /* Signals use their own centered lane so a rally never replaces a kill. */
    int shown = 0;
    for (int i = 0; i < 4 && shown < 2; i++) {
        Feed *f = &G.feed[i];
        if (!f->msg[0] || G.time - f->time >= 3.5f)
            continue;
        float y = (G.cardCount ? 146 : 82) + shown * 35;
        float w = clampf(text_width(f->msg, 15, F_BOLD) + 70, 144, 630), x = 640 - w / 2;
        float plate[] = {x,     y + 15, x + 13,     y,      x + w - 13, y,
                         x + w, y + 15, x + w - 13, y + 30, x + 13,     y + 30};
        poly(plate, 6, DEEP, .92f);
        line(x + 14, y + 1, x + w - 14, y + 1, f->color, .53f, 1);
        diamond(x + 19, y + 15, 3.5f, f->color, .95f);
        diamond(x + w - 19, y + 15, 3.5f, f->color, .95f);
        txt(f->msg, x + 28, y, w - 56, 30, 15, f->color, F_BOLD, 1);
        shown++;
    }
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
    battle_dispatch();
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
            pill(564, i == 3 ? L"引导 4 秒后返回泉水，受击或行动会中断。" : h->desc[i], 14, F_UI, PAPER, 17,
                 34);
        }
    }
    txt(L"SPACE  普攻     G  集结     TAB  战绩", 880, 633, 369, 26, 15, PAPER, F_UI, 2);
    txt(L"滚轮  缩放     F1  指南     ESC  暂停", 884, 664, 365, 24, 13, MUTED, F_UI, 2);
    if (G.noticeTime > 0 && u->recall <= 0)
        pill(523, G.notice, 15, F_BOLD, GOLD, 18, 33);
    if (!alive(u)) {
        wchar_t b[64];
        swprintf(b, 64, L"等待重生   %.1f s", fmaxf(0, u->respawn));
        pill(477, b, 18, F_BOLD, PAPER, 20, 38);
    } else if (u->recall > 0) {
        wchar_t b[64];
        swprintf(b, 64, L"正在回城   %.1f s", u->recall);
        float w = pill(523, b, 15, F_BOLD, 0x9ee9ff, 18, 34);
        box(640 - w / 2 + 4, 553, (w - 8) * (1 - u->recall / 4), 2, 0x75dcff, 1);
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
static void training_start(int role) {
    select_role(role);
    audio_clear();
    invalidate_shroud();
    game_training(&G, role);
    A.practiceQueued = -1;
    A.screen = TRAINING;
    A.overlay = NONE;
    A.cam = vec(640, 360);
    A.zoom = 1;
    A.pending = A.hoverUnit = -1;
    A.deathShade = 0;
    A.panConsumed = 0;
    memset(A.held, 0, sizeof A.held);
}
static void training_exit(void) {
    A.screen = DRAFT;
    A.overlay = NONE;
    A.pending = A.hoverUnit = -1;
    memset(A.held, 0, sizeof A.held);
    audio_clear();
}
static const Area practiceReset = {968, 22, 119, 38}, practiceLeave = {1099, 22, 156, 38};
static Area trainingSlot(int i) {
    return (Area){471 + i * 116, 632, 106, 65};
}
static void training_hud(void) {
    const HeroDef *h = &heroes[P.role];
    gradient(0, 0, 1280, 105, DEEP, .87f, DEEP, 0, 1);
    gradient(0, 589, 1280, 131, DEEP, 0, DEEP, .94f, 1);
    txt(L"竹庭演武场", 28, 12, 255, 43, 28, PAPER, F_SERIF, 0);
    txt(L"自由练习   /   技能无冷却", 31, 53, 262, 21, 14, GOLD, F_BOLD, 0);
    button(practiceReset.x, practiceReset.y, practiceReset.w, practiceReset.h, L"重置   T", 0);
    button(practiceLeave.x, practiceLeave.y, practiceLeave.w, practiceLeave.h, L"返回选将   Esc", 0);
    panel(24, 633, 297, 62);
    txt(h->name, 38, 637, 100, 29, 21, PAPER, F_SERIF, 0);
    txt(L"1–9 / 0  切换英雄", 40, 669, 159, 19, 13, GOLD, F_BOLD, 0);
    txt(L"WASD  移动", 168, 640, 143, 22, 14, PAPER, F_UI, 0);
    txt(L"空格 / 右键  普攻", 168, 664, 144, 22, 14, PAPER, F_UI, 0);
    for (int i = 0; i < 3; i++) {
        Area r = trainingSlot(i);
        panel(r.x, r.y, r.w, r.h);
        skillart(P.role, i, r.x + 4, r.y + 3, r.w - 8, 34, 1);
        if (A.pending == i)
            frame(r.x, r.y, r.w, r.h, GOLD, 1, 2);
        fmt(r.x, r.y + 37, r.w, 27, 14, PAPER, F_BOLD, 1, L"%lc  %ls", L"QER"[i], h->skill[i]);
        if(inside(r)){
            float tw=fminf(870,text_width(h->desc[i],15,F_UI)+32);
            panel(640-tw*.5f,591,tw,31);
            txt(h->desc[i],640-tw*.5f,591,tw,31,15,PAPER,F_UI,1);
        }
    }
    panel(943, 633, 312, 62);
    fmt(954, 637, 287, 26, 16, PAPER, F_BOLD, 1, L"累计伤害  %.0f     命中  %d", G.practiceDamage,
        G.practiceHits);
    fmt(954, 668, 287, 21, 13, GOLD, F_UI, 1, L"上次伤害  %.0f     /     靶子耗尽后回满", G.lastDamage);
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
    button(guideDone.x, guideDone.y, guideDone.w, guideDone.h, L"明白了    ↵", 1);
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
    if(A.screen==REPORT) area_button(statsBack,L"←  返回个人战绩",0);
    else txt(L"松开 Tab 返回战场", 123, 592, 1033, 30, 14, GOLD, F_UI, 1);
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
    area_button(reportHome,L"返回主界面",0);
    area_button(reportStats,L"查看全局战绩",0);
    area_button(reportAgain,L"再来一局 · 重新选将",1);
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
    if(r>=5){
        range=i==0?480:i==2||r==9?470:r==8?156:0;
        radius=i==1?(float[]){0,245,138,0,99}[r-5]:0;
    }
    if(i==1 && r==1) range=WIND_ARROW_RANGE;
    if(i==1 && r==3) radius=AEGIS_RADIUS;
    if(i==1 && r==5) radius=ARC_REACH;
    if(i==1 && r==6){range=TIDE_ECHO_RANGE;radius=TIDE_ECHO_RADIUS;}
    V target = world(add(P.pos, mul(dir, fminf(range, dist(m, P.pos)))));
    uint32_t c = heroes[r].color;
    if(i==1 && (r==3||r==6))
        oval(pos.x,pos.y,(r==3?242:245)*z,(r==3?242:245)*z,JADE,.55f,0,1.5f);
    if(i==1 && r==1){
        V end=world(add(P.pos,mul(dir,130)));
        oval(end.x,end.y,13*z,8*z,c,.85f,0,2*z);
    }
    if (range > 0)
        oval(pos.x, pos.y, range * z, range * z, c, .37f, 0, 1);
    if(r>=5 && i==2){
        V centers[5];float rad;int n=hero_seals(r,P.pos,m,P.face,centers,&rad);
        for(int j=0;j<n;j++){
            V v=world(centers[j]);
            oval(v.x,v.y,rad*z,rad*z,c,.06f,1,0);
            oval(v.x,v.y,rad*z,rad*z,c,.77f,0,1.5f);
        }
    }
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
    else if (A.screen == VERSUS)
        present_versus(&G,A.lineup,A.sceneTime);
    else if(A.screen==OPENING){
        battlefield();
        float fade=clampf(1-A.sceneTime/.32f,0,1);
        if(fade>0)box(0,0,1280,720,DEEP,fade);
    } else if(A.screen==TRAINING){
        battlefield();if(A.pending>=0)aim_overlay();training_hud();
    } else if (A.screen == FINALE || A.screen == REPORT) {
        battlefield();
        int losing = 1-G.winner;
        present_core(world(G.u[16+7*losing].pos), A.zoom, losing, A.sceneTime);
        if(A.screen==REPORT){if(A.overlay!=SCORE)result();}
        else present_ending(!G.winner,A.sceneTime);
    } else {
        battlefield();
        if (A.pending >= 0)
            aim_overlay();
        float alpha=hud_alpha();
        if(alpha>0){
            if(alpha<1)opacity_begin(alpha);
            hud();
            if(alpha<1)opacity_end();
        }
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

}
void ui_reset(void) {
    memset(&A, 0, sizeof A);
    A.screen = HOME;
    A.selected = 0;
    A.difficulty = 1;
    A.zoom = .95f;
    A.hoverUnit = A.pending = -1;
}
static void prepare_session(uint32_t seed,int staged) {
    A.hudBirthAge=HUD_REVEAL_DELAY+HUD_REVEAL_SPAN;
    A.deathShade = 0;
    A.panConsumed = 0;
    A.sceneTime = 0;
    audio_clear();
    invalidate_shroud();
    if(staged)game_prepare_match(&G,A.selected,A.difficulty,seed);
    else game_init(&G, A.selected, A.difficulty, seed);
    A.screen = MATCH;
    A.overlay = NONE;
    A.hoverUnit = A.pending = -1;
    A.zoom = .95f;
    memset(A.held, 0, sizeof A.held);
    camera(P.pos, -1);
    audio_event(S_UI);
}
void ui_start(uint32_t seed){prepare_session(seed,0);}
/* Build the roster ONCE before dealing. A private PRNG shuffles display
   order without ever disturbing hero ids, lane AI or simulation randomness. */
void ui_versus(uint32_t seed) {
    prepare_session(seed,1);
    A.screen = VERSUS;
    uint32_t state = seed ^ 0xb7943ad5u;
    for (int i = 0; i < 10; i++)
        A.lineup[i] = i;
    for (int team = 0; team < 2; team++) {
        int first = team ? 5 : 1, last = team ? 9 : 4;
        for (int j = last; j > first; j--) {
            state = state * 1664525u + 1013904223u;
            int k = first + (int)(((uint64_t)state * (j - first + 1)) >> 32);
            int v = A.lineup[j];
            A.lineup[j] = A.lineup[k];
            A.lineup[k] = v;
        }
    }
}
static void enter_finale(void) {
    int loser = 1 - G.winner;
    V core = G.u[16 + loser * 7].pos;
    A.screen = FINALE;
    A.overlay = NONE;
    A.sceneTime = 0;
    A.hoverUnit = A.pending = -1;
    A.deathShade = 0;
    memset(A.held, 0, sizeof A.held);
    G.soundN = 0;
    audio_clear();
    A.zoom = 1.38f;
    camera(add(core, vec(loser ? -95 : 95, -66)), -1);
    A.endFrom = A.cam;
    A.endZoom = A.zoom;
}
static void finale_tick(float dt) {
    float old = A.sceneTime;
    A.sceneTime += dt;
    float k = clampf(A.sceneTime / .94f, 0, 1);
    k = k * k * (3 - 2 * k);
    int loser = 1 - G.winner;
    V core = G.u[16 + loser * 7].pos;
    A.zoom = A.endZoom + (2.40f - A.endZoom) * k;
    V target = add(A.endFrom, mul(sub(add(core, vec(0, -46)), A.endFrom), k));
    float b = A.sceneTime - CORE_BREAK;
    if (b >= 0 && b < .42f)
        target = add(target, mul(vec(sinf(b * 81), cosf(b * 69)), (1 - b / .42f) * 3.8f));
    camera(target, -1);
    if (old < CORE_BREAK && A.sceneTime >= CORE_BREAK)
        audio_event(S_FRACTURE);
    if (old < CREST_AT && A.sceneTime >= CREST_AT) {
        audio_event(G.winner == 0 ? S_WIN : S_LOSE);
        audio_event(G.winner == 0 ? VO_VICTORY : VO_DEFEAT);
    }
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
    /* Buffer one aimed input in practice so a tap in the recovery frames of
       the previous skill is not lost. Never cancel its release/contact frame. */
    if(G.training && P.anim>0 && P.pose>=POSE_SKILL){
        A.practiceQueued=s;A.practiceAim=mouseworld();A.pending=-1;return;
    }
    if (!game_cast(&G, 0, s, mouseworld()) && alive(&P)) {
        if (P.stun > 0)
            game_notice(&G, L"受到控制，暂时无法施法");
        else if (P.pose == POSE_ULT && P.anim > 0)
            game_notice(&G, L"正在完成大招动作");
        else
            game_notice(&G, L"技能尚在冷却");
    }
    A.pending = -1;
}
void ui_click(int right) {
    if (A.mx < 0 || A.my < 0 || A.mx >= 1280 || A.my >= 720)
        return;
    if (A.screen == MATCH && G.winner>=0) enter_finale();
    if (A.screen == VERSUS || A.screen==OPENING) return;
    if (A.screen == FINALE) {
        if(!right && A.sceneTime>=CONTINUE_AT) A.screen=REPORT;
        return;
    }
    if(A.screen==REPORT){
        if(right) return;
        if(A.overlay==SCORE){
            if(inside(statsBack)){A.overlay=NONE;audio_event(S_UI);}
        }else if(inside(reportStats)){A.overlay=SCORE;audio_event(S_UI);}
        else if(inside(reportHome)){A.screen=HOME;audio_clear();}
        else if(inside(reportAgain)){A.screen=DRAFT;audio_clear();}
        return;
    }
    if(A.screen==TRAINING){
        if(!right && inside(practiceLeave)){training_exit();return;}
        if(!right && inside(practiceReset)){training_start(A.selected);return;}
        for(int i=0;i<3;i++)if(inside(trainingSlot(i))){if(!right)A.pending=i;return;}
        if(A.pending>=0){if(right)A.pending=-1;else cast(A.pending);return;}
        if(A.my<80 || A.my>625)return;
        game_order(&G,mouseworld(),target_under(),!right);return;
    }
    if (A.overlay == GUIDE) {
        if (!right && inside(guideDone))
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
            audio_clear();
        }
        return;
    }
    if (A.overlay != NONE)
        return;
    if ((A.screen==HOME || A.screen==DRAFT) && !right) {
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
        if (inside(homeChoose)) {
            A.screen = DRAFT;
            audio_event(S_UI);
        } else if (inside(homeGuide))
            showguide();
        return;
    }
    if (A.screen == DRAFT) {
        if (right)
            return;
        for (int i = 0; i < HERO_COUNT; i++)
            if (inside(card_area(i))) {
                select_role(i);
                return;
            }
        for (int i = 0; i < 3; i++)
            if (inside(difficulty_area(i))) {
                A.difficulty = i;
                audio_event(S_UI);
                return;
            }
        if (hit(62, 621, 133, 48))
            A.screen = HOME;
        else if (hit(747, 621, 204, 48))
            training_start(A.selected);
        else if (hit(977, 615, 243, 59))
            ui_versus(GetTickCount());
        return;
    }
    if (A.pending >= 0) {
        if (right)
            A.pending = -1;
        else
            cast(A.pending);
        return;
    }
    if(hud_alpha()<.075f){ /* Invisible panels are not live hit targets. */
        game_order(&G,mouseworld(),target_under(),!right);return;
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
        V p = vec((A.mx - MAPX) / MAPW * WW, (A.my - MAPY) / MAPH * WH);
        if (alive(&P))
            game_order(&G, p, -1, !right);
        else
            camera(p, -1);
        return;
    }
    if (hit(24, 616, 304, 81) || hit(477, 15, 326, 54) || hit(20, 158, 184, 22))
        return;
    game_order(&G, mouseworld(), target_under(), !right);
}
static int move_bit(int k){return k=='W'?1:k=='A'?2:k=='S'?4:k=='D'?8:0;}
void ui_key(int k, int down) {
    if (k < 0 || k > 255)
        return;
    if (!down)
        A.panConsumed &= ~move_bit(k);
    int was = A.held[k];
    A.held[k] = down;
    if (!down) {
        if (k == VK_TAB && A.screen==MATCH && A.overlay == SCORE)
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
    if (A.screen == MATCH && G.winner>=0) enter_finale();
    if(A.screen==VERSUS || A.screen==OPENING) return;
    if(A.screen==FINALE){
        if((k==VK_RETURN || k==VK_SPACE) && A.sceneTime>=CONTINUE_AT) A.screen=REPORT;
        return;
    }
    if(A.screen==REPORT){
        if(A.overlay==SCORE){
            if(k==VK_ESCAPE || k==VK_RETURN || k==VK_TAB) A.overlay=NONE;
            return;
        }
        if(k==VK_TAB){A.overlay=SCORE;return;}
        if(k==VK_RETURN) A.screen=DRAFT;
        if(k==VK_ESCAPE) A.screen=HOME;
        if(A.screen!=REPORT)audio_clear();
        return;
    }
    if(A.screen==TRAINING){
        if(k==VK_ESCAPE){if(A.pending>=0)A.pending=-1;else training_exit();}
        else if(k=='T')training_start(A.selected);
        else if(k>='0' && k<='9' && (k-'0'+9)%10!=A.selected)training_start((k-'0'+9)%10);
        else if(k=='Q'||k=='E'||k=='R')cast(k=='Q'?0:k=='E'?1:2);
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
        if (k >= '0' && k <= '9')
            select_role((k-'0'+9)%10);
        else if (k == 'V')
            training_start(A.selected);
        else if (k == VK_LEFT)
            select_role((A.selected+HERO_COUNT-1)%HERO_COUNT);
        else if (k == VK_RIGHT)
            select_role((A.selected+1)%HERO_COUNT);
        else if (k == VK_UP || k == VK_DOWN)
            select_role((A.selected+5)%HERO_COUNT);
        else if (k == VK_RETURN)
            ui_versus(GetTickCount());
        else if (k == VK_ESCAPE)
            A.screen = HOME;
        return;
    }
    if (G.winner >= 0) {
        if (k == VK_RETURN)
            A.screen = DRAFT;
        else if (k == VK_ESCAPE)
            A.screen = HOME;
        if(A.screen!=MATCH) audio_clear();
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
        camera(alive(&P) ? P.pos : A.cam, -1);
    }
}
void ui_blur(void) {
    A.practiceQueued = -1;
    A.panConsumed = 0;
    memset(A.held, 0, sizeof A.held);
    A.pending = -1;
    if (A.screen == MATCH && G.winner < 0 && (A.overlay == NONE || A.overlay == SCORE))
        A.overlay = PAUSED;
}
void ui_tick(float dt) {
    A.clock += dt;
    if (A.screen == VERSUS) {
        float old = A.sceneTime;
        A.sceneTime += dt;
        if (old < .38f && A.sceneTime >= .38f)
            audio_event(S_VS);
        if (A.sceneTime >= VS_DURATION) {
            A.screen = OPENING;
            A.sceneTime = 0;
            A.zoom = fminf(1280.f / WW, 720.f / WH);
            A.cam = vec(WW * .5f, WH * .5f);
            audio_event(VO_WELCOME);
            memset(A.held, 0, sizeof A.held);
        }
        return;
    }
    if (A.screen == FINALE) {
        finale_tick(dt);
        return;
    }
    if (A.screen == OPENING) {
        A.sceneTime += dt;
        float k = clampf((A.sceneTime - .40f) / (OPENING_DURATION - .58f), 0, 1);
        k = k * k * k * (k * (6 * k - 15) + 10);
        float wide = fminf(1280.f / WW, 720.f / WH);
        A.zoom = wide * powf(.95f / wide, k);
        float hw = fminf(640 / A.zoom, WW * .5f), hh = fminf(360 / A.zoom, WH * .5f);
        V center = vec(WW * .5f, WH * .5f), core = G.u[16].pos;
        V aim = add(center, mul(sub(core, center), k));
        A.cam = vec(clampf(aim.x, hw, WW - hw), clampf(aim.y, hh, WH - hh));
        if (A.sceneTime >= OPENING_DURATION) {
            game_deploy(&G);
            A.screen = MATCH;
            A.hudBirthAge=0;
            A.zoom = .95f;
            camera(P.pos, -1);
            A.sceneTime = 0;
            memset(A.held, 0, sizeof A.held);
        }
        return;
    }
    if ((A.screen != MATCH && A.screen != TRAINING) || A.overlay != NONE)
        return;
    if(A.screen==MATCH)
        A.hudBirthAge=fminf(HUD_REVEAL_DELAY+HUD_REVEAL_SPAN,A.hudBirthAge+dt);
    if (G.winner >= 0) {
        enter_finale();
        return;
    }
    int wasAlive = alive(&P), serial = P.serial;
    int w = A.held['W'] && !(A.panConsumed & 1), a = A.held['A'] && !(A.panConsumed & 2),
        s = A.held['S'] && !(A.panConsumed & 4), d = A.held['D'] && !(A.panConsumed & 8);
    Input in = {(float)(d - a), (float)(s - w), A.held[VK_SPACE]};
    V old = P.pos;
    if (G.training && (P.anim <= 0 || P.pose < POSE_SKILL) && A.pending < 0) {
        if (A.practiceQueued >= 0) {
            game_cast(&G, 0, A.practiceQueued, A.practiceAim);
            A.practiceQueued = -1;
        } else
            for (int i = 2; i >= 0; i--)
                if (A.held[(int)"QER"[i]]) {
                    cast(i);
                    break;
                }
    }
    game_step(&G, dt, wasAlive ? in : (Input){0});
    if (G.winner >= 0) {
        enter_finale();
        return;
    }
    if (!alive(&P)) {
        A.pending = -1;
        A.deathShade = fminf(.41f, A.deathShade + dt * 1.85f);
        V dir = G.winner < 0 ? norm(vec(in.x, in.y)) : vec(0, 0);
        camera(add(A.cam, mul(dir, dt * 820.f / A.zoom)), -1);
    } else {
        A.deathShade = 0;
        if (!wasAlive) {
            /* A spectating key held across respawn must not walk the new hero. */
            A.panConsumed =
                (A.held['W'] ? 1 : 0) | (A.held['A'] ? 2 : 0) | (A.held['S'] ? 4 : 0) | (A.held['D'] ? 8 : 0);
        }
        if (!G.training)
            camera(P.pos, !wasAlive || serial != P.serial || dist(old, P.pos) > 350 ? -1 : dt);
    }
    A.hoverUnit = target_under();
    for (int i = 0; i < G.soundN; i++)
        audio_event(G.sound[i]);
    G.soundN = 0;
}
