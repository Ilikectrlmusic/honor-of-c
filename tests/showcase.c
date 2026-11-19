/* Deterministic, staged encounters for the audiovisual regression reel.
   Same UI, events, renderer and audio mixer as the playable game. Only initial
   positions/health and keyboard inputs are staged; no FX or banners injected. */
#include "../src/ui.h"
#include "../src/draw.h"
#include "../src/audio.h"
#include <assert.h>
#include <objbase.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#define W 1280
#define H 720
#define SECS 29
static unsigned char pixels[W * H * 4];
static short audio[2206];
static void tap(int k) {
    ui_key(k, 1);
    ui_key(k, 0);
}
static void still(const wchar_t *name) {
    wchar_t p[160];
    swprintf(p, 160, L"build\\gallery\\%ls.bmp", name);
    assert(draw_save_bmp(p));
}
static void stage_camp(void) {
    Game *g = &app.g;
    Unit *m = &g->u[24], *p = &g->u[0];
    for (int i = 1; i < MAXU; i++)
        if (i != 1 && i != 6 && i != 24)
            g->u[i].used = 0;
    g->u[6].pos = g->u[6].home;
    g->u[6].stun = 99;
    g->u[1].stun = 99;
    g->u[1].pos = add(g->u[1].home, vec(110, -38));
    g->wave = g->pay = 99;
    g->time = 32;
    p->atk = 99;
    p->pos = add(m->home, vec(69, 0));
    m->pos = m->home;
    m->hp = m->maxhp;
    m->stun = m->atk = m->anim = m->slow = m->aggro = 0;
    m->target = m->swingTarget = -1;
    m->face = vec(1, 0);
    m->dir = 1;
    p->stun = 0;
    memset(g->shot, 0, sizeof g->shot);
    memset(g->fx, 0, sizeof g->fx);
    app.zoom = 1.33f;
    app.cam = p->pos;
    g->sightTimer = 0;
    app.mx = 860;
    app.my = 236;
}
/* Stage only a final legal hit. The normal kill, camera, audio and UI pipeline
   then runs unchanged; never inject winner flags, banners or fragments. */
