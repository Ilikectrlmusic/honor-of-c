/* Live fixed-tick probes. No privileged damage call and no human auto-attack
   in the E tests; an unrelated basic must not make a broken skill pass. */
#include "../src/game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static void step(int n){while(n--)game_step(&g,1.f/120,(Input){0});}
static void all_es(void){
    for(int r=0;r<HERO_COUNT;r++){
        for(int near=0;near<=1;near++){
            game_training(&g,r);
            Unit *p=&g.u[0],*d=&g.u[5];
            p->pos=vec(230,325);d->pos=vec(near?315:1120,325);
            V start=p->pos;float hp=d->hp;
            p->hp-=220;
            assert(game_cast(&g,0,1,d->pos));
            if(r==1)assert(dist(p->pos,start)>100 && p->haste>0);
            if(r==3||r==5)assert(p->shield>100);
            if(r==6){assert(p->hp>p->maxhp-115);assert(d->hp==hp);}
            step(130);
            assert(g.castCount[1]==1 && g.practiceHits==near);
            assert(near?(d->hp<hp && g.practiceDamage>0):(d->hp==hp && !g.practiceDamage));
        }
    }
    /* A wind arrow has a direction, not an invisible auto-hit. */
    game_training(&g,1);
    g.u[0].pos=vec(380,325);g.u[5].pos=vec(305,325);
    assert(game_cast(&g,0,1,vec(900,325)));step(130);assert(!g.practiceHits);
    /* At its default spawn distance Shan Yin's cursor echo reaches the dummy;
       a target that leaves the telegraph before it resolves is safe. */
    for(int dodge=0;dodge<2;dodge++){
        game_training(&g,6);
        assert(game_cast(&g,0,1,g.u[5].pos));
        if(dodge)g.u[5].pos.y+=140;
        step(50);assert(g.practiceHits==!dodge);
    }
    game_training(&g,5);
    g.u[0].pos=vec(250,325);g.u[5].pos=vec(385,325);
    Unit *second=&g.u[6];second->used=1;second->pos=vec(475,325);second->grace=0;
    float old=second->hp;
    assert(game_cast(&g,0,1,g.u[5].pos) && g.practiceHits==1 && second->hp<old);
    puts("E: all 10 hit in range, all 10 miss out of range; mobility, shield, heal, echo dodge and chain checked.");
}
/* A blue human opponent and one red bot in a clear part of mid. Freeze the
   vision masks in these fixtures so hidden/visible is controlled by the test. */
static void duel(int seed,int role) {
    game_init(&g,2,2,seed);
    for(int j=1;j<MAXU;j++)if(j!=5)g.u[j].used=0;
    g.wave=g.pay=g.sightTimer=999;g.garrisonMask=0;
    memset(g.sight,255,sizeof g.sight);
    Unit *u=&g.u[5],*v=&g.u[0];u->role=role;u->mode=1;u->think=0;
    u->reach=heroes[role].reach;u->speed=heroes[role].speed;
    u->pos=lane_at(1,.50f);v->pos=add(u->pos,vec(125,-70));
    v->atk=500;v->hp=v->maxhp=3500;
}
static void tactics(void){
    duel(51,2);Unit *u=&g.u[5],*v=&g.u[0];
    memset(g.sight[1],0,sizeof g.sight[1]);
    step(15);assert(u->target==-1 && u->cd[0]==0 && u->cd[1]==0);
    memset(g.sight[1],255,sizeof g.sight[1]);u->think=0;step(1);
    assert(u->target==0 && g.bot[5].readyAt>g.time && u->swingTarget<0 && !u->cd[1]);
    step(40);
    assert(u->cd[1]>0 && !u->cd[0] && !u->cd[2]); /* control first, not three skills on one frame */
    memset(g.sight[1],0,sizeof g.sight[1]);step(1);assert(u->target<0);
    int dodged=0,missed=0;
    for(int seed=1;seed<=30;seed++){
        duel(seed,4);V initial=u->pos;
        v->pos=add(initial,vec(220,-45));
        assert(game_cast(&g,0,2,initial)); /* a real delayed enemy meteor */
        step(1);
        if(g.bot[5].evadeUntil>0){
            assert(g.bot[5].evadeAt>g.time && dist(initial,u->pos)<.05f);
            step(38); assert(dist(initial,u->pos)>7 && walkable(&g,u->pos));
            assert(dist(u->pos,g.bot[5].escape)<dist(initial,g.bot[5].escape));
            dodged++;
        }else missed++;
    }
    assert(dodged && missed);
    duel(11,6);
    v->pos=vec(270,1910); /* injured teammate is just outside the healing ring */
    Unit *ally=&g.u[6];ally->used=1;ally->pos=add(u->pos,vec(282,-43));
    ally->stun=10;ally->hp=ally->maxhp*.45f;float wounded=ally->hp;
    step(1);assert(g.bot[5].careSlot==7 && u->cd[1]==0);
    step(145);assert(u->cd[1]>0 && ally->hp>wounded+90);
    duel(4,1);u->reach=280;v->reach=80;
    u->cd[0]=u->cd[1]=u->cd[2]=100;
    V at=u->pos;float begin=dist(u->pos,v->pos);
    step(95);
    assert(v->hp<v->maxhp && dist(at,u->pos)>12 && dist(u->pos,v->pos)>begin);
    /* A guaranteed last hit competes with, but isn't lost to, a closer
       healthy minion. Positions, levels and AI cooldowns otherwise identical. */
    duel(8,1);v->pos=vec(270,1910);
    for(int j=MOB0;j<MOB0+2;j++){
        Unit *m=&g.u[j];memset(m,0,sizeof *m);
        m->used=1;m->serial=300+j;m->kind=MELEE;m->team=0;m->stun=5;
        m->pos=add(u->pos,vec(j==MOB0?74:125,0));
        m->maxhp=250;m->hp=j==MOB0?250:13;
    }
    step(1);assert(u->target==MOB0+1);
    printf("Bots: vision, delayed reaction, spaced control combo, last-hit and kiting OK; responded to %d/30 visible tells.\n",dodged);
}
int main(void){all_es();tactics();return 0;}
