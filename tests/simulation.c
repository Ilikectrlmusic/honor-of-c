#include "../src/game.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
static Game g;
static Input idle = {0};
/* Keep one caster and one durable target on an unobstructed stretch of mid lane. */
static void range_test(int role, int slot) {
    game_init(&g, role, 1, 7);
    for (int i = 1; i < MAXU; i++)
        if (i != 5)
            g.u[i].used = 0;
    Unit *p = &g.u[0], *v = &g.u[5];
    p->pos = lane_at(1, .45f);
    p->hp -= 200;
    p->atk = 99;
    v->pos = add(p->pos, vec(80, -44));
    v->hp = v->maxhp = 3000;
    v->stun = 3;
    V start = p->pos;
    assert(game_cast(&g, 0, slot, v->pos));
    for (int i = 0; i < 61; i++)
        game_step(&g, 1.f / 60, idle);
    if (role == 1 && slot == 1) {
        assert(p->haste > 0 && dist(start, p->pos) > 60);
    } else if (role == 3 && slot == 1) {
        assert(p->shield > 160);
    } else
        assert(v->hp < 2935);
    assert(g.castCount[slot] == 1);
    assert(!game_cast(&g, 0, slot, v->pos));
}
int main(void) {
    for (int role = 0; role < 5; role++)
        for (int s = 0; s < 3; s++)
            range_test(role, s);
    for (int role = 0; role < 5; role++) {
        game_init(&g, role, 1, 17);
        int side[2][5] = {{0}};
        for (int i = 0; i < 10; i++)
            side[g.u[i].team][g.u[i].role]++;
        for (int t = 0; t < 2; t++)
            for (int j = 0; j < 5; j++)
                assert(side[t][j] == 1);
        for (int j = 0; j < 3; j++) {
            assert(game_cast(&g, 0, j, vec(600, 1700)));
            assert(!game_cast(&g, 0, j, vec(600, 1700)));
        }
    }
    game_init(&g, 2, 1, 19);
    assert(protected_building(&g, 23));
    assert(protected_building(&g, 17));
    g.u[18].hp = 0;
    assert(!protected_building(&g, 17));
    g.u[17].hp = 0;
    assert(!protected_building(&g, 23));
    assert(!game_buy(&g, 0, 0));
    g.u[0].gold = 9999;
    for (int j = 0; j < 4; j++)
        assert(game_buy(&g, 0, 0));
    assert(!game_buy(&g, 0, 0));
    game_init(&g, 0, 1, 18);
    V orig = g.u[0].pos;
    game_order(&g, vec(820, 1020), -1, 0);
    for (int s = 0; s < 600; s++)
        game_step(&g, 1.f / 60, idle);
    assert(dist(orig, g.u[0].pos) > 500);
    assert(walkable(&g, g.u[0].pos));
    game_recall(&g);
    game_step(&g, .05f, (Input){1, 0, 0});
    assert(g.u[0].recall <= 0);
    g.u[0].pos = lane_at(1, .14f);
    game_recall(&g);
    for (int s = 0; s < 260; s++)
        game_step(&g, 1.f / 60, idle);
    assert(dist(g.u[0].pos, g.u[0].home) < 35);
    game_init(&g, 3, 1, 89);
    assert(visible(&g, 0, g.u[0].pos));
    assert(!visible(&g, 0, g.u[23].pos));
    game_rally(&g, lane_at(1, .3f));
    assert(g.rallyTime > 0 && g.rallyIds[0] > 0 && g.rallyIds[1] > 0 && g.rallyIds[1] != g.rallyIds[0]);
    V mark = g.rally;
    game_rally(&g, lane_at(1, .8f));
    assert(dist(mark, g.rally) < 1);
    for (int n = 0; n < 3; n++) {
        game_init(&g, n, n, 150 + n);
        g.autoplay = 1;
        float dt = n == 1 ? 1.f / 60 : .05f;
        for (int s = 0; s < (int)(1200 / dt) && g.winner < 0; s++) {
            game_step(&g, dt, idle);
            g.soundN = 0;
            if (s % 120 == 0) {
                for (int h = 0; h < 10; h++) {
                    assert(isfinite(g.u[h].hp) && g.u[h].gold >= 0 && g.u[h].level <= 15);
                    assert(walkable(&g, g.u[h].pos));
                }
            }
        }
        printf("seed %d | winner %d | %.0fs | kills %d:%d | towers %d:%d | waves %d\n", 150 + n, g.winner,
               g.time, g.kills[0], g.kills[1], g.towers[0], g.towers[1], g.waves);
        fflush(stdout);
        assert(g.winner >= 0);
        assert(g.towers[g.winner] >= 2);
    }
    puts("simulation checks passed");
    return 0;
}
