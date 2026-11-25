#include "game.h"
#include "river_mask.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define JADE 0x6bdfbf
#define GOLD 0xf2cc81
#define RED 0xff876b
const uint32_t teamColor[3] = {0x61dcd2, 0xfc7b68, 0xe7bd70};
const HeroDef heroes[HERO_COUNT] = {
    {L"烬锋",
     L"战士",
     L"近战突进",
     L"赤刃开道，以连斩撕开敌阵。",
     {L"掠火", L"回锋", L"焚天斩"},
     {L"向指针突进，对沿途敌人斩击。", L"横扫周身，命中时回复生命。", L"向前劈出宽阔剑气，击晕沿线敌人。"},
     0xf28764,
     1010,
     73,
     88,
     184,
     .82f,
     {5, 8, 22}},
    {L"青翎",
     L"射手",
     L"远程持续",
     L"以游走和连射，牵制整条兵线。",
     {L"穿林箭", L"逐风", L"流星箭雨"},
     {L"向指针射出可穿透敌人的利箭。", L"翻滚时朝指针射出一支风矢，并短时提高攻速和移速。",
      L"在指定区域降下箭雨，造成伤害与减速。"},
     0x85d9a0,
     755,
     69,
     285,
     177,
     .78f,
     {4.5f, 8, 21}},
    {L"望舒",
     L"法师",
     L"范围控制",
     L"借月辉封锁河道，在远处掌控战局。",
     {L"月蚀", L"星缚", L"天星坠"},
     {L"发射碰撞后爆裂的月球。", L"短暂延迟后束缚指定区域的敌人。",
      L"在目标区域召唤延时坠星，造成高额伤害。"},
     0xbba3ff,
     730,
     61,
     265,
     168,
     1.02f,
     {5, 7.5f, 23}},
    {L"镇岳",
     L"坦克",
     L"前排守护",
     L"以重盾守住阵线，为同伴争取时间。",
     {L"撼地", L"金城", L"山崩"},
     {L"盾击周围敌人，并使其减速。", L"为自身及附近友军加盾，同时用盾震打击身边敌人。", L"向指针跃进，震晕落点周围的敌人。"},
     0xe2b86c,
     1330,
     66,
     85,
     162,
     .98f,
     {5, 9, 23}},
    {L"夜渡",
     L"刺客",
     L"机动爆发",
     L"双刃穿过阴影，收割落单的对手。",
     {L"瞬影", L"寒刃", L"断夜"},
     {L"快速突进并切割路径上的敌人。", L"投出冰刃，命中后重度减速。",
      L"闪至指针附近，对身边残血敌人造成额外伤害。"},
     0x83c9ff,
     810,
     80,
     83,
     199,
     .75f,
     {4, 6, 19}},
    {L"玄霆", L"战士", L"雷枪破阵", L"枪尖引下天雷，以护身电甲冲开阵线。",
     {L"贯雷", L"惊蛰甲", L"九霄落雷"},
     {L"掷出一道贯穿敌人的雷枪。", L"披上电甲、短时加速；电弧从身边目标向另一名敌人跳跃。", L"在指针处刻下雷印，落雷重创并击晕范围内敌人。"},
     0xa99cff, 1080, 72, 104, 178, .90f, {5, 9, 22}},
    {L"珊音", L"辅助", L"潮声群愈", L"一弦唤潮，一曲回春。海上归舟皆识此音。",
     {L"弦波", L"回澜曲", L"三叠潮"},
     {L"弹出穿透声波，使命中的敌人减速。", L"为身边友军及自己回血，在指针处奏响一次延时伤敌的回音。", L"向指定地点推出三重渐进的潮浪，连续造成伤害与减速。"},
     0x75e5d4, 860, 59, 248, 175, .94f, {5, 8, 23}},
    {L"丹砂", L"法师", L"丹火炼法", L"拂尘拢袖，指间起火。一葫芦金丹，半是药，半是雷。",
     {L"流火丸", L"护炉焰", L"天炉散星"},
     {L"投出火丸，碰敌爆裂并波及附近目标。", L"升起护盾，爆开近身炉焰并使敌人减速。", L"在目标附近布下三枚依次引爆的丹火。"},
     0xffad65, 845, 66, 239, 166, 1.02f, {5.5f, 9, 24}},
    {L"雪翎", L"射手", L"霜弓游击", L"雪过无痕，唯有弦响时，冰花才在敌阵盛开。",
     {L"三羽箭", L"踏雪", L"寒晶绽"},
     {L"向扇形区域射出三支霜箭；近身可多箭命中。", L"向指针滑步，留下霜环减速追兵。", L"在指定地点绽开冰晶，冻结范围内的敌人。"},
     0x95dfff, 745, 69, 298, 181, .82f, {5, 7, 22}},
    {L"墨衡", L"法师", L"墨阵封路", L"落笔成阵，收锋化龙。战场便是未干的长卷。",
     {L"飞白", L"困字诀", L"游龙入墨"},
     {L"挥笔打出贯穿敌人的墨锋。", L"在目标处写下墨圈，短暂延迟后定住敌人。", L"沿笔锋方向依次爆开五段墨龙，重叠处可多次命中。"},
     0xb5cfbb, 780, 64, 255, 173, .98f, {4.5f, 8, 23}}};
const wchar_t *itemNames[3] = {L"破军刃", L"玄甲", L"追风履"};
const wchar_t *itemDesc[3] = {L"攻击 +18，普攻和技能均受益", L"生命上限 +210，立即回复 210",
                              L"移速 +12，攻击间隔缩短 7%"};
V vec(float x, float y) {
    V v = {x, y};
    return v;
}
V add(V a, V b) {
    return vec(a.x + b.x, a.y + b.y);
}
V sub(V a, V b) {
    return vec(a.x - b.x, a.y - b.y);
}
V mul(V a, float f) {
    return vec(a.x * f, a.y * f);
}
float lengthv(V a) {
    return sqrtf(a.x * a.x + a.y * a.y);
}
float dist(V a, V b) {
    return lengthv(sub(a, b));
}
V norm(V a) {
    float l = lengthv(a);
    return l > .001f ? mul(a, 1 / l) : vec(0, 0);
}
float clampf(float x, float lo, float hi) {
    return x < lo ? lo : x > hi ? hi : x;
}
static float dot(V a, V b) {
    return a.x * b.x + a.y * b.y;
}
static float segdist(V p, V a, V b) {
    V ab = sub(b, a);
    float t = clampf(dot(sub(p, a), ab) / fmaxf(dot(ab, ab), 1), 0, 1);
    return dist(p, add(a, mul(ab, t)));
}
static const V upper[] = {{270, 1910}, {340, 1540}, {342, 1080}, {375, 610},  {535, 390},
                          {935, 310},  {1530, 275}, {2160, 275}, {2500, 288}, {2730, 290}};
static const V mid[] = {{270, 1910}, {625, 1665}, {990, 1455}, {1260, 1285}, {1500, 1100},
                        {1740, 915}, {2010, 745}, {2375, 535}, {2730, 290}};
static const V river[] = {{-100, -40},  {375, 195},   {690, 455},   {1010, 780}, {1480, 1100},
                          {1860, 1400}, {2200, 1650}, {2530, 1930}, {3080, 2240}};
static const V trails[][4] = {{{368, 880}, {800, 1020}, {1100, 680}, {1470, 310}},
                              {{720, 1610}, {800, 1020}, {1150, 1290}, {1150, 1290}},
                              {{1510, 1900}, {1920, 1560}, {2160, 1180}, {2600, 1340}},
                              {{2170, 605}, {2160, 1180}, {1810, 960}, {1810, 960}}};
static const V camps[] = {{800, 1020}, {1100, 680}, {1920, 1560}, {2160, 1180}};
static const V bases[] = {{270, 1910}, {2730, 290}};
static int nway(int l) {
    return l == 1 ? 9 : 10;
}
static V waypoint(int l, int i) {
    if (l == 1)
        return mid[i];
    if (l == 0)
        return upper[i];
    return vec(WW - upper[9 - i].x, WH - upper[9 - i].y);
}
V lane_at(int l, float t) {
    float total = 0;
    for (int i = 1; i < nway(l); i++)
        total += dist(waypoint(l, i - 1), waypoint(l, i));
    t = clampf(t, 0, 1) * total;
    for (int i = 1; i < nway(l); i++) {
        V a = waypoint(l, i - 1), b = waypoint(l, i);
        float len = dist(a, b);
        if (t <= len)
            return add(a, mul(sub(b, a), t / len));
        t -= len;
    }
    return waypoint(l, nway(l) - 1);
}
static float rnd(Game *g) {
    g->rng ^= g->rng << 13;
    g->rng ^= g->rng >> 17;
    g->rng ^= g->rng << 5;
    return (g->rng >> 8) * (1.f / 16777216.f);
}
static void sound(Game *g, int s) {
    if (g->soundN < 16)
        g->sound[g->soundN++] = s;
}
static void feed(Game *g, const wchar_t *msg, uint32_t c) {
    memmove(g->feed + 1, g->feed, 3 * sizeof(Feed));
    swprintf(g->feed[0].msg, 100, L"%ls", msg);
    g->feed[0].time = g->time;
    g->feed[0].color = c;
}
void game_notice(Game *g, const wchar_t *msg) {
    swprintf(g->notice, 100, L"%ls", msg);
    g->noticeTime = 2.4f;
}
int alive(const Unit *u) {
    return u->used && u->hp > 0;
}
int hostile(const Unit *a, const Unit *b) {
    return a->team != b->team;
}
static int structure(const Unit *u) {
    return u->kind == TOWER || u->kind == CORE;
}
int protected_building(const Game *g, int id) {
    const Unit *u = &g->u[id];
    if (u->kind == TOWER && u->tier == 0)
        return alive(&g->u[id + 1]);
    if (u->kind == CORE) {
        for (int l = 0; l < 3; l++)
            if (!alive(&g->u[10 + u->team * 7 + l * 2]))
                return 0;
        return 1;
    }
    return 0;
}
static void nav_init(Game *g) {
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++) {
            V p = vec(x * 40 + 20, y * 40 + 20);
            int ok = dist(p, bases[0]) < 220 || dist(p, bases[1]) < 220;
            for (int l = 0; l < 3 && !ok; l++)
                for (int j = 1; j < nway(l); j++)
                    if (segdist(p, waypoint(l, j - 1), waypoint(l, j)) < 99)
                        ok = 1;
            for (int j = 1; j < 9 && !ok; j++)
                if (segdist(p, river[j - 1], river[j]) < 111)
                    ok = 1;
            for (int l = 0; l < 4 && !ok; l++) {
                if (dist(p, camps[l]) < 116)
                    ok = 1;
                for (int j = 1; j < 4; j++)
                    if (segdist(p, trails[l][j - 1], trails[l][j]) < 66)
                        ok = 1;
            }
            g->nav[y * GW + x] = (unsigned char)ok;
        }
}
static int cell(V p) {
    int x = (int)clampf(floorf(p.x / 40), 0, GW - 1), y = (int)clampf(floorf(p.y / 40), 0, GH - 1);
    return y * GW + x;
}
static V center(int i) {
    return vec((i % GW) * 40 + 20, (i / GW) * 40 + 20);
}
int visible(const Game *g, int team, V pos) {
    return g->sight[team][cell(pos)] >= 50;
}
/* Stationary defenses are permanent shared observers until destroyed. Cache their
   layer independently of the moving roster and union both for main view + minimap.
   'clear' is a fully lit interior, not merely a tile marked targetable. */
