/* Combat feedback tests exercise real start, attack, damage, gold and UI events. */
#include "../src/game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static const float dt = 1.f / 120;
static void steps(int n) {
    while (n--)
        game_step(&g, dt, (Input){0});
}
static int count(int kind, int val) {
    int n = 0;
    for (int i = 0; i < MAXFX; i++)
        if (g.fx[i].live && g.fx[i].kind == kind && (val < 0 || g.fx[i].n == val))
            n++;
    return n;
}
static int heard(int id) {
    int n = 0;
    for (int j = 0; j < g.soundN; j++)
        n += g.sound[j] == id;
    return n;
}
static void camp(void) {
    game_init(&g, 3, 1, 51);
    for (int j = 1; j < MAXU; j++)
        if (j != 24)
            g.u[j].used = 0;
    g.u[0].pos = add(g.u[24].home, vec(58, 0));
    g.u[0].atk = 99;
    g.wave = g.pay = 99;
    steps(1);
    assert(g.u[24].swingTarget == 0);
}
static void announcer_checks(void) {
    /* Real lethal hits, including all four local perspectives and both teams'
       multikill/streak banks. Seed only the previous history for milestones. */
    for (int team=0;team<2;team++) for(int state=0;state<13;state++){
        game_init(&g,3,1,51); g.time=50; g.firstBlood=1;
        int killer=team?5:state==0?0:1;
        int victim=team?(state==0?0:1):5;
        Unit *k=&g.u[killer], *v=&g.u[victim];
        k->role=3;k->pos=lane_at(1,.5f);v->pos=add(k->pos,vec(50,0));v->hp=1;
        int expect;
        if (state<2) expect=team?(state?VO_ENEMYKILL:VO_PLAYERDEATH):(state?VO_ALLYKILL:VO_PLAYERKILL);
        else if(state<=5){
            k->multi=state-1;k->spree=state-1;k->lastKillAt=49.f;
            expect=(team?VO_ENEMY_DOUBLE:VO_DOUBLE)+state-2;
        } else if(state<=10) {
            k->spree=state-4;k->multi=2;k->lastKillAt=35.f;
            expect=(team?VO_ENEMY_SPREE:VO_SPREE)+state-6;
        } else {
            v->spree=4;expect=VO_SHUTDOWN;
            if(state==12)for(int j=5*v->team;j<5*v->team+5;j++)if(j!=victim)g.u[j].hp=0;
        }
        g.soundN=0;
        assert(game_cast(&g,killer,0,v->pos));
        assert(heard(expect) && g.cards[0].voice==expect && !v->spree && !v->multi);
        assert(heard(VO_ACED)==(state==12));
    }
}
int main(void) {
    announcer_checks();
    game_init(&g, 3, 1, 51);
    assert(count(FX_ARRIVAL, -1) == 10 && heard(VO_WELCOME) == 1 && !heard(VO_COUNTDOWN) && g.waves == 0);
    assert(!g.feed[0].msg[0]);
    steps(241);
    assert(heard(VO_COUNTDOWN)==1 && g.wave>4.98f && g.wave<=5 && !g.feed[0].msg[0]);
    steps(598);
    assert(!g.waves);
    steps(2);
    assert(g.waves == 1 && heard(VO_MARCH) == 1);
    assert(!g.feed[0].msg[0]);
    /* A planted fist actually misses when the target leaves the locked circle.
       During tracking it follows part, not all, of a target's lateral move. */
    for (int mode = 0; mode < 5; mode++) {
        camp();
        Unit *m = &g.u[24], *p = &g.u[0];
        float hp = p->hp;
        V feet = m->pos, old = m->stoneAim;
        p->pos.y += 26;
        steps(1);
        assert(m->stoneAim.y > old.y && m->stoneAim.y < p->pos.y - 8);
        steps(40);
        V locked = m->stoneAim;
        assert(m->releaseIn > 0 && hp == p->hp);
        if (mode == 1)
            p->pos.y += 94;
        if (mode == 2)
            m->stun = 1;
        if (mode == 3)
            p->serial++;
        if (mode == 4)
            m->aggro = 0;
        steps(41);
        assert(dist(locked, m->stoneAim) < .01f && dist(feet, m->pos) < .1f && hp == p->hp);
        steps(10);
        assert((p->hp < hp) == (mode == 0));
        assert(count(FX_EARTH, -1) == (mode < 2));
    }
    /* Interrupted return must not manufacture an arrival beacon. */
    camp();
    g.u[24].used = 0;
    steps(190);
    assert(!count(FX_ARRIVAL, -1));
    game_recall(&g);
    game_step(&g, dt, (Input){1, 0, 0});
    steps(5);
    assert(!count(FX_ARRIVAL, -1) && g.u[0].recall <= 0);
    game_recall(&g);
    steps(481);
    assert(dist(g.u[0].pos, g.u[0].home) < 5 && count(FX_ARRIVAL, -1) == 1);
    steps(210);
    g.u[0].hp = 0;
    g.u[0].respawn = .035f;
    steps(6);
    assert(alive(&g.u[0]) && count(FX_ARRIVAL, -1) == 1);
    /* First Blood uses the credited killer. A second simultaneous kill queues
       behind it; rally is a different channel. Gold FX match each real bounty. */
    game_init(&g, 3, 1, 912);
    g.wave = 0;
    steps(1);
    for (int j = 1; j < MAXU; j++)
        if (j != 1 && j != 5 && j != 6 && j != 24 && j != MOB0 + 9)
            g.u[j].used = 0;
    V a = g.u[24].home;
    g.u[0].pos = a;
    g.u[1].pos = g.u[0].home;
    int ids[] = {5, 6, 24, MOB0 + 9};
    for (int j = 0; j < 4; j++) {
        g.u[ids[j]].pos = add(a, vec(58, j * 6));
        g.u[ids[j]].hp = 1;
    }
    assert(g.u[MOB0 + 9].team == 1);
    int start = g.u[0].gold;
    g.soundN = 0;
    assert(game_cast(&g, 0, 0, a));
    assert(g.u[0].gold - start == 180 * 2 + 115 + 30);
    assert(count(FX_COINS, 180) == 2 && count(FX_COINS, 115) == 1 && count(FX_COINS, 30) == 1);
    assert(heard(S_COIN) == 1 && heard(VO_FIRST) == 1 && heard(VO_DOUBLE) == 1);
    assert(g.cardCount == 2 && g.cards[0].first && !g.cards[1].first);
    assert(g.cards[0].sourceRole == 3 && g.cards[0].victimRole == g.u[5].role);
    game_rally(&g, a);
    assert(heard(VO_RALLY) == 1 && wcsstr(g.feed[0].msg, L"请求集合"));
    game_rally(&g, a);
    assert(heard(VO_RALLY) == 1 && g.cardCount == 2);
    steps(310);
    assert(g.cardCount == 1 && !g.cards[0].first);
    puts("feedback checks passed: tracking/lock/dodge, interrupt, 3 arrivals, delayed wave, kill cards, "
         "rally, rewards and speech events");
    return 0;
}
