#ifndef RIFT_GAME_H
#define RIFT_GAME_H
#include <stdint.h>
#include <wchar.h>
#define WW 3000
#define WH 2200
#define MAXU 208
#define MAXP 256
#define MAXFX 320
#define HEROES 10
#define MOB0 28
#define GW 75
#define GH 55
typedef struct {
    float x, y;
} V;
typedef enum { HERO, MELEE, RANGED, SIEGE, TOWER, CORE, GOLEM } Kind;
enum { FX_RING, FX_LINE, FX_SLASH, FX_BURST, FX_NUMBER, FX_MARK, FX_HEAL, FX_ZONE };
enum { S_ATTACK, S_HIT, S_SKILL, S_ULT, S_KILL, S_BUY, S_LEVEL, S_ALERT, S_UI, S_WIN, S_LOSE };
typedef struct {
    const wchar_t *name, *role, *tag, *bio, *skill[3], *desc[3];
    uint32_t color;
    float health, attack, reach, speed, period, cd[3];
} HeroDef;
extern const HeroDef heroes[5];
extern const wchar_t *itemNames[3], *itemDesc[3];
typedef struct {
    int used, serial, kind, team, role, lane, tier, dir, target, targetSerial, mode, wp, kills, deaths,
        assists, cs, level, xp, gold, items[3], order, retreat;
    V pos, home, dest, face, path[100];
    int pathN, pathI;
    float hp, maxhp, shield, power, reach, speed, period, atk, think, cd[3], slow, stun, haste, empower, buff,
        respawn, anim, moving, recall, repath, grace, aggro, tag[HEROES], lastCombat;
} Unit;
typedef struct {
    int live, owner, serial, target, tserial, team, kind, pass;
    V p, v, start;
    float life, damage, radius, splash, slow, stun;
    uint32_t color, hits[(MAXU + 31) / 32];
} Shot;
typedef struct {
    int live, kind, n;
    V a, b;
    float life, total, size;
    uint32_t color;
} Fx;
typedef struct {
    wchar_t msg[100];
    float time;
    uint32_t color;
} Feed;
typedef struct {
    Unit u[MAXU];
    Shot shot[MAXP];
    Fx fx[MAXFX];
    Feed feed[4];
    uint32_t rng;
    int serial, winner, diff, autoplay, kills[2], towers[2], waves, castCount[3], sound[16], soundN;
    float time, wave, pay, markTime, noticeTime;
    V aim, mark;
    wchar_t notice[100];
    unsigned char nav[GW * GH], sight[2][GW * GH];
    int fxCursor, shotCursor, sightFrame, rallyIds[2];
    float sightTimer, rallyTime, rallyCd;
    V rally;
} Game;
typedef struct {
    float x, y;
    int attack;
} Input;
extern const uint32_t teamColor[3];
V vec(float x, float y);
V add(V a, V b);
V sub(V a, V b);
V mul(V a, float f);
float lengthv(V a);
float dist(V a, V b);
V norm(V a);
float clampf(float x, float lo, float hi);
V lane_at(int lane, float progress);
void game_init(Game *g, int role, int diff, uint32_t seed);
void game_step(Game *g, float dt, Input input);
int game_cast(Game *g, int id, int slot, V aim);
void game_order(Game *g, V dest, int target, int attackMove);
void game_recall(Game *g);
int game_buy(Game *g, int id, int item);
int item_cost(const Unit *u, int item);
int xp_next(const Unit *u);
int protected_building(const Game *g, int id);
int walkable(const Game *g, V p);
int visible(const Game *g, int team, V pos);
void game_rally(Game *g, V pos);
int alive(const Unit *u);
int hostile(const Unit *a, const Unit *b);
void game_notice(Game *g, const wchar_t *msg);
#endif