static void illuminate(unsigned char *mask, V p, float clear, float outer) {
    float rr = outer * outer, inner = clear * clear;
    int x0 = (int)clampf((p.x - outer) / 40, 0, GW - 1), x1 = (int)clampf((p.x + outer) / 40, 0, GW - 1);
    int y0 = (int)clampf((p.y - outer) / 40, 0, GH - 1), y1 = (int)clampf((p.y + outer) / 40, 0, GH - 1);
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            int n = y * GW + x;
            if (mask[n] == 255)
                continue;
            float dx = x * 40 + 20 - p.x, dy = y * 40 + 20 - p.y, d2 = dx * dx + dy * dy;
            if (d2 >= rr)
                continue;
            int value = d2 <= inner ? 255 : (int)(255 * (rr - d2) / (rr - inner));
            if (value > mask[n])
                mask[n] = (unsigned char)value;
        }
}
static unsigned standing_defenses(const Game *g) {
    unsigned bits = 0;
    for (int i = 10; i < 24; i++)
        if (alive(&g->u[i]) && structure(&g->u[i]))
            bits |= 1u << (i - 10);
    return bits;
}
static void vision(Game *g) {
    unsigned bits = standing_defenses(g);
    if (bits != g->garrisonMask) {
        memset(g->garrisonSight, 0, sizeof g->garrisonSight);
        for (int i = 10; i < 24; i++)
            if (bits & (1u << (i - 10))) {
                Unit *u = &g->u[i];
                int core = u->kind == CORE;
                illuminate(g->garrisonSight[u->team], u->pos, core ? 545.f : 455.f, core ? 640.f : 555.f);
            }
        g->garrisonMask = bits;
    }
    memcpy(g->sight, g->garrisonSight, sizeof g->sight);
    for (int i = 0; i < MAXU; i++) {
        Unit *u = &g->u[i];
        if (!alive(u) || u->team == 2 || structure(u))
            continue;
        float r = u->kind == HERO ? 550.f : 318.f;
        illuminate(g->sight[u->team], u->pos, r * .77955f, r);
    }
    g->sightFrame++;
    g->sightTimer = .20f;
}
int walkable(const Game *g, V p) {
    return p.x >= 12 && p.x < WW - 12 && p.y >= 12 && p.y < WH - 12 && g->nav[cell(p)];
}
static V snap(const Game *g, V p) {
    if (walkable(g, p))
        return p;
    float best = 1e10f;
    V res = bases[0];
    for (int i = 0; i < GW * GH; i++)
        if (g->nav[i]) {
            V v = center(i), d = sub(v, p);
            float ds = dot(d, d);
            if (ds < best) {
                best = ds;
                res = v;
            }
        }
    return res;
}
static int clearline(const Game *g, V a, V b) {
    float d = dist(a, b);
    int n = (int)(d / 17) + 1;
    for (int i = 1; i <= n; i++)
        if (!walkable(g, add(a, mul(sub(b, a), (float)i / n))))
            return 0;
    return 1;
}
/* Breadth-first route on a bounded 8-connected grid, then line-of-sight smoothing. */
static void land_route(Game *g, Unit *u, V to) {
    int prev[GW * GH], q[GW * GH], rev[GW * GH];
    u->dest = snap(g, to);
    u->pathN = u->pathI = 0;
    if (clearline(g, u->pos, u->dest))
        return;
    int a = cell(u->pos), b = cell(u->dest), r = 0, w = 0;
    memset(prev, -1, sizeof prev);
    prev[a] = a;
    q[w++] = a;
    const int dx[] = {1, 0, -1, 0, 1, -1, 1, -1}, dy[] = {0, 1, 0, -1, 1, 1, -1, -1};
    while (r < w && prev[b] < 0) {
        int i = q[r++], x = i % GW, y = i / GW;
        for (int k = 0; k < 8; k++) {
            int xx = x + dx[k], yy = y + dy[k];
            if (xx < 0 || xx >= GW || yy < 0 || yy >= GH)
                continue;
            int j = yy * GW + xx;
            if (prev[j] >= 0 || !g->nav[j])
                continue;
            if (k >= 4 && (!g->nav[y * GW + xx] || !g->nav[yy * GW + x]))
                continue;
            prev[j] = i;
            q[w++] = j;
        }
    }
    if (prev[b] < 0) {
        u->dest = u->pos;
        return;
    }
    int n = 0;
    for (int i = b; i != a; i = prev[i])
        rev[n++] = i;
    V p = u->pos;
    for (int i = n - 1; i >= 0 && u->pathN < 98;) {
        int j = i;
        while (j > 0 && clearline(g, p, center(rev[j - 1])))
            j--;
        p = center(rev[j]);
        u->path[u->pathN++] = p;
        i = j - 1;
    }
    u->path[u->pathN++] = u->dest;
}
#define WADING_SPEED .65f
/* Foot position, not sprite bounds. The painted banks and all five crossing
   decks are excluded from the baked spans, including the two narrow paths. */
static int in_water(V p){
    if(p.x<0 || p.x>=WW || p.y<0 || p.y>=WH)return 0;
    int x=(int)(p.x*(RIVER_W/(float)WW)), y=(int)(p.y*(RIVER_H/(float)WH));
    const unsigned short *s=river_spans[y];
    return (x>=s[0] && x<s[1]) || (x>=s[2] && x<s[3]);
}
/* Compare routes in land-distance equivalent units. Sample the same foot
   mask as movement, so nearby bridges are useful but not mandatory. */
static float terrain_cost(V a,V b,float length){
    int n=1+(int)(length/16),wet=0;
    V step=mul(sub(b,a),1.f/n),p=add(a,mul(step,.5f));
    for(int j=0;j<n;j++,p=add(p,step))wet+=in_water(p);
    return length*(1+(1/WADING_SPEED-1)*wet/n);
}
static void route(Game *g,Unit *u,V to){
    land_route(g,u,to);
    if(u->kind!=HERO || g->training || u->pathN>=99)return;
    V a=u->pos,b=u->pathN?u->path[0]:u->dest;
    float length=dist(a,b);
    if(length<48)return;
    float best=terrain_cost(a,b,length);
    if(best<length+6)return;
    /* Centers of the five visible decks, in painting coordinates. */
    static const V decks[]={{302,177},{487,382},{731,532},{972,717},{1150,894}};
    V via={0};int found=0;
    for(unsigned j=0;j<sizeof decks/sizeof *decks;j++){
        V c=vec(decks[j].x*(WW/1464.f),decks[j].y*(WH/1074.f));
        float ac=dist(a,c),cb=dist(c,b);
        if(ac<14 || cb<14 || ac+cb+6>=best)continue;
        float cost=terrain_cost(a,c,ac)+terrain_cost(c,b,cb);
        if(cost+6<best && clearline(g,a,c) && clearline(g,c,b)){
            best=cost;via=c;found=1;
        }
    }
    if(found){
        memmove(u->path+1,u->path,u->pathN*sizeof *u->path);
        u->path[0]=via;u->pathN++;
    }
}
static void move(Game *g, Unit *u, V dir, float amount, int manualRun) {
    V old = u->pos;
    dir = norm(dir);
    V v = mul(dir, amount), p = add(u->pos, v);
    if (walkable(g, p))
        u->pos = p;
    else if (walkable(g, vec(p.x, u->pos.y)))
        u->pos.x = p.x;
    else if (walkable(g, vec(u->pos.x, p.y)))
        u->pos.y = p.y;
    float travelled = dist(old, u->pos);
    /* A held movement key still drives the legs against a wall. Keep actual
       travel for wall slides and AI; never feed visual stride into collision. */
    if (manualRun && travelled <= .01f)
        travelled = amount;
    if (travelled > .01f) {
        float cycle = u->kind == HERO ? 88.f : u->kind == SIEGE ? 102.f : u->kind == GOLEM ? 73.f : 54.f;
        u->stride = fmodf(u->stride + travelled / cycle, 1.f);
        u->moving = .065f;
        u->face = dir;
        if (fabsf(dir.x) > .12f)
            u->dir = dir.x < 0 ? -1 : 1;
    }
}
static float speed(const Game *g,const Unit *u) {
    float terrain=!g->training && u->kind==HERO && in_water(u->pos)?WADING_SPEED:1.f;
    return terrain * u->speed * (u->slow > 0 ? .52f : 1) * (u->haste > 0 ? 1.38f : 1);
}
static void travel(Game *g, Unit *u, V to, float dt) {
    if (u->repath <= 0 || dist(to, u->dest) > 90) {
        route(g, u, to);
        u->repath = .65f;
    }
    V p = u->pathI < u->pathN ? u->path[u->pathI] : u->dest;
    if (dist(p, u->pos) < 9 && u->pathI < u->pathN) {
        u->pathI++;
        p = u->pathI < u->pathN ? u->path[u->pathI] : u->dest;
    }
    move(g, u, sub(p, u->pos), fminf(dist(p, u->pos), speed(g,u) * dt), 0);
}
static Unit *make(Game *g, int id, int kind, int team, V p) {
    Unit *u = &g->u[id];
    memset(u, 0, sizeof *u);
    u->used = 1;
    u->serial = ++g->serial;
    u->kind = kind;
    u->team = team;
    u->pos = u->home = u->dest = p;
    u->target = -1;
    u->swingTarget = -1;
    u->stride = fmodf(u->serial * .381966f, 1.f);
    u->dir = team == 1 ? -1 : 1;
    u->face = vec(u->dir, 0);
    u->level = 1;
    u->lastCombat = -10;
    for (int i = 0; i < 10; i++)
        u->tag[i] = -100;
    return u;
}
int xp_next(const Unit *u) {
    return 100 + 45 * u->level;
}
int item_cost(const Unit *u, int i) {
    return 220 + 140 * u->items[i];
}
int game_buy(Game *g, int id, int i) {
    if (i < 0 || i > 2 || id < 0 || id >= 10)
        return 0;
    Unit *u = &g->u[id];
    if (u->items[i] >= 4 || u->gold < item_cost(u, i))
        return 0;
    u->gold -= item_cost(u, i);
    u->items[i]++;
    if (i == 0)
        u->power += 18;
    else if (i == 1) {
        u->maxhp += 210;
        if (alive(u))
            u->hp += 210;
    } else {
        u->speed += 12;
        u->period *= .93f;
    }
    if (!id)
        sound(g, S_BUY);
    return 1;
}
static void xp(Game *g, int id, int amt) {
    Unit *u = &g->u[id];
    if (u->level >= 15)
        return;
    u->xp += amt;
    while (u->level < 15 && u->xp >= xp_next(u)) {
        u->xp -= xp_next(u);
        u->level++;
        u->maxhp += 75;
        if (alive(u))
            u->hp += 75;
        u->power += 6;
        if (!id) {
            sound(g, S_LEVEL);
            game_notice(g, L"等级提升 · 最大生命与攻击提高");
        }
    }
}
static Fx *fx(Game *g, int k, V a, V b, float time, float sz, uint32_t c, int n) {
    Fx *f = &g->fx[g->fxCursor++ % MAXFX];
    *f = (Fx){.live = 1,
              .kind = k,
              .n = n,
              .a = a,
              .b = b,
              .life = time,
              .total = time,
              .size = sz,
              .color = c,
              .anchor = -1};
    return f;
}
static void arrival(Game *g, int id, float seconds, float scale) {
    Unit *u = &g->u[id];
    Fx *f = fx(g, FX_ARRIVAL, u->pos, u->pos, seconds, scale, 0, 0);
    f->anchor = id;
    f->anchorSerial = u->serial;
}
/* Resolve on draw, after all movement/dashes/separation. An old arrival may
   never be displayed on a corpse or rebound to another life using this slot. */
