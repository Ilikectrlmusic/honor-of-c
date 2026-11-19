/* Offscreen Win32/Direct2D tests: compare the playable UI, not mock sprites. */
#include "../src/ui.h"
#include "../src/draw.h"
#include "../src/domain.h"
#include "../src/audio.h"
#include "../src/presentation.h"
#include <assert.h>
#include <math.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { W = 1280, H = 720, N = W * H };
static uint32_t before[N], after[N], mask0[N], mask1[N], plate[N];
static void capture(uint32_t *pixels) {
    assert(draw_end() && draw_copy_pixels((unsigned char *)pixels, N * 4));
}
static int change(uint32_t a, uint32_t b) {
    int max = 0;
    for (int k = 0; k < 3; k++, a >>= 8, b >>= 8) {
        int d = abs((int)(a & 255) - (int)(b & 255));
        if (d > max)
            max = d;
    }
    return max;
}
static int empty(uint32_t pixel) {
    return (pixel & 0xffffff) == 0xff00ff;
}
static void hero_mask(float t, uint32_t *out) {
    float h = 1280.f * 1024 / 1536;
    draw_begin();
    box(0, 0, W, H, 0xff00ff, 1);
    living_art(0, 0, -(h - 720) * .15f, W, h, t, 1);
    capture(out);
}
static void layer_check(void) {
    draw_begin();
    cover(ART_LANDSCAPE, 0, 0, W, H, .15f, 1);
    capture(plate);
    app.clock = .27f;
    ui_draw();
    capture(before);
    hero_mask(app.clock, mask0);
    app.clock = 2.13f;
    ui_draw();
    capture(after);
    hero_mask(app.clock, mask1);
    int fixed = 0, revealed = 0, moving = 0, hem = 0;
    for (int i = 0; i < N; i++) {
        int old = empty(mask0[i]), now = empty(mask1[i]), x = i % W, y = i / W;
        if (old && now) {
            assert(change(before[i], after[i]) <= 1);
            fixed++;
        }
        if (!old && !now && change(before[i], after[i]) > 5)
            moving++;
        if (!old && !now && x > 1120 && y > 625 && change(before[i], after[i]) > 5)
            hem++;
        /* No home typography/gradient here: newly uncovered silhouette
           pixels must expose the real full-color clean plate, not a smear. */
        if (!old && now && x > 798 && y > 85 && y < 550) {
            assert(change(plate[i], after[i]) <= 1);
            revealed++;
        }
    }
    assert(fixed > 350000 && moving > 10000 && revealed > 30 && hem > 350);
    printf(
        "Title: %d stationary scenery pixels, %d revealed original plate pixels, %d moving subject pixels.\n",
        fixed, revealed, moving);
    for (int role = 0; role < 5; role++) {
        for (int frame = 0; frame < 2; frame++) {
            draw_begin();
            cursor_clip(840, 98, 390, 493);
            domain_draw(role, 840, 98, 390, 493, frame ? 2.73f : .21f);
            clip_end();
            capture(frame ? after : before);
        }
        int n = 0;
        for (int i = 0; i < N; i++)
            if (change(before[i], after[i]) > 3) {
                assert(i % W >= 840 && i % W < 1230 && i / W >= 98 && i / W < 591);
                n++;
            }
        assert(n > 400);
        printf("Draft hero %d: %d animated environment pixels without the hero.\n", role + 1, n);
    }
}
static void ticks(int n) {
    while (n--)
        ui_tick(1.f / 60);
}
static int stable_color(int x0, int y0, int x1, int y1, uint32_t rgb) {
    int count = 0;
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
            int i = y * W + x;
            if (change(before[i], rgb) <= 2) {
                assert(change(before[i], after[i]) <= 1);
                count++;
            }
        }
    return count;
}
static void spectator_check(void) {
    ui_start(151);
    Game *g = &app.g;
    Unit *p = &g->u[0], *foe = &g->u[5];
    for (int i = 0; i < MAXU; i++)
        if (i != 0 && i != 1 && i != 5 && !(i >= 10 && i <= 23))
            g->u[i].used = 0;
    g->wave = g->pay = 99;
    ticks(240); /* Let opening beams and the initial feed expire before staging the duel. */
    p->pos = lane_at(1, .5f);
    V corpse = p->pos;
    g->u[1].pos = add(corpse, vec(-155, 55));
    g->u[1].stun = 99;
    foe->pos = add(corpse, vec(65, 8));
    foe->role = 3;
    p->hp = 1;
    p->grace = 0;
    ui_wheel(0);
    game_rally(g, corpse);
    assert(game_cast(g, 5, 0, corpse) && !alive(p) && g->cardCount == 1);
    ticks(17);
    assert(app.deathShade > .40f);
    ui_draw();
    capture(after);
    assert(draw_save_bmp(L"build/gallery/death-watch.bmp"));
    float shade = app.deathShade;
    app.deathShade = 0;
    ui_draw();
    capture(before);
    app.deathShade = shade;
    long long lit = 0, dim = 0;
    for (int y = 260; y < 520; y += 2)
        for (int x = 250; x < 1010; x += 2) {
            uint32_t a = before[y * W + x], b = after[y * W + x];
            lit += (a >> 8) & 255;
            dim += (b >> 8) & 255;
        }
    assert(dim < lit * .83);
    assert(stable_color(1065, 22, 1171, 58, 0xe7c98d) > 20);         /* gold + shop */
    assert(stable_color(553, 99, 727, 129, 0xf4efdc) > 12);          /* kill banner text */
    assert(stable_color(540, 146, 740, 176, g->feed[0].color) > 12); /* simultaneous rally */
    int identical = 0;
    for (int y = 91; y < 127; y++)
        for (int x = 515; x < 549; x++)
            identical += before[y * W + x] == after[y * W + x];
    assert(identical > 1000);
    /* Red paint outside BOTH corners, upper-left to lower-right. */
    assert(stable_color(713, 72, 725, 86, 0xe83340) + stable_color(713, 72, 725, 86, 0xd42b38) > 5);
    assert(stable_color(771, 132, 785, 147, 0xe83340) > 2);
    V camera = app.cam;
    ui_key('W', 1);
    ui_key('D', 1);
    ticks(14);
    V delta = sub(app.cam, camera);
    assert(delta.x > 100 && fabsf(delta.x + delta.y) < .5f && dist(p->pos, corpse) < .001f);
    assert(fabsf(dist(camera, app.cam) - 820.f / app.zoom * 14 / 60) < 1);
    ui_key('W', 0);
    ui_key('D', 0);
    camera = app.cam;
    ui_wheel(120);
    assert(dist(camera, app.cam) < .1f);
    ui_key('A', 1);
    ui_key('S', 1);
    ticks(14);
    assert(app.cam.x < camera.x - 100 && app.cam.y > camera.y + 100);
    ui_key('A', 0);
    ui_key('S', 0);
    /* Park the observer where our team has no sight. The camera is not an
       observer in the fog simulation; even the enemy crystal stays in fog. */
    int order = p->order;
    app.mx = 198;
    app.my = 26;
    ui_click(0);
    ticks(2);
    assert(p->order == order && dist(corpse, p->pos) < .001f);
    assert(!visible(g, 0, g->u[23].pos));
    camera = app.cam;
    ui_key('D', 1);
    ui_key('W', 1);
    ticks(10);
    assert(dist(camera, app.cam) < .05f); /* both coordinates clamped */
    ui_key('W', 0);
    ui_key('D', 0);
    ui_key('A', 1);
    ticks(5);
    ui_key(VK_ESCAPE, 1);
    ui_key(VK_ESCAPE, 0);
    camera = app.cam;
    ticks(12);
    assert(app.overlay == PAUSED && dist(camera, app.cam) < .001f);
    ui_key(VK_ESCAPE, 1);
    ui_key(VK_ESCAPE, 0);
    ui_key('A', 0);
    for (int i = 1; i < 10; i++)
        g->u[i].used = 0;
    p->respawn = .025f;
    ui_key('D', 1);
    ticks(3);
    assert(alive(p) && app.deathShade == 0 && app.panConsumed);
    assert(fabsf(app.cam.x - clampf(p->pos.x, 640 / app.zoom, WW - 640 / app.zoom)) < .05f);
    assert(fabsf(app.cam.y - clampf(p->pos.y, 360 / app.zoom, WH - 360 / app.zoom)) < .05f);
    V born = p->pos;
    ticks(8);
    assert(dist(p->pos, born) < .01f);
    ui_key('D', 0);
    ui_key('D', 1);
    ticks(8);
    assert(dist(p->pos, born) > 8);
    ui_key('D', 0);
    printf("Death veil: sampled world green channel %.0f%% of normal; HUD gold and kill card preserved. "
           "WASD, bounds, pause, fog and respawn passed.\n",
           100. * dim / lit);
}
static int green_sample(const uint32_t *pixels, int x, int y, int radius) {
    int value = 0;
    for (int j = -radius; j <= radius; j++)
        for (int i = -radius; i <= radius; i++)
            value += (pixels[(y + j) * W + x + i] >> 8) & 255;
    return value;
}
static void garrison_view(void) {
    ui_start(11);
    Game *g = &app.g;
    for (int i = 0; i < MAXU; i++)
        if (i != 13)
            g->u[i].used = 0;
    V fort = g->u[13].pos, tile = add(fort, vec(172, -108));
    app.cam = vec(clampf(fort.x, 640 / app.zoom, WW - 640 / app.zoom),
                  clampf(fort.y, 360 / app.zoom, WH - 360 / app.zoom));
    int x = (int)(640 + (tile.x - app.cam.x) * app.zoom), y = (int)(360 + (tile.y - app.cam.y) * app.zoom);
    int mx = (int)(25 + 174 * tile.x / WW), my = (int)(25 + 128 * tile.y / WH);
    game_step(g, 1.f / 60, (Input){0});
    assert(visible(g, 0, tile));
    ui_draw();
    capture(before);
    g->u[13].hp = 0;
    game_step(g, 1.f / 60, (Input){0});
    assert(!visible(g, 0, tile));
    ui_draw();
    capture(after);
    assert(green_sample(after, x, y, 4) < .8f * green_sample(before, x, y, 4));
    assert(green_sample(after, mx, my, 1) < .8f * green_sample(before, mx, my, 1));
    puts("Lone tower visibly clears both the main scene and minimap; both regain fog when it falls.");
}
static void tap_key(int k) {
    ui_key(k, 1);
    ui_key(k, 0);
}
static int cue_energy(void) {
    short mix[2048];
    int sum = 0;
    audio_capture_block(mix, 1024, 0);
    for (int i = 0; i < 2048; i++)
        sum += abs(mix[i]);
    return sum;
}
static void ceremony_checks(void) {
    assert(audio_capture_open());
    ui_reset();
    tap_key(VK_RETURN);
    audio_clear();
    app.mx = 110 + 164;
    app.my = 250;
    ui_click(0);
    assert(app.selected == 1 && cue_energy() > 100000);
    audio_clear();
    ui_click(0);
    assert(cue_energy() == 0);
    tap_key('3');
    assert(app.selected == 2 && cue_energy() > 100000);
    tap_key('M');
    tap_key(VK_RIGHT);
    assert(app.selected == 3 && cue_energy() == 0);
    tap_key('M');
    int first[10], different = 0;
    for (int seed = 1; seed <= 12; seed++) {
        ui_versus(seed);
        assert(app.lineup[0] == 0 && app.g.u[0].role == 3);
        int bits = 0;
        for (int i = 0; i < 10; i++) {
            int id = app.lineup[i];
            assert((unsigned)id < 10 && app.g.u[id].team == i / 5 && !(bits & (1 << id)));
            bits |= 1 << id;
        }
        if (seed == 1)
            memcpy(first, app.lineup, sizeof first);
        else
            different |= memcmp(first, app.lineup, sizeof first) != 0;
    }
    assert(different);
    ui_key('D', 1);
    tap_key('Q');
    tap_key(VK_RETURN);
    ui_click(0);
    ticks(90);
    assert(app.screen == VERSUS && app.g.time == 0 && app.g.waves == 0 && app.g.castCount[0] == 0);
    ui_draw();
    capture(before);
    ticks(15);
    ui_draw();
    capture(after);
    int pixels = 0;
    for (int y = 57; y <= 283; y++)
        for (int x = 148; x <= 343; x++)
            if (x < 155 || x > 337 || y < 64 || y > 276)
                pixels += change(before[y * W + x], after[y * W + x]) > 12;
    assert(pixels > 25);
    ticks(125);
    assert(app.screen == MATCH && app.g.time < .1f && !app.held['D']);
    for (int team = 0; team < 2; team++) {
        app.selected = 3;
        ui_start(148 + team);
        Game *g = &app.g;
        Unit *core = &g->u[16 + 7 * (1 - team)], *hero = &g->u[5 * team];
        g->u[10 + 7 * (1 - team)].hp = 0;
        core->hp = 1;
        hero->role = 3;
        hero->pos = add(core->pos, vec(team ? 72 : -72, 0));
        assert(game_cast(g, 5 * team, 0, core->pos) && g->winner == team);
        ticks(1);
        assert(app.screen == FINALE && app.sceneTime == 0);
        ticks(68);
        ui_draw();
        capture(before);
        assert(dist(app.cam, core->pos) < 65 && app.zoom > 2.3f && cue_energy() == 0);
        tap_key(VK_RETURN);
        tap_key(VK_F1);
        ui_click(0);
        assert(app.screen == FINALE && app.overlay == NONE && g->time == 0);
        ticks(51);
        ui_draw();
        capture(after);
        int shattered = 0;
        for (int y = 135; y < 475; y += 3)
            for (int x = 440; x < 840; x += 3)
                shattered += change(before[y * W + x], after[y * W + x]) > 18;
        assert(shattered > 1500 && cue_energy() > 100000);
        audio_clear(); /* Isolate the later fanfare from the fracture cue. */
        ticks(62);
        assert(app.sceneTime >= CREST_AT && app.sceneTime < CONTINUE_AT && cue_energy() > 10000);
        ui_click(0);
        assert(app.screen == FINALE);
        ticks(42);
        ui_draw();
        capture(after);
        if (team)
            tap_key(VK_RETURN);
        else
            ui_click(0);
        assert(app.screen == REPORT);
        tap_key(VK_ESCAPE);
        assert(app.screen == HOME);
    }
    puts("Ceremonies: audible mouse/key selections and mute; shuffled real lineups, live gold rim, frozen "
         "intro; both real core kills, fracture, delayed crest and continue passed.");
}
int main(void) {
    assert(SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)));
    CreateDirectoryW(L"build/gallery", NULL);
    ui_reset();
    assert(draw_init(NULL, W, H));
    layer_check();
    ui_key(VK_F1, 1);
    ui_key(VK_F1, 0);
    assert(app.overlay == GUIDE);
    app.my = 610;
    app.mx = 875;
    ui_click(0); /* Former left edge is no longer clickable. */
    assert(app.overlay == GUIDE);
    app.mx = 975;
    ui_click(0);
    assert(app.overlay == NONE);
    spectator_check();
    garrison_view();
    ceremony_checks();
    draw_close();
    CoUninitialize();
    return 0;
}
