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
enum {
    FX_RING,
    FX_LINE,
    FX_SLASH,
    FX_BURST,
    FX_NUMBER,
    FX_MARK,
    FX_HEAL,
    FX_ZONE,
    FX_ULT_CAST,
    FX_ULT_IMPACT,
    FX_ARRIVAL,
    FX_COINS,
    FX_EARTH
};
enum { POSE_READY, POSE_ATTACK, POSE_SKILL, POSE_ULT };
enum { SHOT_PLAIN, SHOT_FIREBLADE, SHOT_ARROW_RAIN, SHOT_METEOR };
enum {
    S_ATTACK,
    S_HIT,
    S_SKILL,
    S_ULT,
    S_KILL,
    S_BUY,
    S_LEVEL,
    S_ALERT,
    S_UI,
    S_WIN,
    S_LOSE,
    S_AMBIENT,
    S_STONE,
    S_COIN,
    S_PICK0, S_PICK1, S_PICK2, S_PICK3, S_PICK4, S_VS, S_FRACTURE,
    VO_COUNTDOWN,
    VO_MARCH,
    VO_FIRST,
    VO_RALLY,
    VO_ALLYKILL,
    VO_ENEMYKILL,
    VO_TOWERUP,
    VO_TOWERDOWN,
    VO_VICTORY,
    VO_DEFEAT,
    VO_WELCOME,
    VO_PLAYERKILL,
    VO_PLAYERDEATH,
    VO_DOUBLE,
    VO_TRIPLE,
    VO_QUADRA,
    VO_PENTA,
    VO_ENEMY_DOUBLE,
    VO_ENEMY_TRIPLE,
    VO_ENEMY_QUADRA,
    VO_ENEMY_PENTA,
    VO_SPREE,
    VO_RAMPAGE,
    VO_UNSTOPPABLE,
    VO_GODLIKE,
    VO_LEGENDARY,
    VO_ENEMY_SPREE,
    VO_ENEMY_RAMPAGE,
    VO_ENEMY_UNSTOPPABLE,
    VO_ENEMY_GODLIKE,
    VO_ENEMY_LEGENDARY,
    VO_SHUTDOWN,
    VO_ACED,
    VO_EXECUTED,
    SOUND_COUNT
};
#define STONE_RADIUS 39.f
#define STONE_TRACK .32f
#define STONE_CONTACT .74f
#define STONE_CYCLE 1.07f
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
    /* Gait is driven by travel distance. The action clock and signed facing are
       latched at startup so a moving actor cannot reverse a swing mid-frame. */
    float stride, actionTime, releaseIn, ultWindup;
    int pose, poseDir, swingTarget, swingSerial;
    int spree, multi;
    float lastKillAt;
    V ultStart, ultAim, ultVector, stoneAim;
    float hp, maxhp, shield, power, reach, speed, period, atk, think, cd[3], slow, stun, haste, empower, buff,
        respawn, anim, moving, recall, repath, grace, aggro, tag[HEROES], lastCombat;
} Unit;
typedef struct {
    int live, owner, serial, target, tserial, team, kind, style;
    V p, v, start;
    float life, damage, radius, splash, slow, stun;
    uint32_t color, hits[(MAXU + 31) / 32];
} Shot;
typedef struct {
    int live, kind, n;
    V a, b;
    float life, total, size;
    uint32_t color;
    int team, anchor, anchorSerial;
} Fx;
typedef struct {
    wchar_t msg[100];
    float time;
    uint32_t color;
} Feed;
typedef struct {
    int sourceKind, sourceRole, sourceTeam, victimRole, victimTeam, first, voice;
} KillCard;
typedef struct {
    Unit u[MAXU];
    Shot shot[MAXP];
    Fx fx[MAXFX];
    Feed feed[4];
    KillCard cards[10];
    int cardCount, firstBlood;
    float cardLife, cardStart, coinSoundAt;
    uint32_t rng;
    int serial, winner, diff, autoplay, kills[2], towers[2], waves, castCount[3], sound[16], soundN;
    float time, wave, pay, markTime, noticeTime;
    V aim, mark;
    wchar_t notice[100];
    unsigned char nav[GW * GH], sight[2][GW * GH], garrisonSight[2][GW * GH];
    unsigned garrisonMask;
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
int game_fx_position(const Game *g,const Fx *f,V *out);
int visible(const Game *g, int team, V pos);
void game_rally(Game *g, V pos);
int alive(const Unit *u);
int hostile(const Unit *a, const Unit *b);
void game_notice(Game *g, const wchar_t *msg);
#endif