int game_fx_position(const Game *g, const Fx *f, V *out) {
    if (!f->live)
        return 0;
    if (f->kind != FX_ARRIVAL) {
        *out = f->a;
        return 1;
    }
    if (f->anchor < 0 || f->anchor >= HEROES)
        return 0;
    const Unit *u = &g->u[f->anchor];
    if (!alive(u) || u->serial != f->anchorSerial)
        return 0;
    *out = u->pos;
    return 1;
}
static void spawn_wave(Game *g) {
    g->waves++;
    for (int t = 0; t < 2; t++)
        for (int l = 0; l < 3; l++)
            for (int n = 0; n < (g->waves % 3 == 0 ? 4 : 3); n++) {
                int id = MOB0;
                while (id < MAXU && g->u[id].used)
                    id++;
                if (id == MAXU)
                    return;
                int k = n < 2 ? MELEE : n == 2 ? RANGED : SIEGE;
                V p = lane_at(l, t ? 1 - .035f : .035f);
                p.x += (n % 2 ? 1 : -1) * 20;
                p.y += (n / 2) * 23;
                Unit *u = make(g, id, k, t, p);
                u->lane = l;
                u->wp = t ? nway(l) - 2 : 1;
                float grow = fminf(3.f, g->time / 290);
                u->maxhp = u->hp = (k == SIEGE ? 610 : k == MELEE ? 265 : 200) * (1 + grow * .50f);
                u->power = (k == SIEGE ? 65 : k == MELEE ? 29 : 33) * (1 + grow * .65f);
                u->speed = 105 + (k != SIEGE) * 8;
                u->reach = k == MELEE ? 54 : k == SIEGE ? 215 : 161;
                u->period = k == SIEGE ? 1.65f : 1.18f;
                u->think = rnd(g) * .3f;
            }
}
static void init_world(Game *g, int role, int diff, uint32_t seed,int staged) {
    memset(g, 0, sizeof *g);
    /* Avalanche adjacent GetTickCount / replay seeds before the first draw. */
    uint32_t mix=seed+0x9e3779b9u;
    mix=(mix^(mix>>16))*0x85ebca6bu;
    mix=(mix^(mix>>13))*0xc2b2ae35u;
    mix^=mix>>16;
    g->rng = mix ? mix : 1;
    g->diff = diff;
    g->winner = -1;
    nav_init(g);
    for (int t = 0; t < 2; t++) {
        /* Independent draws, without replacement inside each team. Reserve the
           player's choice before shuffling; the VS screen reads these same units. */
        int pool[HERO_COUNT], lanes[5] = {1, 2, 1, 0, 0};
        for (int k = 0; k < HERO_COUNT; ++k) pool[k] = k;
        if (!t) {
            if (role < 0 || role >= HERO_COUNT) role = 0;
            pool[0] = role;
            pool[role] = 0;
        }
        int first = !t;
        for (int end = HERO_COUNT - 1; end > first; --end) {
            int n = first + (int)(rnd(g) * (end - first + 1));
            int a = pool[n]; pool[n] = pool[end]; pool[end] = a;
        }
        /* Assign the five lane slots by preference, without ever abandoning a lane. */
        for (int j = 0; j < 5; j++) {
            int r = pool[j], pick = j;
            int pref = (int[]){1,2,1,0,0,0,2,1,2,1}[r];
            for (int k = j; k < 5; ++k) if (lanes[k] == pref) {pick = k; break;}
            const HeroDef *h = &heroes[r];
            V p = add(bases[t], vec((j % 3 - 1) * 49, (j / 3) * 42 - 25));
            Unit *u = make(g, t * 5 + j, HERO, t, p);
            u->home = bases[t];
            u->role = r;
            u->lane = lanes[pick]; lanes[pick] = lanes[j];
            u->wp = t ? nway(u->lane) - 2 : 1;
            u->maxhp = u->hp = h->health;
            u->power = h->attack;
            u->speed = h->speed;
            u->period = h->period;
            u->reach = h->reach;
            u->gold = 160;
            u->think = rnd(g) * .3f;
            if(staged) u->used=0;
            else arrival(g, t * 5 + j, j == 0 ? 1.45f : 1.10f, j == 0 ? 1.f : .67f);
        }
        for (int l = 0; l < 3; l++)
            for (int k = 0; k < 2; k++) {
                float s = k ? .365f : .180f;
                Unit *u = make(g, 10 + t * 7 + l * 2 + k, TOWER, t, lane_at(l, t ? 1 - s : s));
                u->tier = k;
                u->lane = l;
                u->maxhp = u->hp = k ? 1450 : 1750;
                u->reach = 266;
                u->power = 109;
                u->period = .93f;
            }
        Unit *u = make(g, 16 + t * 7, CORE, t, bases[t]);
        u->maxhp = u->hp = 3000;
        u->reach = 274;
        u->power = 125;
        u->period = .92f;
    }
    for (int i = 0; i < 4; i++) {
        Unit *u = make(g, 24 + i, GOLEM, 2, camps[i]);
        u->maxhp = u->hp = 780;
        u->power = 46;
        u->reach = 85;
        u->speed = 82;
        u->period = 1.35f;
    }
    for (int i = 5; i < 10; i++) {
        float f = (float[]){.82f, 1.f, 1.17f}[diff];
        g->u[i].power *= f;
        g->u[i].maxhp *= f;
        g->u[i].hp = g->u[i].maxhp;
    }
    g->wave = 7; /* Two seconds for the greeting, then a real five-second warning. */
    g->awaitingSpawn = staged;
    if (!staged)
        sound(g, VO_WELCOME);
    g->pay = 1;
    g->aim = lane_at(1, .5f);
    vision(g);
}
void game_init(Game *g, int role, int diff, uint32_t seed) {
    init_world(g, role, diff, seed, 0);
}
void game_prepare_match(Game *g, int role, int diff, uint32_t seed) {
    init_world(g, role, diff, seed, 1);
}
void game_deploy(Game *g) {
    if (!g->awaitingSpawn)
        return;
    g->awaitingSpawn = 0;
    /* Five distinct feet/beam origins on the fountain apron; do not obscure
       either the crystal or the player behind a stack of overlapping sprites. */
    static const V marks[5]={{0,99},{-115,40},{-59,80},{59,80},{115,40}};
    for (int i = 0; i < HEROES; i++) {
        g->u[i].pos=add(g->u[i].home,mul(marks[i%5],i<5?1.f:-1.f));
        g->u[i].used = 1;
        arrival(g, i, 1.45f, i % 5 == 0 ? 1.f : .78f);
    }
    /* Give control immediately; let the last arrival beam finish before
       the five-second warning. The first wave really departs five seconds later. */
    g->wave = 6.55f;
    vision(g);
}
void game_training(Game *g, int role) {
    init_world(g, role, 1, 0x725b, 1);
    g->awaitingSpawn = 0;
    g->training = 1;
    for (int i = 0; i < MAXU; i++)
        g->u[i].used = i == 0 || i == 5;
    memset(g->nav, 0, sizeof g->nav);
    /* The courtyard occupies the first 32 by 18 world cells. Stone walls
       bound one shared collision grid for WASD, click routes and dashes. */
    for (int y = 3; y < 16; y++)
        for (int x = 2; x < 30; x++)
            if (y < 14 || (x >= 4 && x < 28))
                g->nav[y * GW + x] = 1;
    Unit *p = &g->u[0], *d = &g->u[5];
    p->home = p->pos = vec(276, 338);
    p->dir = p->poseDir = 1;
    p->face = vec(1, 0);
    d->home = d->pos = vec(765, 320);
    d->role = 3;
    d->dir = d->poseDir = -1;
    d->face = vec(-1, 0);
    d->maxhp = d->hp = 2000;
    d->grace = 0;
    memset(g->sight, 255, sizeof g->sight);
    g->sightFrame++;
    arrival(g, 0, 1.45f, 1.f);
}
static int valid(const Game *g, int id, int serial) {
    return id >= 0 && id < MAXU && alive(&g->u[id]) && g->u[id].serial == serial;
}
static void lock(Game *g, Unit *u, int id) {
    u->target = id;
    u->targetSerial = id < 0 ? 0 : g->u[id].serial;
}
/* Wallet credit and its presentation share one authoritative reward event.
   Passive income never spawns coins; nearby shared gold has a smaller burst. */
