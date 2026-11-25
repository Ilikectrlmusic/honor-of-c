/* Birth, recall, new lives and static defense observers in the real simulation. */
#include "../src/game.h"
#include <assert.h>
#include <stdio.h>

static Game g;
static void advance(int ticks, Input in) {
    while (ticks--)
        game_step(&g, 1.f / 120, in);
}
static Fx *attached(int id) {
    V pos;
    for (int i = 0; i < MAXFX; ++i) {
        Fx *f = &g.fx[i];
        if (f->kind == FX_ARRIVAL && f->anchor == id && game_fx_position(&g, f, &pos)) {
            assert(f->anchorSerial == g.u[id].serial && dist(pos, g.u[id].pos) < .001f);
            return f;
        }
    }
    return NULL;
}
static void arrivals(void) {
    game_init(&g, 0, 1, 51);
    V start[HEROES], pos;
    for (int i = 0; i < HEROES; i++) {
        start[i] = g.u[i].pos;
        assert(attached(i));
    }
    advance(65, (Input){1, 0, 0});
    int moved[2] = {0};
    for (int i = 0; i < HEROES; i++) {
        assert(attached(i));
        if (dist(start[i], g.u[i].pos) > 5)
            moved[g.u[i].team]++;
    }
    assert(dist(start[0], g.u[0].pos) > 20 && moved[0] > 1 && moved[1] > 0);
    for (int id = 0; id <= 5; id += 5) {
        game_init(&g, 3, 1, 51);
        g.wave = g.pay = 99;
        Unit *u = &g.u[id];
        for (int j = 0; j < MAXU; j++)
            if (j != id)
                g.u[j].used = 0;
        advance(190, (Input){0});
        u->pos = lane_at(1, .5f);
        u->hp = u->maxhp * .14f;
        u->lastCombat = -10;
        u->think = 0;
        if (!id)
            game_recall(&g);
        advance(2, (Input){0});
        assert(u->recall > 3.9f && !attached(id));
        advance(481, (Input){0});
        assert(u->recall <= 0 && dist(u->home, u->pos) < 10 && attached(id));
        Fx previous = *attached(id);
        u->hp = u->maxhp;
        u->retreat = 0;
        u->think = 0;
        V returned = u->pos;
        advance(45, (Input){1, 0, 0});
        assert(dist(returned, u->pos) > 10 && attached(id));
        u->hp = 0;
        u->respawn = .025f;
        assert(!game_fx_position(&g, &previous, &pos));
        advance(6, (Input){0});
        assert(alive(u) && attached(id));
        assert(!game_fx_position(&g, &previous, &pos));
        V born = u->pos;
        advance(50, (Input){1, 0, 0});
        assert(dist(born, u->pos) > 10 && attached(id));
    }
}
static void defenses(void) {
    /* Each team's outer tower and crystal alone: no hero or minion sight. */
    for (int team = 0; team < 2; team++)
        for (int core = 0; core < 2; core++) {
            game_init(&g, 2, 1, 99);
            g.wave = g.pay = 99;
            int id = (core ? 16 : 11) + team * 7;
            for (int i = 0; i < MAXU; i++)
                if (i != id)
                    g.u[i].used = 0;
            V p = g.u[id].pos;
            V q = add(p, vec((team ? -1 : 1) * (core ? 495 : 400), 0));
            int tile = (int)(q.y / 40) * GW + (int)(q.x / 40);
            advance(1, (Input){0});
            assert(visible(&g, team, q) && g.sight[team][tile] == 255 && !visible(&g, 1 - team, p));
            assert(g.sightTimer > .1f);
            g.u[id].hp = 0;
            advance(1, (Input){0});
            assert(!visible(&g, team, p) && !visible(&g, team, q));
        }
    /* Find an actual overlap between two friendly structures, then remove
       them in succession; the union must survive until the second is lost. */
    game_init(&g, 1, 1, 151);
    g.wave = g.pay = 99;
    for (int i = 0; i < MAXU; i++)
        if (i != 10 && i != 16)
            g.u[i].used = 0;
    V overlap = mul(add(g.u[10].pos, g.u[16].pos), .5f);
    advance(1, (Input){0});
    assert(visible(&g, 0, overlap));
    g.u[16].hp = 0;
    advance(1, (Input){0});
    assert(visible(&g, 0, overlap));
    g.u[10].hp = 0;
    advance(1, (Input){0});
    assert(!visible(&g, 0, overlap));
}
static void full_opening(void) {
    game_prepare_match(&g, 2, 1, 198);
    for (int i = 0; i < 10; i++)
        assert(!alive(&g.u[i]) && !attached(i));
    advance(500, (Input){1, 0, 1});
    assert(g.time == 0 && g.soundN == 0 && !g.waves);
    game_deploy(&g);
    for (int i = 0; i < 10; i++)
        assert(alive(&g.u[i]) && attached(i));
    int warning = 0, march = 0;
    for (int i = 0; i < 820; i++) {
        g.soundN = 0;
        advance(1, (Input){0});
        for (int j = 0; j < g.soundN; j++) {
            if (g.sound[j] == VO_COUNTDOWN) {
                assert(!warning && g.wave <= 5 && g.wave > 4.98f && !g.waves);
                for (int k = 0; k < 5; k++)
                    assert(!attached(k));
                warning = i;
            }
            if (g.sound[j] == VO_MARCH) {
                assert(g.waves == 1 && warning && !march);
                march = i;
            }
        }
    }
    assert(march - warning >= 599 && march - warning <= 601);
}
static void repeat_skills(void) {
    for(int role=0;role<HERO_COUNT;role++){
        game_training(&g,role);
        Unit *p=&g.u[0],*d=&g.u[5];
        V still=d->pos;
        for(int pass=0;pass<2;pass++)for(int slot=0;slot<3;slot++){
            p->pos=add(still,vec(-78,0));
            assert(game_cast(&g,0,slot,still));
            assert(p->cd[slot]==0);
            advance(140,(Input){0});
            assert(g.castCount[slot]==pass+1 && dist(d->pos,still)==0 && alive(d));
        }
        assert(g.practiceHits && g.practiceDamage>0 && !g.waves && g.winner<0);
        assert(!walkable(&g,vec(1220,660)) && walkable(&g,p->home));
    }
}
int main(void) {
    repeat_skills();
    full_opening();
    arrivals();
    defenses();
    puts("Observer simulation: moving births on both teams, player and AI recalls/respawns, stale life "
         "rejection, isolated tower/core sight and overlap passed.");
    return 0;
}