static void last_hit(int team){
    Game *g=&app.g;
    Unit *core=&g->u[16+7*(1-team)],*attacker=&g->u[team*5];
    g->u[10+7*(1-team)].hp=0;
    core->hp=1;
    attacker->role=3;attacker->cd[0]=attacker->stun=0;
    attacker->pos=add(core->pos,vec(team?86:-86,40));
    assert(game_cast(g,team*5,0,core->pos) && g->winner==team);
}
int main(int argc, char **argv) {
    int portraits = argc == 2 && !strcmp(argv[1], "--portraits");
    int ceremony = argc == 2 && !strcmp(argv[1], "--ceremony");
    int observer = argc == 2 && !strcmp(argv[1], "--observer");
    _setmode(_fileno(stdout), _O_BINARY);
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    ui_reset();
    if (!draw_init(NULL, W, H) || !audio_capture_open())
        return 2;
    if (text_width(L"金币不足", 15, F_BOLD) + 30 >= text_width(L"集结信号正在冷却", 15, F_BOLD) + 30)
        return 3;
    FILE *pcm = fopen(ceremony ? "build/ceremony-s16le.pcm" : portraits  ? "build/portraits-s16le.pcm"
                      : observer ? "build/observer-s16le.pcm"
                                 : "build/feedback-s16le.pcm",
                      "wb");
    if (!pcm)
        return 3;
    CreateDirectoryW(L"build\\gallery", NULL);
    Game *g = &app.g;
    Unit *p = &g->u[0], *m = &g->u[24], *v = &g->u[6];
    for (int frame = 0; frame < (ceremony ? 18 : portraits ? 25 : observer ? 13 : SECS) * 20; frame++) {
        if(ceremony){
            if(!frame)tap(VK_RETURN);
            if(frame==4)tap('2');
            if(frame==16)tap('4');
            if(frame==30){app.mx=1060;app.my=640;ui_click(0);assert(app.screen==VERSUS);}
            if(frame==109)assert(app.screen==MATCH && p->role==3);
            if(frame==130)last_hit(0);
            if(frame==240){ui_start(991);last_hit(1);}
        } else if (observer) {
            if (!frame) {
                ui_start(151);
                ui_key('D', 1);
            }
            if (frame == 26)
                ui_key('D', 0);
            if (frame == 40) {
                p->pos = lane_at(1, .47f);
                g->u[1].pos = add(p->pos, vec(-136, 60));
                g->u[1].atk = 99;
                g->u[1].stun = 2;
                g->u[5].pos = add(p->pos, vec(82, -12));
                g->u[5].atk = 99;
                g->u[5].stun = 2;
                g->sightTimer = 0;
                ui_wheel(0);
                game_rally(g, p->pos);
            }
            if (frame == 61) {
                Unit *attacker = &g->u[5];
                attacker->stun = 0;
                attacker->role = 3;
                attacker->cd[0] = 0;
                p->hp = 1;
                p->grace = 0;
                assert(game_cast(g, 5, 0, p->pos) && !alive(p));
            }
            if (frame == 97)
                ui_key('A', 1);
            if (frame == 113)
                ui_key('A', 0);
            if (frame == 144)
                ui_key('W', 1);
            if (frame == 165)
                ui_key('W', 0);
            if (frame == 224) {
                assert(alive(p));
                ui_key('D', 1);
            }
            if (frame == 247)
                ui_key('D', 0);
        } else if (portraits) {
            /* Five seconds on the title, four seconds per hero. The same live
               selection paths used by mouse, numbers and arrows drive this reel. */
            if (frame == 100)
                tap(VK_RETURN);
            if (frame >= 100 && (frame - 100) % 80 == 0) {
                int id = (frame - 100) / 80;
                if (id % 2) {
                    app.mx = 110 + 164 * id;
                    app.my = 255;
                    ui_click(0);
                } else
                    tap('1' + id);
                assert(app.screen == DRAFT && app.selected == id);
                app.mx = 625;
                app.my = 65;
            }
        } else {
            if (frame == 94)
                tap(VK_RETURN);
            if (frame == 97)
                tap('4');
            if (frame == 118)
                ui_start(151);
            if (frame == 264)
                stage_camp();
            if (frame == 272)
                ui_key('S', 1);
            if (frame == 282)
                ui_key('S', 0);
            if (frame == 322) {
                m->hp = 4;
                assert(game_cast(g, 0, 0, m->pos));
                assert(!alive(m));
            }
            if (frame == 365) {
                v->pos = add(p->pos, vec(71, -8));
                v->hp = 2;
                v->grace = 0;
                v->stun = 99;
                p->atk = 0;
                game_order(g, v->pos, 6, 0);
            }
            if (frame == 384) {
                assert(g->firstBlood);
                game_rally(g, p->pos);
            }
            if (frame == 443)
                tap('B');
        }
        for (int i = 0; i < 3; i++)
            ui_tick(1.f / 60);
        ui_draw();
        if (!draw_end() || !draw_copy_pixels(pixels, sizeof pixels))
            return 4;
        if(ceremony) {
            if(frame==70)still(L"versus");
            if(frame==153)still(L"core-charge");
            if(frame==162)still(L"core-break");
            if(frame==220)still(L"victory-crest");
            if(frame==332)still(L"defeat-crest");
        } else if (!portraits && !observer) {
            if (frame == 131)
                still(L"fountain-spawn");
            if (frame == 274) {
                assert(m->swingTarget == 0 && m->releaseIn > 0 && m->releaseIn < STONE_CONTACT - STONE_TRACK);
                still(L"golem-lock");
            }
            if (frame == 280)
                still(L"golem-dodge");
            if (frame == 328)
                still(L"monster-bounty");
            if (frame == 388)
                still(L"first-blood-rally");
            if (frame == 496)
                still(L"recall-channel");
            if (frame == 530)
                still(L"fountain-return");
        }
        if (fwrite(pixels, 1, sizeof pixels, stdout) != sizeof pixels)
            return 5;
        int samples = ((frame + 1) * 22050 / 20) - (frame * 22050 / 20);
        audio_capture_block(audio, samples, app.screen == HOME || app.screen == DRAFT);
        if (fwrite(audio, 4, samples, pcm) != (size_t)samples)
            return 6;
    }
    fclose(pcm);
    draw_close();
    CoUninitialize();
    return 0;
}