static void bounty(Game *g, int who, V from, int amount) {
    g->u[who].gold += amount;
    Fx *f = fx(g, FX_COINS, from, g->u[who].pos, 1.04f, amount < 10 ? .66f : 1.f, GOLD, amount);
    f->team = who; /* hero slot; only tracked while alive and visible */
    if (!who && g->time >= g->coinSoundAt) {
        sound(g, S_COIN);
        g->coinSoundAt = g->time + .14f;
    }
}
static void kill_card(Game *g, Unit *k, Unit *victim) {
    int first = !g->firstBlood && k->kind == HERO;
    if (first)
        g->firstBlood = 1;
    if (g->cardCount == 10) { /* Never evict the card currently on screen. */
        memmove(g->cards + 1, g->cards + 2, 8 * sizeof *g->cards);
        g->cardCount--;
    }
    if (!g->cardCount) {
        g->cardLife = 2.45f;
        g->cardStart = g->time;
    }
    int voice = VO_EXECUTED;
    if (k->kind == HERO) {
        k->multi = k->multi && g->time-k->lastKillAt <= 10.f ? k->multi+1 : 1;
        k->lastKillAt = g->time;
        k->spree++;
        if (first) voice = VO_FIRST;
        else if (k->multi >= 2) voice = (k->team ? VO_ENEMY_DOUBLE : VO_DOUBLE) + (k->multi > 5 ? 3 : k->multi-2);
        else if (victim->spree >= 3) voice = VO_SHUTDOWN;
        else if (k->spree >= 3) voice = (k->team ? VO_ENEMY_SPREE : VO_SPREE) + (k->spree > 7 ? 4 : k->spree-3);
        else if (k == &g->u[0]) voice = VO_PLAYERKILL;
        else if (victim == &g->u[0]) voice = VO_PLAYERDEATH;
        else voice = victim->team ? VO_ALLYKILL : VO_ENEMYKILL;
    }
    victim->multi = victim->spree = 0;
    g->cards[g->cardCount++] = (KillCard){k->kind, k->role, k->team, victim->role, victim->team, first, voice};
    sound(g, voice);
    int down = 0;
    for (int i=5*victim->team; i<5*victim->team+5; ++i)
        down += g->u[i].used && !alive(&g->u[i]);
    if (down == 5) {
        sound(g, VO_ACED);
        feed(g, victim->team ? L"敌方团灭" : L"我方团灭", victim->team ? JADE : RED);
    }
}
static void die(Game *g, int id, int killer) {
    Unit *u = &g->u[id], *k = &g->u[killer];
    u->hp = 0;
    u->recall = u->shield = u->buff = 0;
    u->order = 0;
    lock(g, u, -1);
    uint32_t c = teamColor[k->team];
    wchar_t msg[100];
    fx(g, FX_BURST, u->pos, u->face, .65f, structure(u) ? 82 : 38, c, 0);
    if (u->kind == CORE) {
        g->winner = 1 - u->team;
        sound(g, g->winner == 0 ? S_WIN : S_LOSE);
        sound(g, g->winner == 0 ? VO_VICTORY : VO_DEFEAT);
        return;
    }
    if (u->kind == HERO) {
        if (k->kind != HERO) {
            float newest = g->time - 5;
            for (int h = 0; h < 10; h++)
                if (g->u[h].team != u->team && u->tag[h] > newest) {
                    newest = u->tag[h];
                    killer = h;
                    k = &g->u[h];
                }
        }
        c = teamColor[k->team];
        u->deaths++;
        if (k->team < 2)
            g->kills[k->team]++;
        if (k->kind == HERO) {
            k->kills++;
            bounty(g, killer, u->pos, 180);
            xp(g, killer, 140);
        }
        for (int h = 0; h < 10; h++)
            if (h != killer && g->u[h].team != u->team && g->time - u->tag[h] < 8) {
                g->u[h].assists++;
                if (h == 0)
                    bounty(g, h, u->pos, 65);
                else
                    g->u[h].gold += 65;
                xp(g, h, 75);
            }
        kill_card(g, k, u);
        u->respawn = 7 + u->level * .85f;
        u->stun = u->haste = u->empower = u->slow = 0;
        if (!id || !killer)
            sound(g, S_KILL);
        return;
    }
    if (u->kind == TOWER) {
        g->towers[1 - u->team]++;
        swprintf(msg, 100, L"%ls路%ls塔已破", ((const wchar_t *[]){L"上", L"中", L"下"})[u->lane],
                 u->team ? L"敌方" : L"我方");
        feed(g, msg, c);
        sound(g, u->team ? VO_TOWERUP : VO_TOWERDOWN);
        for (int h = 0; h < 10; h++)
            if (g->u[h].team != u->team) {
                if (h == killer || h == 0)
                    bounty(g, h, u->pos, 80);
                else
                    g->u[h].gold += 80;
                xp(g, h, 70);
            }
        return;
    }
    if (u->kind == GOLEM) {
        u->respawn = 43;
        if (k->kind == HERO) {
            k->buff = 35;
            bounty(g, killer, u->pos, 115);
            k->hp = fminf(k->maxhp, k->hp + 160);
            xp(g, killer, 130);
            if (!killer)
                game_notice(g, L"获得山灵祝福 · 35 秒内攻击提高 25%");
        }
        return;
    }
    if (k->kind == HERO) {
        bounty(g, killer, u->pos, u->kind == SIEGE ? 54 : 30);
        k->cs++;
    }
    for (int h = 0; h < 10; h++)
        if (alive(&g->u[h]) && g->u[h].team != u->team && dist(u->pos, g->u[h].pos) < 475) {
            xp(g, h, 28);
            if (h != killer) {
                if (h == 0)
                    bounty(g, h, u->pos, 9);
                else
                    g->u[h].gold += 9;
            }
        }
    u->used = 0;
}
static void damage(Game *g, int src, int id, float amt, float slow, float stun, int skill) {
    if (g->winner >= 0 || !alive(&g->u[id]))
        return;
    Unit *u = &g->u[id], *k = &g->u[src];
    if (u->grace > 0 || protected_building(g, id))
        return;
    if (structure(u)) {
        int escort = 0;
        for (int i = MOB0; i < MAXU; i++)
            if (alive(&g->u[i]) && g->u[i].team != u->team && dist(u->pos, g->u[i].pos) < 330) {
                escort = 1;
                break;
            }
        amt *= escort ? 1 : .24f;
        if (skill)
            amt *= .32f;
        if (k->kind == SIEGE)
            amt *= 1.55f;
        slow = stun = 0;
    }
    if (k->kind == HERO) {
        u->tag[src] = g->time;
        if (u->kind == HERO) {
            for (int i = 10; i < 24; i++) {
                Unit *t = &g->u[i];
                if (alive(t) && t->team == u->team && dist(k->pos, t->pos) < t->reach) {
                    lock(g, t, src);
                    t->aggro = 2.6f;
                }
            }
        }
    }
    if (u->kind == GOLEM) {
        if (dist(u->pos, u->home) > 260)
            return;
        lock(g, u, src);
        u->aggro = 5;
    }
    if (u->shield > 0) {
        float absorb = fminf(amt, u->shield);
        u->shield -= absorb;
        amt -= absorb;
    }
    u->hp -= amt;
    u->slow = fmaxf(u->slow, slow);
    u->stun = fmaxf(u->stun, stun);
    u->lastCombat = g->time;
    if (u->recall > 0) {
        u->recall = 0;
        if (!id)
            game_notice(g, L"受到攻击，回城已中断");
    }
    if ((id == 0 || src == 0) && amt >= 1) {
        fx(g, FX_NUMBER, add(u->pos, vec((rnd(g) - .5f) * 20, -36)), vec(0, 0), .72f, skill ? 22 : 17,
           id == 0 ? RED
           : skill ? GOLD
                   : 0xf5f3df,
           (int)(amt + .5f));
    }
    if(g->training && id==5){
        g->practiceHits++;g->lastDamage=amt;g->practiceDamage+=amt;
        if(u->hp<=0)u->hp=u->maxhp;
        return;
    }
    if (u->hp <= 0)
        die(g, id, src);
}
static void area(Game *g, int src, V p, float radius, float dmg, float slow, float stun, int execute) {
    Unit *k = &g->u[src];
    for (int i = 0; i < MAXU; i++) {
        Unit *u = &g->u[i];
        if (!alive(u) || !hostile(k, u) || dist(p, u->pos) > radius + (structure(u) ? 20 : 9))
            continue;
        float bonus = execute ? (1 - u->hp / u->maxhp) * 1.3f : 0;
        damage(g, src, i, dmg * (1 + bonus), slow, stun, 1);
    }
}
static Shot *shot(Game *g, int id, int kind, V p, V v, float time, float dmg, uint32_t color) {
    Shot *s = &g->shot[g->shotCursor++ % MAXP];
    memset(s, 0, sizeof *s);
    s->live = 1;
    s->owner = id;
    s->serial = g->u[id].serial;
    s->team = g->u[id].team;
    s->kind = kind;
    s->p = s->start = p;
    s->v = v;
    s->life = time;
    s->damage = dmg;
    s->color = color;
    s->target = -1;
    return s;
}
static void pose(Unit *u, int type, float duration) {
    u->pose = type;
    u->poseDir = u->dir;
    u->anim = u->actionTime = duration;
}
static void release_basic(Game *g, int id, int target) {
    Unit *u = &g->u[id], *b = &g->u[target];
    float dmg = u->power * (u->buff > 0 ? 1.25f : 1);
    uint32_t c = u->kind == HERO ? heroes[u->role].color : teamColor[u->team];
    if (u->reach >= 140) {
        V muzzle = add(u->pos, mul(norm(sub(b->pos, u->pos)), structure(u) ? 0 : u->kind == HERO ? 23 : 15));
        Shot *s = shot(g, id, 0, muzzle, vec(0, 0), 2, dmg, c);
        s->target = target;
        s->tserial = b->serial;
        s->radius = structure(u) ? 6 : u->kind == HERO ? 4 : 3;
    } else {
        fx(g, FX_SLASH, u->pos, b->pos, .21f, u->kind == HERO ? 58 : 30, c, 0);
        damage(g, id, target, dmg, 0, 0, 0);
    }
    if (!id)
        sound(g, S_ATTACK);
    else if (g->soundN == 0 && dist(u->pos, g->u[0].pos) < 460 && ((u->serial + (int)(g->time * 6)) % 6 == 0))
        sound(g, S_AMBIENT);
}
static void basic(Game *g, int id, int target) {
    Unit *u = &g->u[id];
    if (u->atk > 0 || u->stun > 0 || (u->anim > 0 && u->pose >= POSE_SKILL))
        return;
    if (u->kind == GOLEM) {
        /* Plant both feet. The marker drags toward the target, then locks;
           the fist strikes that piece of ground even if the target sidesteps. */
        V toward = sub(g->u[target].pos, u->pos);
        u->stoneAim = add(u->pos, mul(u->face, fminf(u->reach, lengthv(toward))));
        pose(u, POSE_ATTACK, STONE_CYCLE);
        u->atk = u->period;
        u->swingTarget = target;
        u->swingSerial = g->u[target].serial;
        u->releaseIn = STONE_CONTACT;
        return;
    }
    u->face = norm(sub(g->u[target].pos, u->pos));
    if (fabsf(u->face.x) > .1f)
        u->dir = u->face.x < 0 ? -1 : 1;
    u->atk = u->period * (u->haste > 0 ? .58f : 1);
    if (structure(u)) {
        release_basic(g, id, target);
        return;
    }
    pose(u, POSE_ATTACK, fminf(u->atk * .80f, u->kind == SIEGE ? .64f : u->kind == GOLEM ? .60f : .47f));
    u->swingTarget = target;
    u->swingSerial = g->u[target].serial;
    u->releaseIn = u->actionTime * ((u->kind == HERO && u->role == 1) ? .66f : .50f);
}
static V bounded(V a, V b, float range) {
    V v = sub(b, a);
    return add(a, mul(norm(v), fminf(range, lengthv(v))));
}
static V dash_end(const Game *g, V from, V d, float range) {
    for (int i = 0; i < (int)(range / 6); i++) {
        V p = add(from, mul(d, 6));
        if (!walkable(g, p))
            break;
        from = p;
    }
    return from;
}
static void dash(Game *g, Unit *u, V d, float range) {
    u->pos = dash_end(g, u->pos, d, range);
    u->repath = 0;
}
static void ultimate_fx(Game *g, int type, Unit *u, V a, V b, float duration, float radius) {
    fx(g, type, a, b, duration, radius, heroes[u->role].color, u->role)->team = u->team;
}
/* Melee ultimates resolve on their authored contact frame. Ranged zones have
   their own projectile timer and survive the caster's subsequent movement. */
static void release_ultimate(Game *g, int id) {
    Unit *u = &g->u[id];
    V p = u->pos, d = u->ultVector;
    float a = u->power * (u->buff > 0 ? 1.25f : 1);
    if (u->role == 0) {
        Shot *s = shot(g, id, 2, p, mul(d, 710), .62f, a * 3.15f, heroes[0].color);
        s->radius = 73;
        s->stun = .82f;
        s->style = SHOT_FIREBLADE;
        ultimate_fx(g, FX_ULT_IMPACT, u, p, add(p, mul(d, 90)), .43f, 80);
    } else {
        int tank = u->role == 3;
        u->pos = u->ultAim;
        u->repath = 0;
        area(g, id, u->pos, tank ? 162 : 128, a * (tank ? 2.2f : 2.65f), 0, tank ? 1.45f : 0, !tank);
        ultimate_fx(g, FX_ULT_IMPACT, u, u->pos, u->ultStart, tank ? .91f : .64f, tank ? 162 : 128);
    }
}
/* Each seal is a real delayed hit; draw both its tell and its impact at the hit
   location so a moving target can dodge even the later links of a sequence. */
