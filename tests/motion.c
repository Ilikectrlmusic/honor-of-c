#include "../src/game.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static Game g;
static const float tick = 1.f / 120;
static const V start = {1250, 1925};
static const Input still = {0};

static void duel(int role) {
    game_init(&g, role, 1, 103);
    g.wave=0; game_step(&g,tick,still); /* create a real troop for the attack tests */
    for (int i = 1; i < MAXU; i++)
        if (i != 5)
            g.u[i].used = 0;
    g.u[0].pos = start;
    g.u[5].pos = add(start, vec(52, 0));
    g.u[5].hp = g.u[5].maxhp = 5000;
    g.u[5].stun = 999;
    g.wave = 999;
}
static int effects(int kind, int model) {
    int n = 0;
    for (int i = 0; i < MAXFX; i++)
        n += g.fx[i].live && g.fx[i].kind == kind && g.fx[i].n == model;
    return n;
}
static int projectiles(int id) {
    int n = 0;
    for (int i = 0; i < MAXP; i++)
        n += g.shot[i].live && g.shot[i].owner == id;
    return n;
}

static void hit_frame(int role, int kind) {
    duel(role);
    int id = 0;
    if (kind != HERO) {
        id = MOB0;
        Unit *u = &g.u[id];
        u->used = 1;
        u->pos = start;
        u->team = 0;
        u->kind = kind;
        u->think = 0;
        u->reach = kind == MELEE ? 55 : 180;
        u->period = 1.2f;
        g.u[0].used = 0;
    }
    Unit *u = &g.u[id], *v = &g.u[5];
    float hp = v->hp;
    game_step(&g, tick, still);
    assert(u->pose == POSE_ATTACK && u->swingTarget == 5 && u->anim > 0);
    while (u->releaseIn > tick * 1.01f) {
        assert(v->hp == hp && !projectiles(id));
        game_step(&g, tick, still);
    }
    assert(v->hp == hp);
    for (int i = 0; i < 2 && u->swingTarget >= 0; i++)
        game_step(&g, tick, still);
    assert(u->swingTarget < 0 && u->anim > 0);
    assert(u->reach < 140 ? v->hp < hp : projectiles(id) > 0);
    for (int i = 0; i < 30; i++)
        game_step(&g, tick, still);
    assert(v->hp < hp);
}

int main(void) {
    for (int role=0;role<5;role++) {
        duel(role);
        game_order(&g,g.u[5].pos,5,0);
        for(int i=0;i<40;i++) {
            V prev=g.u[0].pos;
            game_step(&g,tick,(Input){0,1,1});
            assert(g.u[0].order==2 && g.u[0].target==5);
            assert(g.u[0].pos.y>prev.y && fabsf(g.u[0].pos.x-start.x)<.01f);
            if (i==3) assert(g.u[0].pose==POSE_ATTACK && g.u[0].swingTarget==5);
        }
        assert(g.u[5].hp<5000 || projectiles(0)>0);
    }
    for (int i = 0; i < 5; i++)
        hit_frame(i, HERO);
    hit_frame(0, MELEE);
    hit_frame(0, RANGED);
    hit_frame(0, SIEGE);
    for (int mode = 0; mode < 3; mode++) {
        duel(0);
        game_step(&g, tick, still);
        assert(g.u[0].swingTarget == 5);
        if (mode == 0)
            g.u[0].stun = 1;
        else if (mode == 1)
            g.u[5].serial++;
        else {
            g.u[0].hp = 0;
            g.u[0].respawn = 3;
        }
        for (int i = 0; i < 50; i++)
            game_step(&g, tick, still);
        assert(g.u[5].hp == 5000); /* No stale contact after CC, slot reuse, or death. */
    }
    duel(0);
    g.u[5].used = 0;
    float phase = g.u[0].stride;
    game_step(&g, .05f, (Input){1, 0, 0});
    float d = dist(g.u[0].pos, start);
    assert(d > 0 && fabsf(fmodf(phase + d / 88, 1) - g.u[0].stride) < .0001f);
    phase = g.u[0].stride;
    for (int i = 0; i < 20; i++)
        game_step(&g, tick, still);
    assert(g.u[0].moving <= 0 && g.u[0].stride == phase);
    /* Start on a walkable tile just inside its right edge, facing a solid tile. */
    g.u[0].pos = vec(1279.9f, 1940);
    g.nav[48 * GW + 31] = 1;
    g.nav[48 * GW + 32] = 0;
    V blocked = g.u[0].pos;
    int frames = 0;
    for (int i = 0; i < 120; ++i) {
        game_step(&g, tick, (Input){1, 0, 0});
        assert(dist(blocked, g.u[0].pos) == 0 && g.u[0].moving > 0 && g.u[0].dir == 1);
        frames |= 1 << (int)(g.u[0].stride * 6);
    }
    assert(frames == 63);
    phase = g.u[0].stride;
    for (int i = 0; i < 20; i++) game_step(&g, tick, still);
    assert(g.u[0].moving <= 0 && g.u[0].stride == phase);
    g.u[0].stun = .3f;
    game_step(&g, tick, (Input){1,0,0});
    assert(g.u[0].moving <= 0 && g.u[0].stride == phase);
    for (int role = 0; role < 5; role++) {
        duel(role);
        Unit *u = &g.u[0], *v = &g.u[5];
        u->atk = 99;
        assert(game_cast(&g, 0, 2, v->pos));
        assert(effects(FX_ULT_CAST, role) == 1);
        assert(!game_cast(&g, 0, 0, v->pos) && u->pose == POSE_ULT);
        assert(v->hp == 5000);
        int impact = 0, airborne = 0;
        for (int n = 0; n < 125; n++) {
            game_step(&g, tick, still);
            impact |= effects(FX_ULT_IMPACT, role);
            airborne |= u->ultWindup > 0 && dist(u->pos, start) > 5;
        }
        assert(impact && v->hp < 5000);
        if (role == 3)
            assert(airborne);
    }
    duel(0);
    assert(game_cast(&g, 0, 2, g.u[5].pos));
    g.u[0].stun = 1;
    for (int i = 0; i < 70; i++)
        game_step(&g, tick, still);
    assert(!projectiles(0) && g.u[5].hp == 5000);
    puts("motion checks passed: strides, 8 attack contacts, cancellation and 5 distinct ultimate timelines");
    return 0;
}