static void seal(Game *g, int id, V center, V origin, float delay, float radius,
                 float damageAmount, float slow, float stun) {
    Unit *u = &g->u[id];
    Shot *s = shot(g,id,3,center,vec(0,0),delay,damageAmount,heroes[u->role].color);
    s->radius=radius; s->slow=slow; s->stun=stun; s->style=SHOT_SEAL;
    ultimate_fx(g,FX_ULT_CAST,u,center,origin,delay,radius);
}
int hero_seals(int r,V p,V aim,V facing,V out[5],float *radius){
    if(r<5 || r>=HERO_COUNT) return 0;
    V d=norm(sub(aim,p));
    if(lengthv(d)<.5f) d=facing;
    V q=bounded(p,aim,470),side=vec(-d.y,d.x);
    int n=(int[]){1,3,3,1,5}[r-5];
    *radius=(float[]){127,110,94,151,76}[r-5];
    for(int j=0;j<n;j++){
        if(r==6) out[j]=add(q,mul(d,(j-1)*68.f));
        else if(r==7) out[j]=add(q,add(mul(d,j==0?-42.f:25.f),mul(side,j==0?0:j==1?-49.f:49.f)));
        else if(r==9) out[j]=add(add(p,mul(sub(q,p),(j+1)*.2f)),mul(side,sinf((j+1)*1.55f)*22));
        else out[j]=q;
    }
    return n;
}
static void cast_second_roster(Game *g, int id, int slot, V aim, V d, float power) {
    Unit *u=&g->u[id]; int r=u->role; V p=u->pos, q=bounded(p,aim,470), side=vec(-d.y,d.x);
    uint32_t c=heroes[r].color;
    if (slot==0) {
        int fan=r==8?3:1;
        for (int i=0;i<fan;i++) {
            float angle=fan==3?(i-1)*.19f:0;
            V v=add(mul(d,cosf(angle)),mul(side,sinf(angle)));
            float speed=r==7?485:r==6?565:750;
            Shot *s=shot(g,id,r==7||r==8?1:2,p,mul(v,speed),480/speed,
                         power*(float[]){1.87f,1.40f,1.86f,.94f,1.72f}[r-5],c);
            s->radius=r==6?32:r==7?22:15;
            s->splash=r==7?91:0;
            s->slow=r==6?1.5f:r==8?.65f:0;
        }
    } else if (slot==1) {
        switch(r) {
        case 5:
            u->shield=fmaxf(u->shield,85+power*1.35f); u->haste=2.2f;
            fx(g,FX_HEAL,p,p,.55f,52,c,0);
            { V origin=p; int previous=-1;
              for(int hop=0;hop<2;hop++){
                int next=-1; float near=hop?135:ARC_REACH;
                for(int j=0;j<MAXU;j++){
                    Unit *v=&g->u[j];
                    if(j==previous || !alive(v) || !hostile(u,v) || v->grace>0 || protected_building(g,j)) continue;
                    float distance=dist(origin,v->pos);
                    if(distance<=near){near=distance;next=j;}
                }
                if(next<0) break;
                V end=g->u[next].pos;
                fx(g,FX_LINE,origin,end,.28f,10-hop*3,c,0);
                fx(g,FX_BURST,end,end,.25f,21,c,0);
                damage(g,id,next,power*(hop?.64f:1.03f),0,0,1);
                origin=end; previous=next;
              }
            }
            break;
        case 6:
            fx(g,FX_RING,p,p,.62f,245,c,0);
            for(int j=0;j<HEROES;j++) {
                Unit *ally=&g->u[j];
                if(!alive(ally)||ally->team!=u->team||dist(ally->pos,p)>245) continue;
                ally->hp=fminf(ally->maxhp,ally->hp+60+power*1.35f);
                fx(g,FX_HEAL,ally->pos,p,.78f,48,c,0);
            }
            { V echo=bounded(p,aim,TIDE_ECHO_RANGE);
              Shot *s=shot(g,id,3,echo,vec(0,0),.31f,power*1.16f,c);
              s->radius=TIDE_ECHO_RADIUS;
              fx(g,FX_LINE,p,echo,.29f,7,c,0);fx(g,FX_ZONE,echo,echo,.31f,TIDE_ECHO_RADIUS,c,3);
            }
            break;
        case 7:
            u->shield=fmaxf(u->shield,70+power);
            area(g,id,p,138,power*1.10f,1.3f,0,0); fx(g,FX_RING,p,p,.48f,138,c,0); break;
        case 8:
            area(g,id,p,115,power*.78f,2,0,0); dash(g,u,d,156);
            fx(g,FX_RING,p,p,.55f,115,c,0); fx(g,FX_LINE,p,u->pos,.46f,12,c,0); break;
        case 9: {
            Shot *s=shot(g,id,3,q,vec(0,0),.48f,power*1.12f,c);
            s->radius=99;s->stun=1.42f;fx(g,FX_ZONE,q,q,.48f,99,c,0); break;
        }}
    } else {
        /* First hit, interval, power ratio, slow and stun. */
        static const float timing[][5]={{.53f,0,3.1f,0,1.05f},{.38f,.20f,1.16f,1.35f,0},
            {.44f,.19f,1.37f,0,0},{.62f,0,2.65f,.85f,1.45f},{.355f,.105f,1.02f,1.05f,0}};
        const float *s=timing[r-5];V centers[5];float radius;
        int n=hero_seals(r,p,aim,d,centers,&radius);
        for(int j=0;j<n;j++) seal(g,id,centers[j],p,s[0]+s[1]*j,radius,power*s[2],s[3],s[4]);
    }
}
int game_cast(Game *g, int id, int slot, V aim) {
    if (slot < 0 || slot > 2 || id < 0 || id >= 10 || g->winner >= 0)
        return 0;
    Unit *u = &g->u[id];
    if (!alive(u) || u->stun > 0 || u->cd[slot] > .001f || (u->pose == POSE_ULT && u->anim > 0))
        return 0;
    int r = u->role;
    const HeroDef *h = &heroes[r];
    V d = norm(sub(aim, u->pos));
    if (lengthv(d) < .5f)
        d = u->face;
    V p = u->pos;
    u->face = d;
    if (fabsf(d.x) > .1f)
        u->dir = d.x < 0 ? -1 : 1;
    u->cd[slot] = g->training ? 0 : h->cd[slot];
    u->recall = 0;
    u->swingTarget = -1;
    pose(u, slot == 2 ? POSE_ULT : POSE_SKILL, slot == 2 ? (float[]){.57f,.69f,.91f,.71f,.45f,.68f,.80f,.78f,.66f,.82f}[r] : .31f);
    float a = u->power * (u->buff > 0 ? 1.25f : 1);
    uint32_t c = h->color;
    Shot *s;
    V q;
    if (!id) {
        g->castCount[slot]++;
        sound(g, slot == 2 ? S_ULT : S_SKILL);
    }
    if (r>=5) cast_second_roster(g,id,slot,aim,d,a);
    else if ((r == 0 || r == 4) && slot == 0) {
        dash(g, u, d, r == 0 ? 156 : 198);
        q = u->pos;
        fx(g, FX_LINE, p, q, .34f, 24, c, 0);
        fx(g, FX_SLASH, q, add(q, mul(d, 75)), .33f, 72, c, 0);
        for (int j = 0; j < MAXU; j++)
            if (alive(&g->u[j]) && hostile(u, &g->u[j]) && segdist(g->u[j].pos, p, add(q, mul(d, 47))) < 59)
                damage(g, id, j, 1.68f * a, 0, 0, 1);
    } else if (r == 0 && slot == 1) {
        fx(g, FX_RING, p, p, .45f, 160, c, 0);
        for (int j = 0; j < MAXU; j++)
            if (alive(&g->u[j]) && hostile(u, &g->u[j]) && dist(p, g->u[j].pos) < 161) {
                damage(g, id, j, a * 1.7f, 0, 0, 1);
                u->hp = fminf(u->maxhp, u->hp + 33);
            }
    } else if (slot == 2 && (r == 0 || r == 3 || r == 4)) {
        u->ultVector = d;
        u->ultStart = p;
        u->ultAim = r == 0 ? add(p, mul(d, 90)) : dash_end(g, p, d, fminf(dist(p, aim), r == 3 ? 232 : 295));
        u->ultWindup = u->actionTime * .50f;
        ultimate_fx(g, FX_ULT_CAST, u, p, u->ultAim, u->ultWindup, r == 3 ? 162 : r == 4 ? 128 : 73);
    } else if ((r == 1 && slot == 0) || (r == 2 && slot == 0) || (r == 4 && slot == 1)) {
        s = shot(g, id, r == 1 ? 2 : 1, p, mul(d, r == 2 ? 510 : 760), r == 2 ? .94f : .72f,
                 a * (r == 1 ? 1.8f : 2.05f), c);
        s->radius = r == 2 ? 24 : 17;
        s->splash = r == 2 ? 100 : 0;
        s->slow = r == 4 ? 2 : 0;
    } else if (r == 1 && slot == 1) {
        /* Fire before translating: a close target must not end up behind the
           muzzle when the roll carries the archer past it. */
        s=shot(g,id,1,p,mul(d,790),WIND_ARROW_RANGE/790.f,1.24f*a,c);
        s->radius=15;
        dash(g, u, d, 130);
        u->haste = 3.2f;
        fx(g, FX_LINE, p, u->pos, .42f, 13, c, 0);
    } else if ((r == 1 && slot == 2) || (r == 2 && slot > 0)) {
        q = bounded(p, aim, 475);
        float rad = slot == 1 ? 112 : r == 1 ? 153 : 170;
        float delay = slot == 1 ? .38f : r == 1 ? .48f : .8f;
        s = shot(g, id, 3, q, vec(0, 0), delay, a * (slot == 1 ? 1.12f : r == 1 ? 2.9f : 3.9f), c);
        s->radius = rad;
        s->stun = slot == 1 ? 1.25f : 0;
        s->slow = slot == 2 ? 1.7f : 0;
        if (slot == 2) {
            s->style = r == 1 ? SHOT_ARROW_RAIN : SHOT_METEOR;
            ultimate_fx(g, FX_ULT_CAST, u, q, p, delay, rad);
        } else
            fx(g, FX_ZONE, q, q, delay, rad, c, 3);
    } else if (r == 3 && slot == 0) {
        area(g, id, p, 143, a * 1.65f, 1.8f, 0, 0);
        fx(g, FX_RING, p, p, .45f, 143, c, 0);
    } else if (r == 3 && slot == 1) {
        area(g,id,p,AEGIS_RADIUS,a*1.03f,0,0,0);
        fx(g,FX_RING,p,p,.42f,AEGIS_RADIUS,c,0);
        for (int j = 0; j < 10; j++)
            if (alive(&g->u[j]) && g->u[j].team == u->team && dist(p, g->u[j].pos) < 242) {
                g->u[j].shield = fmaxf(g->u[j].shield, 130 + a * 1.5f);
                fx(g, FX_HEAL, g->u[j].pos, p, .7f, 52, c, 0);
            }
    }
    return 1;
}
void game_order(Game *g, V dest, int target, int attackMove) {
    Unit *u = &g->u[0];
    if (!alive(u) || g->winner >= 0)
        return;
    u->recall = 0;
    if (target >= 0 && target < MAXU && alive(&g->u[target]) && hostile(u, &g->u[target])) {
        lock(g, u, target);
        u->order = 2;
        dest = g->u[target].pos;
        if (protected_building(g, target))
            game_notice(g, L"封印保护：先拆除同路前置防御塔");
    } else {
        target = -1;
        lock(g, u, -1);
        u->order = attackMove ? 3 : 1;
    }
    route(g, u, dest);
    u->repath = .8f;
    g->mark = target >= 0 ? dest : u->dest;
    g->markTime = .8f;
}
static void begin_recall(Game *g, int id) {
    Unit *u = &g->u[id];
    if (!alive(u) || u->recall > 0 || u->stun > 0 || u->ultWindup > 0 || g->winner >= 0)
        return;
    u->recall = 4;
    u->swingTarget = -1;
    u->order = 0;
    lock(g, u, -1);
    if (!id)
        game_notice(g, L"回城中 · 移动、施法或受击将中断");
}
void game_recall(Game *g) {
    begin_recall(g, 0);
}
void game_rally(Game *g, V pos) {
    if (!alive(&g->u[0]) || g->winner >= 0)
        return;
    if (g->rallyCd > 0) {
        game_notice(g, L"集结信号正在冷却");
        return;
    }
    g->rally = snap(g, pos);
    g->rallyIds[0] = g->rallyIds[1] = -1;
    for (int n = 0; n < 2; n++) {
        float best = 1e9f;
        for (int i = 1; i < 5; i++) {
            Unit *u = &g->u[i];
            if (!alive(u) || u->retreat || (n && g->rallyIds[0] == i))
                continue;
            float d = dist(u->pos, g->rally);
            if (d < best) {
                best = d;
                g->rallyIds[n] = i;
            }
        }
    }
    if (g->rallyIds[0] < 0) {
        game_notice(g, L"暂时没有可响应的队友");
        return;
    }
    g->rallyTime = 12;
    g->rallyCd = 22;
    sound(g, VO_RALLY);
    feed(g, L"请求集合  ·  向信标靠拢", JADE);
}
static int pick(Game *g, int id, float radius, int neutral) {
    Unit *u = &g->u[id];
    int tactical=id<HEROES && u->kind==HERO && !g->training && (id!=0 || g->autoplay);
    int best = -1;
    float score = 1e7f;
    for (int j = 0; j < MAXU; j++) {
        Unit *v = &g->u[j];
        if (!alive(v) || !hostile(u, v) || v->grace > 0 || protected_building(g, j) ||
            (!neutral && v->team == 2))
            continue;
        float d = dist(u->pos, v->pos);
        if (d > radius)
            continue;
        if(tactical && !structure(v) && !visible(g,u->team,v->pos)) continue;
        float s = d;
        if (structure(u)) {
            if (v->kind == HERO)
                s += 800;
            if (v->kind == SIEGE)
                s -= 50;
        } else if (u->kind == HERO) {
            if (v->kind == HERO)
                s -= 65;
            if (v->kind == CORE)
                s -= 75;
            s += v->hp / v->maxhp * 35;
            if(tactical){
                if(j==u->target && v->serial==u->targetSerial) s-=32;
                if(v->kind==HERO){
                    s-=78*(1-v->hp/v->maxhp);
                    if(v->hp+v->shield<u->power*2.2f) s-=94;
                    for(int k=u->team*5;k<u->team*5+5;k++)
                        if(alive(&g->u[k]) && g->u[k].hp<g->u[k].maxhp*.55f &&
                           g->time-g->u[k].tag[j]<2.3f && dist(u->pos,g->u[k].pos)<330){
                            s-=90;break;
                        }
                } else if((v->kind==MELEE||v->kind==RANGED||v->kind==SIEGE) &&
                          v->hp+v->shield<=u->power && d<=u->reach+18)
                    s-=102;
            }
        }
        if (s < score) {
            best = j;
            score = s;
        }
    }
    return best;
}
static int nearest_wp(Unit *u) {
    int best = 0;
    float score = 1e9f;
    for (int i = 0; i < nway(u->lane) - 1; i++) {
        V a = waypoint(u->lane, i), b = waypoint(u->lane, i + 1);
        float d = segdist(u->pos, a, b);
        if (d < score) {
            score = d;
            best = i;
        }
    }
    return best + (u->team == 0);
}
static void march(Game *g, Unit *u, float dt) {
    int end = u->team ? 0 : nway(u->lane) - 1;
    V p = waypoint(u->lane, u->wp);
    float offset = ((u->serial * 17) % 57) - 28.f;
    if (u->wp != end) {
        p.x += offset;
        p.y += offset * .65f;
    }
    if (dist(u->pos, p) < 49 && u->wp != end) {
        u->wp += u->team ? -1 : 1;
        p = waypoint(u->lane, u->wp);
        u->repath = 0;
    }
    travel(g, u, p, dt);
}
static void player(Game *g, Unit *u, Input in, float dt) {
    int steering = fabsf(in.x) + fabsf(in.y) > .05f;
    if (steering) {
        /* Direction overrides the path, not a deliberately selected enemy or
           an already committed swing. Never auto-chase against a held key. */
        if (u->order != 2) {
            u->order = 0;
            lock(g, u, -1);
        }
        u->recall = 0;
        move(g, u, vec(in.x, in.y), speed(g,u) * dt, 1);
    }
    if (u->recall > 0)
        return;
    if (u->order == 2 && valid(g, u->target, u->targetSerial) && !structure(&g->u[u->target]) &&
        !visible(g, 0, g->u[u->target].pos)) {
        u->order = steering ? 0 : 1;
        lock(g, u, -1);
    }
    if (u->order == 2 && !valid(g, u->target, u->targetSerial)) {
        u->order = 0;
        lock(g, u, -1);
    }
    if (u->order == 2) {
        Unit *v = &g->u[u->target];
        if (dist(u->pos, v->pos) > u->reach + 14) {
            if (!steering)
                travel(g, u, v->pos, dt);
            return;
        }
        basic(g, 0, u->target);
        return;
    }
    if ((!g->training || in.attack || u->order==3) && (u->order != 1 || in.attack) && (u->atk <= 0 || u->order == 3)) {
        int a = pick(g, 0, u->reach + 14, in.attack);
        if (a >= 0) {
            basic(g, 0, a);
        }
    }
    if (in.attack && u->order == 0 && fabsf(in.x) + fabsf(in.y) < .05f) {
        int a = pick(g, 0, u->reach + 135, 1);
        if (a >= 0 && dist(u->pos, g->u[a].pos) > u->reach + 14)
            travel(g, u, g->u[a].pos, dt);
    }
    if (u->order == 1 || u->order == 3) {
        V dest = u->dest;
        if (dist(u->pos, dest) < 10)
            u->order = 0;
        else
            travel(g, u, dest, dt);
    }
}
static void shop_ai(Game *g, int id) {
    Unit *u = &g->u[id];
    int item = (u->items[0] + u->items[1] + u->items[2]) % 5;
    int plan[5] = {0, 1, 0, 2, 1};
    int j = plan[item];
    if (u->role == 3 && item == 0)
        j = 1;
    if (u->items[j] >= 4)
        for (j = 0; j < 3 && u->items[j] >= 4; j++)
            ;
    if (j < 3)
        game_buy(g, id, j);
}
/* These routines run only for bot heroes. Perception is sampled on the think
   clock, not on every render/simulation frame. Never use the player's input
   vector to predict a shot or the hidden aim of an unannounced enemy cast. */
static int bot_difficulty(const Game *g,const Unit *u){return u->team?g->diff:1;}
static float tower_risk(const Game *g,const Unit *u,V p) {
    float risk=0;
    for(int i=10+(1-u->team)*7;i<17+(1-u->team)*7;i++){
        const Unit *t=&g->u[i];
        if(alive(t)) risk+=fmaxf(0,t->reach+25-dist(p,t->pos));
    }
    return risk;
}
static void bot_observe(Game *g,int id) {
    Unit *u=&g->u[id]; BotMind *m=&g->bot[id];
    int diff=bot_difficulty(g,u);
    if(valid(g,u->target,u->targetSerial)){
        Unit *v=&g->u[u->target];
        if(v->serial!=m->observedSerial){
            m->observedSerial=v->serial;m->velocity=vec(0,0);
            m->readyAt=g->time+.27f-.07f*diff+.12f*rnd(g);
        } else {
            float elapsed=g->time-m->sampleTime;
            if(elapsed>.03f){
                V speedSample=mul(sub(v->pos,m->sample),1/elapsed);
                /* Discard flashes/teleports rather than extrapolating them. */
                m->velocity=lengthv(speedSample)<310 ? mul(add(m->velocity,speedSample),.5f):vec(0,0);
            }
        }
        m->sample=v->pos; m->sampleTime=g->time;
    } else {m->observedSerial=0; m->velocity=vec(0,0);}
    m->friends=1;m->rivals=0;m->buddy=-1;float best=-10000,need=0;int care=0;
    if(u->role==6 && u->maxhp-u->hp>85){
        need=(1-u->hp/u->maxhp); care=id+1;
    }
    for(int j=0;j<HEROES;j++){
        Unit *v=&g->u[j];if(j==id||!alive(v))continue;
        float d=dist(u->pos,v->pos);if(d>440)continue;
        if(v->team!=u->team){
            if(visible(g,u->team,v->pos))m->rivals++;
        }else{
            m->friends++;
            /* Prefer a nearby ally who can stand between us and a pursuer. */
            float score=95*v->hp/v->maxhp+(v->reach<150?30:0)-d*.28f;
            if(!v->retreat && score>best){m->buddy=j;best=score;}
            float urgency=1-v->hp/v->maxhp - fmaxf(0,d-245)*.0007f;
            if(u->role==6 && v->maxhp-v->hp>85 && d<395 && urgency>need){
                need=urgency;care=j+1;
            }
        }
    }
    if(care!=m->careSlot){
        m->careSlot=care;
        if(care)m->careAt=g->time+.20f-.05f*diff;
    }
    if(g->time>=m->strafeAt){
        m->strafeSide=rnd(g)<.5f?-1:1;
        m->strafeAt=g->time+.72f+rnd(g)*.88f;
    }
    if(g->time<m->nextDodge)return;
    V hazards[12];float radii[12];int count=0;
    for(int i=0;i<MAXP&&count<12;i++){
        Shot *s=&g->shot[i];
        if(!s->live||s->kind!=3||s->team==u->team||s->life<.08f||s->life>1.35f||
           dist(u->pos,s->p)>s->radius+8||!visible(g,u->team,s->p)) continue;
        hazards[count]=s->p;radii[count++]=s->radius;
    }
    for(int i=24;i<28&&count<12;i++){
        Unit *v=&g->u[i];
        if(alive(v)&&v->swingTarget>=0 && v->releaseIn>.09f && v->releaseIn<STONE_CONTACT &&
           visible(g,u->team,v->pos) && dist(u->pos,v->stoneAim)<STONE_RADIUS+8){
            hazards[count]=v->stoneAim;radii[count++]=STONE_RADIUS;
        }
    }
    if(!count)return;
    m->nextDodge=g->time+1.20f+rnd(g)*.55f;
    if(rnd(g)>(.48f+diff*.20f))return; /* a tell is not a guaranteed dodge */
    float top=-1e8f;V dest=u->pos;
    for(int j=0;j<8;j++){
        float ang=(j+.21f*id)*.7853982f;
        V q=dash_end(g,u->pos,vec(cosf(ang),sinf(ang)),120);
        float clearance=1e5f;
        for(int h=0;h<count;h++) clearance=fminf(clearance,dist(q,hazards[h])-radii[h]);
        float score=clearance-tower_risk(g,u,q)*2-.08f*dist(q,u->home);
        if(dist(q,u->pos)>25 && score>top){top=score;dest=q;}
    }
    if(dist(dest,u->pos)>25){
        m->escape=dest;m->evadeAt=g->time+.19f-.045f*diff+rnd(g)*.09f;
        m->evadeUntil=m->evadeAt+.62f;
    }
}
static int bot_spell(Game *g,int id,int slot,V aim){
    BotMind *m=&g->bot[id];Unit *u=&g->u[id];
    if(g->time<m->skillAt || (u->swingTarget>=0&&u->releaseIn>0) || !game_cast(g,id,slot,aim))return 0;
    m->skillAt=g->time+.40f+.21f*rnd(g)+.07f*(2-bot_difficulty(g,u));
    return 1;
}
static void bot_combo(Game *g,int id,int target,float distance){
    Unit *u=&g->u[id],*v=&g->u[target];BotMind *m=&g->bot[id];
    int r=u->role,duel=v->kind==HERO,diff=bot_difficulty(g,u);
    if(g->time<m->readyAt||g->time<m->skillAt||structure(v))return;
    /* Winning the last hit with a basic is better than wasting a spell. */
    if(!duel && v->kind!=GOLEM && v->hp<=u->power && distance<u->reach+14) return;
    float lead=fminf(.41f,distance/680.f)*(.42f+.19f*diff);
    V aim=add(v->pos,mul(m->velocity,lead));
    V back=add(u->pos,mul(norm(sub(u->pos,v->pos)),150));
    if(duel && (r==1||r==8) && v->reach<150 && distance<125 && bot_spell(g,id,1,back))return;
    if(duel && (r==3||r==5||r==7) && distance<160 && u->shield<36 && bot_spell(g,id,1,v->pos))return;
    if(duel && (r==2||r==9) && distance<462 && v->stun<.1f && bot_spell(g,id,1,aim))return;
    if(duel && distance<(r==3?228:r==4?285:423) &&
       (v->stun>.18f || v->hp<v->maxhp*.64f || m->rivals>1 || u->cd[0]>.1f) &&
       bot_spell(g,id,2,v->stun>.18f?v->pos:aim))return;
    float qRange=r==3?141:r==0?175:r==4?208:465;
    if(distance<qRange && bot_spell(g,id,0,aim))return;
    float eRange=(float[]){160,450,460,158,500,180,475,138,115,460}[r];
    if(distance>=eRange || (!duel && r!=0 && r!=4 && r!=9))return;
    if(r==1 && (distance<200 || tower_risk(g,u,add(u->pos,mul(norm(sub(aim,u->pos)),130)))>0))return;
    if(r==6 && (u->hp<u->maxhp*.7f || m->careSlot))return; /* save a needed heal */
    bot_spell(g,id,1,r==8?back:aim);
}
static void brain(Game *g, int id, float dt) {
    Unit *u = &g->u[id];
    int hero = u->kind == HERO;
    if (u->kind == GOLEM) {
        /* A camp guards its clearing. Close heroes can draw it without first
           landing a hit, but it never searches globally or pursues past leash. */
        if ((u->aggro<=0 || !valid(g,u->target,u->targetSerial)) && dist(u->pos,u->home)<108) {
            int near = -1;
            float best = 112;
            for (int h = 0; h < HEROES; h++)
                if (alive(&g->u[h]) && g->u[h].grace <= 0) {
                    float d = dist(g->u[h].pos, u->pos);
                    if (d < best && dist(g->u[h].pos, u->home) < 178) {
                        best = d;
                        near = h;
                    }
                }
            if (near >= 0) {
                lock(g, u, near);
                u->aggro = 5;
            }
        }
        if (u->aggro <= 0 || dist(u->pos, u->home) > 250 || !valid(g, u->target, u->targetSerial)) {
            lock(g, u, -1);
            if (dist(u->pos, u->home) > 5) {
                travel(g, u, u->home, dt);
                u->hp = fminf(u->maxhp, u->hp + u->maxhp * dt * .22f);
            }
            return;
        }
        Unit *v = &g->u[u->target];
        if (dist(v->pos, u->home) > 290) {
            u->aggro = 0;
            return;
        }
        if (dist(u->pos, v->pos) > u->reach)
            travel(g, u, v->pos, dt);
        else
            basic(g, id, u->target);
        return;
    }
    if(hero && valid(g,u->target,u->targetSerial) && !structure(&g->u[u->target]) &&
       !visible(g,u->team,g->u[u->target].pos)){
        lock(g,u,-1);g->bot[id].observedSerial=0;
    }
    if (u->think <= 0) {
        u->think = .23f + rnd(g) * .18f;
        if (hero) {
            if (u->hp / u->maxhp < .245f)
                u->retreat = 1;
            if (u->retreat && u->hp / u->maxhp > .88f) {
                u->retreat = 0;
                u->wp = nearest_wp(u);
            }
            /* Rotate toward an exposed enemy nexus; defend the home crystal first. */
            if (g->time > 28 && !u->retreat) {
                int lane = u->lane;
                Unit *core = &g->u[16 + u->team * 7];
                if (core->hp < core->maxhp * .94f && g->time - core->lastCombat < 4 &&
                    dist(u->pos, core->pos) > 350) {
                    for (int l = 0; l < 3; l++)
                        if (!alive(&g->u[10 + u->team * 7 + 2 * l]))
                            lane = l;
                } else if ((u->role == 4 || u->role == 2) &&
                           protected_building(g, 16 + (1 - u->team) * 7) == 0) {
                    for (int l = 0; l < 3; l++)
                        if (!alive(&g->u[10 + (1 - u->team) * 7 + l * 2]))
                            lane = l;
                }
                if (lane != u->lane) {
                    u->lane = lane;
                    u->wp = nearest_wp(u);
                    u->repath = 0;
                }
            }
        }
        if (!(structure(u) && u->aggro > 0 && valid(g, u->target, u->targetSerial) &&
              dist(u->pos, g->u[u->target].pos) < u->reach + 8)) {
            int a = pick(g, id, structure(u) ? u->reach : hero ? (u->reach>=200?435:355) : 178, 0);
            if (a != u->target && a >= 0)
                u->repath = 0;
            lock(g, u, a);
        }
        if(hero) bot_observe(g,id);
    }
    if(hero && g->time>=g->bot[id].evadeAt && g->time<g->bot[id].evadeUntil &&
       dist(u->pos,g->bot[id].escape)>7){
        move(g,u,sub(g->bot[id].escape,u->pos),speed(g,u)*dt,0);
        return;
    }
    /* Heal wounded allies even when not tracking an enemy. Healthy groups
       keep the heal available for the next engagement. */
    if(hero && u->role==6 && u->cd[1]<=0 && g->bot[id].careSlot &&
       g->time>=g->bot[id].careAt) {
        Unit *v=&g->u[g->bot[id].careSlot-1];
        if(alive(v) && v->maxhp-v->hp>65 && dist(u->pos,v->pos)<=245){
            V aim=valid(g,u->target,u->targetSerial)?g->u[u->target].pos:v->pos;
            bot_spell(g,id,1,aim);
        }
    }
    if (hero && u->retreat) {
        /* Retreat on foot under fire; once safe, bots can use exactly the same
           interruptible fountain return and arrival as the human player. */
        /* A four-second channel is not worth it if walking home is as fast.
           Leave a second's margin for starting the channel and nearby danger. */
        if (dist(u->pos, u->home) > u->speed * 5.f && g->time - u->lastCombat > 4.f && u->stun <= 0) {
            int safe = 1;
            for (int j = 0; j < MAXU && safe; j++) {
                Unit *v = &g->u[j];
                if (alive(v) && hostile(u, v) && (v->kind != GOLEM || v->target == id) &&
                    dist(u->pos, v->pos) < 350)
                    safe = 0;
            }
            if (safe) {
                begin_recall(g, id);
                return;
            }
        }
        if (dist(u->pos, u->home) > 100)
            travel(g, u, u->home, dt);
        if ((u->role == 3 || u->role == 5 || u->role == 7) && u->shield < 60 && u->hp < .4f * u->maxhp)
            bot_spell(g, id, 1, u->home);
        return;
    }
    if (hero && u->role == 4 && g->time > 1.5f && g->time < 100 && u->mode == 0) {
        int camp = u->team ? 27 : 24;
        Unit *c = &g->u[camp];
        if (!alive(c) || u->hp < u->maxhp * .4f)
            u->mode = 1;
        else if (!valid(g, u->target, u->targetSerial) || g->u[u->target].kind != HERO) {
            if (dist(c->pos, u->pos) > 180) {
                travel(g, u, c->pos, dt);
                return;
            }
            lock(g, u, camp);
        }
    }
    int rally = hero && u->team == 0 && g->rallyTime > 0 && (g->rallyIds[0] == id || g->rallyIds[1] == id);
    if (rally && dist(u->pos, g->rally) > 165 &&
        (!valid(g, u->target, u->targetSerial) || dist(u->pos, g->u[u->target].pos) > 230)) {
        travel(g, u, g->rally, dt);
        return;
    }
    if(hero && u->role==6 && u->cd[1]<1 && !rally && g->bot[id].careSlot &&
       g->time>=g->bot[id].careAt && g->bot[id].rivals<=g->bot[id].friends){
        Unit *v=&g->u[g->bot[id].careSlot-1];
        if(alive(v) && v->hp<v->maxhp*.66f && dist(u->pos,v->pos)>236 &&
           tower_risk(g,u,v->pos)<=tower_risk(g,u,u->pos)){
            travel(g,u,v->pos,dt);return; /* close the gap for a wounded ally, not a suicide dive */
        }
    }
    if (hero && valid(g, u->target, u->targetSerial)) {
        Unit *v = &g->u[u->target];
        for (int j = 10 + (1 - u->team) * 7; j < 17 + (1 - u->team) * 7; j++) {
            Unit *t = &g->u[j];
            if (!alive(t) || t->hp < t->maxhp * .14f || dist(v->pos, t->pos) > t->reach + 35 ||
                dist(u->pos, t->pos) > t->reach + 185)
                continue;
            int wave = 0;
            for (int k = MOB0; k < MAXU; k++)
                if (alive(&g->u[k]) && g->u[k].team == u->team && dist(g->u[k].pos, t->pos) < t->reach + 14) {
                    wave = 1;
                    break;
                }
            if (!wave && dist(u->pos, t->pos) < t->reach + 85) {
                if (dist(u->pos, t->pos) < t->reach + 55) {
                    V back = add(t->pos, mul(norm(sub(u->pos, t->pos)), t->reach + 78));
                    travel(g, u, back, dt);
                }
                return;
            }
        }
    }
    if (valid(g, u->target, u->targetSerial)) {
        int a = u->target;
        Unit *v = &g->u[a];
        float d = dist(u->pos, v->pos);
        if(hero && g->time<g->bot[id].readyAt)return;
        if (hero && ((v->kind == HERO) || (!structure(v) && d < 265))) {
            bot_combo(g,id,a,d);
            if (!alive(v))
                return;
            d = dist(u->pos, v->pos);
        }
        if(hero && v->kind==HERO && g->bot[id].rivals>g->bot[id].friends &&
           u->hp<u->maxhp*.62f && d>u->reach*.70f && v->hp>u->power*2){
            int buddy=g->bot[id].buddy;
            V safe=(buddy>=0 && alive(&g->u[buddy]))?g->u[buddy].pos:u->home;
            if(dist(u->pos,safe)>105 && dist(safe,v->pos)>d+28){travel(g,u,safe,dt);return;}
        }
        if (d <= u->reach + (structure(u) ? 4 : 14)) {
            basic(g, id, a);
            if(hero && u->reach>200 && v->kind==HERO && u->swingTarget<0 && u->atk>.16f){
                V away=norm(sub(u->pos,v->pos));
                float side=(float)g->bot[id].strafeSide;
                V drift=add(mul(away,d<u->reach*.78f?1.f:-.13f),mul(vec(-away.y,away.x),side*.73f));
                V look=add(u->pos,mul(norm(drift),42));
                if(walkable(g,look)&&tower_risk(g,u,look)<=tower_risk(g,u,u->pos)+1)
                    move(g,u,drift,speed(g,u)*dt*.87f,0);
            }
        } else if (!structure(u))
            travel(g, u, v->pos, dt);
        return;
    }
    if (rally) {
        if (dist(u->pos, g->rally) > 32)
            travel(g, u, g->rally, dt);
        return;
    }
    if (!structure(u))
        march(g, u, dt);
}
static void shots(Game *g, float dt) {
    for (int i = 0; i < MAXP; i++) {
        Shot *s = &g->shot[i];
        if (!s->live)
            continue;
        s->life -= dt;
        Unit *k = &g->u[s->owner];
        if (k->serial != s->serial) {
            s->live = 0;
            continue;
        }
        if (s->kind == 3) {
            if (s->life <= 0) {
                area(g, s->owner, s->p, s->radius, s->damage, s->slow, s->stun, 0);
                if (s->style == SHOT_ARROW_RAIN || s->style == SHOT_METEOR || s->style == SHOT_SEAL)
                    ultimate_fx(g, FX_ULT_IMPACT, k, s->p, s->p, s->style == SHOT_METEOR ? .85f : .59f,
                                s->radius);
                else
                    fx(g, FX_BURST, s->p, s->p, .55f, s->radius, s->color, 0);
                s->live = 0;
            }
            continue;
        }
        if (s->life <= 0) {
            s->live = 0;
            continue;
        }
        if (s->kind == 0) {
            if (!valid(g, s->target, s->tserial)) {
                s->live = 0;
                continue;
            }
            V dest = g->u[s->target].pos;
            V dv = sub(dest, s->p);
            s->v = mul(norm(dv), 610);
            if (lengthv(dv) < 610 * dt + 10) {
                damage(g, s->owner, s->target, s->damage, 0, 0, 0);
                fx(g, FX_BURST, dest, dest, .18f, 15, s->color, 0);
                s->live = 0;
                continue;
            }
            s->p = add(s->p, mul(s->v, dt));
        } else {
            V p = s->p;
            s->p = add(p, mul(s->v, dt));
            for (int j = 0; j < MAXU; j++) {
                Unit *v = &g->u[j];
                unsigned mask = 1u << (j % 32);
                if (!alive(v) || v->team == s->team || (s->hits[j / 32] & mask) ||
                    segdist(v->pos, p, s->p) > s->radius + (structure(v) ? 28 : 16))
                    continue;
                s->hits[j / 32] |= mask;
                if (s->splash) {
                    area(g, s->owner, v->pos, s->splash, s->damage, s->slow, s->stun, 0);
                    fx(g, FX_RING, v->pos, v->pos, .38f, s->splash, s->color, 0);
                } else
                    damage(g, s->owner, j, s->damage, s->slow, s->stun, 1);
                if (s->kind == 1) {
                    s->live = 0;
                    break;
                }
            }
        }
    }
}
static void stone_action(Game *g, int id, float dt) {
    Unit *u = &g->u[id];
    if (u->swingTarget < 0)
        return;
    /* No ghost contact if the victim died/reused its slot, or if the camp reset. */
    if (!valid(g, u->swingTarget, u->swingSerial) || u->aggro <= 0) {
        u->swingTarget = -1;
        u->anim = 0;
        return;
    }
    float track = clampf(u->releaseIn - (STONE_CONTACT - STONE_TRACK), 0, dt);
    if (track > 0) {
        V aim = bounded(u->pos, g->u[u->swingTarget].pos, u->reach);
        u->stoneAim = add(u->stoneAim, mul(sub(aim, u->stoneAim), 1 - expf(-8.f * track)));
        V d = norm(sub(u->stoneAim, u->pos));
        if (lengthv(d) > .1f)
            u->face = d;
        if (fabsf(d.x) > .1f)
            u->dir = u->poseDir = d.x < 0 ? -1 : 1;
    }
    if ((u->releaseIn -= dt) > 0)
        return;
    u->swingTarget = -1;
    fx(g, FX_EARTH, u->stoneAim, u->pos, .44f, STONE_RADIUS, GOLD, 0);
    if (dist(g->u[0].pos, u->pos) < 410)
        sound(g, S_STONE);
    for (int j = 0; j < MAXU; ++j)
        if (alive(&g->u[j]) && hostile(u, &g->u[j]) && !structure(&g->u[j]) &&
            dist(g->u[j].pos, u->stoneAim) <= STONE_RADIUS)
            damage(g, id, j, u->power, 0, 0, 0);
}
static void respawn(Game *g, int id) {
    Unit *u = &g->u[id];
    if(id<HEROES)memset(&g->bot[id],0,sizeof g->bot[id]);
    u->pos = add(u->home, vec(id < 10 ? (id % 5 - 2) * 19 : 0, 0));
    u->hp = u->maxhp;
    u->serial = ++g->serial;
    u->grace = u->kind == HERO ? 1.7f : 0;
    u->retreat = u->order = u->pathN = u->pathI = 0;
    u->swingTarget = -1;
    u->ultWindup = u->anim = u->moving = 0;
    u->pose = POSE_READY;
    u->repath = 0;
    u->wp = u->team == 1 ? nway(u->lane) - 2 : 1;
    for (int i = 0; i < 3; i++)
        u->cd[i] = 0;
    for (int i = 0; i < 10; i++)
        u->tag[i] = -100;
    lock(g, u, -1);
    if (u->kind == HERO)
        arrival(g,id,1.45f,1.f);
    else
        fx(g, FX_HEAL, u->pos, u->pos, .8f, 62, JADE, 0);
    if (!id)
        game_notice(g, L"已重返战场");
}
void game_step(Game *g, float dt, Input in) {
    if (g->awaitingSpawn || g->winner >= 0 || dt <= 0)
        return;
    dt = fminf(dt, .1f);
    g->time += dt;
    float previousWave = g->wave;
    if (!g->training)
        g->wave -= dt;
    if (!g->waves && previousWave > 5 && g->wave <= 5)
        sound(g, VO_COUNTDOWN);
    if (!g->training)
        g->pay -= dt;
    g->noticeTime -= dt;
    if (g->cardCount && (g->cardLife -= dt) <= 0) {
        memmove(g->cards, g->cards + 1, (--g->cardCount) * sizeof *g->cards);
        g->cardStart = g->time;
        g->cardLife = g->cardCount > 1 ? 1.55f : 2.45f;
    }
    g->markTime -= dt;
    g->rallyCd -= dt;
    if (g->rallyTime > 0 && (g->rallyTime -= dt) <= 0) {
        for (int n = 0; n < 2; n++) {
            int id = g->rallyIds[n];
            if (id > 0)
                g->u[id].wp = nearest_wp(&g->u[id]);
        }
    }
    g->sightTimer-=dt;
    if (!g->training && g->wave <= 0) {
        spawn_wave(g);
        if (g->waves == 1) {
            sound(g, VO_MARCH);
        }
        g->wave += g->time > 360 ? 14 : 18;
        if (g->waves == 21)
            feed(g, L"决战时刻 · 兵线加速集结", GOLD);
    }
    if (!g->training && g->pay <= 0) {
        g->pay += 1;
        for (int i = 0; i < 10; i++) {
            g->u[i].gold += 5;
            if (i || g->autoplay)
                shop_ai(g, i);
        }
    }
    for (int i = 0; i < MAXFX; i++)
        if (g->fx[i].live && (g->fx[i].life -= dt) <= 0)
            g->fx[i].live = 0;
    for (int i = 0; i < MAXU && g->winner < 0; i++) {
        Unit *u = &g->u[i];
        if (!u->used)
            continue;
        if (!alive(u)) {
            if ((u->kind == HERO || u->kind == GOLEM) && (u->respawn -= dt) <= 0)
                respawn(g, i);
            continue;
        }
        u->think -= dt;
        u->atk -= dt;
        u->repath -= dt;
        u->anim -= dt;
        u->moving -= dt;
        u->slow -= dt;
        u->stun -= dt;
        u->haste -= dt;
        u->aggro -= dt;
        u->grace -= dt;
        u->buff -= dt;
        if(g->training && i==5)continue; /* Never run targeting, regen or movement on the practice dummy. */
        if (u->kind == HERO) {
            for (int j = 0; j < 3; j++)
                u->cd[j] = fmaxf(0, u->cd[j] - dt);
            u->shield = fmaxf(0, u->shield - dt * 6);
            float regen = dist(u->home, u->pos) < 188 ? .135f : g->time - u->lastCombat > 6 ? .009f : .0015f;
            u->hp = fminf(u->maxhp, u->hp + regen * u->maxhp * dt);
            if (u->recall > 0) {
                if (i == 0 && !g->autoplay && (in.x || in.y || in.attack)) {
                    u->recall = 0;
                } else if ((u->recall -= dt) <= 0) {
                    u->pos = u->home;
                    u->order = u->pathN = 0;
                    u->repath = 0;
                    arrival(g, i, 1.45f, 1.f);
                    if (!i)
                        game_notice(g, L"已抵达泉水");
                } else
                    continue;
            }
        }
        if (u->stun > 0) {
            if (u->kind == GOLEM)
                u->anim = 0;
            u->swingTarget = -1;
            if (u->ultWindup > 0) {
                u->ultWindup = 0;
                u->anim = 0;
            }
            continue;
        }
        if (u->ultWindup > 0) {
            u->ultWindup -= dt;
            if (u->role == 3) {
                float t = clampf((1 - u->ultWindup / (u->actionTime * .5f) - .20f) / .80f, 0, 1);
                float ease = t*t*(3-2*t);
                if (!i && !g->autoplay) {
                    /* Advance from the current (possibly steered) position, not
                       the original takeoff point. Still land on the marked aim. */
                    float prev = clampf(t-dt/(u->actionTime*.4f),0,1);
                    prev = prev*prev*(3-2*prev);
                    float fraction = clampf((ease-prev)/fmaxf(.00001f,1-prev),0,1);
                    u->pos = add(u->pos,mul(sub(u->ultAim,u->pos),fraction));
                } else u->pos = add(u->ultStart, mul(sub(u->ultAim, u->ultStart), ease));
            }
            if (!i && !g->autoplay && (in.x || in.y)) {
                /* Keep the skill's locked direction/landing point; the player
                   may still steer their feet during the startup animation. */
                move(g,u,vec(in.x,in.y),speed(g,u)*dt,1);
            }
            if (u->ultWindup <= 0)
                release_ultimate(g, i);
            continue;
        }
        if (u->kind == GOLEM && u->pose == POSE_ATTACK && u->anim > 0) {
            stone_action(g, i, dt);
            continue;
        }
        if (u->swingTarget >= 0 && (u->releaseIn -= dt) <= 0) {
            int target = u->swingTarget;
            u->swingTarget = -1;
            if (valid(g, target, u->swingSerial) && dist(u->pos, g->u[target].pos) <= u->reach + 48)
                release_basic(g, i, target);
            if (g->winner >= 0)
                break;
        }
        if (i == 0 && !g->autoplay)
            player(g, u, in, dt);
        else
            brain(g, i, dt);
    }
    shots(g, dt);
    /* Soft formation separation: keep individual soldiers readable without blocking allies. */
    for (int i = 0; i < MAXU; i++) {
        Unit *a = &g->u[i];
        if (!alive(a) || structure(a))
            continue;
        for (int j = i + 1; j < MAXU; j++) {
            Unit *b = &g->u[j];
            if (!alive(b) || structure(b) || a->team != b->team)
                continue;
            float dx = a->pos.x - b->pos.x, dy = a->pos.y - b->pos.y;
            if (fabsf(dx) > 29 || fabsf(dy) > 29)
                continue;
            float d = sqrtf(dx * dx + dy * dy);
            float gap = (i < 10 || j < 10) ? 30 : 23;
            if (d >= gap)
                continue;
            V force = d > .01f ? vec(dx / d, dy / d) : vec(1, 0);
            force = mul(force, (gap - d) * fminf(dt * 4, .35f));
            V aa = add(a->pos, force), bb = sub(b->pos, force);
            if (i != 0 && walkable(g, aa))
                a->pos = aa;
            if (walkable(g, bb))
                b->pos = bb;
        }
    }
    /* Don't leave a phantom observer for one tick after its tower is destroyed. */
    if (!g->training && (g->sightTimer <= 0 || standing_defenses(g) != g->garrisonMask))
        vision(g);
}
