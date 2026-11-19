#include "living.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define STEP 12
#define NCH 5
#define TAU 6.28318530718f
typedef struct {
    float cx, cy, rx, ry, dx, dy, angle, phase, wave;
    int channel;
} Pin;
typedef struct {
    float a[NCH][4];
    int du, dv, active;
} Node;
/* Coordinates for the eyelid line, in original illustration pixels. */
typedef struct {
    float x1, y1, x2, y2, depth;
} Lid;
typedef struct {
    int x, y, w, h;
    float x1, y1, x2, y2, depth;
    uint32_t tile[64 * 40];
} Eye;
struct Living {
    int model, w, h, nx, ny;
    uint32_t *ink, *dst;
    Node *mesh;
    unsigned char *tiles;
    Eye eye[2];
    int neye;
};
static float sat(float t) {
    return fminf(1, fmaxf(0, t));
}
static float smooth(float t) {
    t = sat(t);
    return t * t * (3 - 2 * t);
}
static float zone(float x, float y, float cx, float cy, float rx, float ry, float angle) {
    float c = cosf(angle), s = sinf(angle);
    float px = ((x - cx) * c + (y - cy) * s) / rx, py = (-(x - cx) * s + (y - cy) * c) / ry;
    return smooth((1 - px * px - py * py) * 1.55f);
}
/* Values are percentages of the illustration. The falloff is compact:
   mountain ridges, feet and paper edges don't slide with the character. */
static const Pin title[] = {
    {.767, .345, .112, .196, -.09, -.37, 0, .1, 0, 0},  /* chest and neck */
    {.755, .200, .100, .133, -.11, -.32, 0, .14, 0, 0}, /* the face translates as a rigid patch */
    {.826, .433, .114, .173, .13, -.30, -.25, .12, 0, 0},
    {.896, .208, .112, .072, .32, -1.19, .15, .32, 4, 1}, /* long tied hair */
    {.958, .222, .091, .075, .22, -.93, .12, .82, 5, 2},
    {.815, .124, .055, .050, .18, -.44, .1, -.3, 4, 1},
    {.639, .430, .114, .046, -.75, -.88, -.65, .24, 3, 1}, /* red scarf across shoulder */
    {.588, .489, .090, .045, .43, -1.28, -.64, .83, 5, 2},
    {.947, .368, .104, .087, .31, -1.12, .43, .7, 4, 1},
    {.975, .429, .092, .083, -.23, -.69, .6, -.2, 3, 2},
    {.417, .757, .194, .075, .44, -.86, -.48, -.4, 5, 1}, /* billowing lower cape */
    {.271, .852, .142, .079, -.46, -1.75, -.47, .2, 5, 2},
    {.193, .904, .113, .042, .38, -1.22, -.45, -.4, 6, 1},
    /* The right cape streams behind the fixed bracer. The lower brocade
       and outer skirt respond successively, not as a translated rectangle. */
    {.970, .555, .083, .138, .74, -1.23, .50, .7, 7, 1},
    {.941, .635, .049, .082, -.55, -.97, .62, 1.5, 8, 2},
    {.804, .837, .055, .158, -.49, .29, -.26, .26, 5, 1},
    {.922, .848, .095, .123, .89, -.54, -.49, 1.35, 7, 2},
    {.979, .914, .087, .120, 1.03, -.63, -.53, 2.0, 6, 1},
    {.567, .698, .057, .103, -.08, -.21, .57, .2, 0, 0},
};
static const Pin personal[5][7] = {{{.24, .25, .23, .19, -1.28, -.95, -.6, .2, 4, 1},
                                    {.16, .46, .18, .30, -1.05, -.65, .36, .4, 5, 2},
                                    {.12, .69, .19, .28, 1.65, -1.12, .32, .4, 4, 1},
                                    {.86, .59, .17, .38, 1.92, -.94, -.24, -.4, 4, 2},
                                    {.79, .83, .24, .19, 1.29, .92, -.2, .68, 4, 1},
                                    {.30, .43, .23, .12, -.63, -.56, -.65, .4, 4, 1},
                                    {.61, .77, .16, .11, .28, -.38, .1, .6, 0, 3}},
                                   {{.81, .24, .24, .21, 1.71, -.96, .55, .6, 4, 1},
                                    {.88, .45, .17, .26, 1.53, -1.20, -.34, -.1, 4, 2},
                                    {.69, .24, .085, .08, .80, .18, 0, .7, 2, 2},
                                    {.16, .77, .17, .25, -1.71, .90, .42, .3, 4, 1},
                                    {.85, .69, .20, .32, 1.92, .79, -.37, .4, 4, 2},
                                    {.206, .25, .13, .28, -.64, -.48, -.18, -.05, 1, 3},
                                    {.30, .45, .16, .12, -.50, -.35, -.2, .05, 1, 3}},
                                   {{.23, .28, .21, .24, -1.74, -1.10, .25, .5, 4, 1},
                                    {.13, .47, .14, .24, -2.04, -1.38, .43, .6, 4, 2},
                                    {.83, .45, .18, .27, 1.64, -1.03, -.25, -.1, 4, 1},
                                    {.12, .70, .20, .29, 2.28, -.83, .38, .3, 5, 1},
                                    {.84, .73, .19, .29, 1.88, -.96, -.2, .42, 4, 2},
                                    {.51, .45, .20, .16, .88, -.91, .10, .2, .8, 3},
                                    {.448, .32, .055, .067, .61, -.19, 0, .23, 3, 2}},
                                   {{.53, .248, .145, .115, .64, -.52, .20, .4, 1.8, 1},
                                    {.142, .30, .15, .091, -.97, -.68, -.3, .7, 3, 2},
                                    {.135, .59, .174, .33, -1.43, -.66, .31, .4, 4, 1},
                                    {.087, .85, .13, .23, 1.32, -.78, .3, -.3, 4, 2},
                                    {.84, .46, .14, .21, 1.01, -.52, -.23, .6, 2, 1},
                                    {.565, .46, .076, .14, .83, .32, -.1, .3, 3, 2},
                                    {.361, .75, .18, .17, -.23, -.25, 0, .2, 0, 3}},
                                   {{.21, .44, .215, .18, -1.98, -1.12, -.15, -.2, 4, 1},
                                    {.12, .62, .14, .27, -1.75, 1.36, .20, .2, 4.5, 2},
                                    {.15, .82, .21, .23, 2.12, -1.27, .39, -.4, 4, 1},
                                    {.47, .22, .14, .11, -.75, -.55, -.4, .4, 2.5, 2},
                                    {.66, .27, .103, .102, .83, -.56, -.2, .4, 3, 1},
                                    {.649, .68, .095, .14, 1.31, -.29, -.2, .3, 3, 2},
                                    {.78, .42, .12, .20, -.42, -.53, .3, .41, 0, 3}}};
static const float pivot[5][2] = {{.543, .249}, {.500, .237}, {.534, .218}, {.566, .151}, {.657, .193}};
static const int fullWidth[] = {1536, 634, 634, 638, 759, 771},
                 fullHeight[] = {1024, 822, 822, 822, 1024, 1024};
static const int origin[] = {0, 0, 638, 1275, 0, 765};
static const Lid lids[6][2] = {{{1118, 199, 1137, 189, 4.2f}, {0, 0, 0, 0, 0}},
                               {{303, 186, 324, 188, 4.4f}, {354, 185, 375, 181, 5.4f}},
                               {{898, 209, 920, 212, 5.6f}, {945, 196, 971, 187, 6.1f}},
                               {{1589, 160, 1619, 162, 5.8f}, {1649, 171, 1677, 177, 5.1f}},
                               {{401, 137, 427, 142, 4.7f}, {463, 149, 481, 155, 3.5f}},
                               {{1234, 198, 1260, 205, 5.6f}, {1294, 206, 1318, 200, 5.3f}}};
static void weigh(Node *n, const Pin *p, float x, float y, int w, int h) {
    float z = zone(x, y, p->cx, p->cy, p->rx, p->ry, p->angle);
    if (z <= 0)
        return;
    float ph = p->phase + (x - p->cx + .64f * (y - p->cy)) * p->wave;
    float s = sinf(ph), c = cosf(ph), a = -z * w * p->dx * .01f, b = -z * h * p->dy * .01f;
    float *k = n->a[p->channel];
    k[0] += a * c;
    k[1] += a * s;
    k[2] += b * c;
    k[3] += b * s;
    n->active = 1;
}
/* Prop motions are baked into the same compact harmonic mesh as the characters.
   No landscape pixel is passed into this module. The bamboo roots, flag hoist
   and flame base are pinned, rather than moving a rectangular sprite. */
static void harmonic(Node *n,int ch,float dx,float dy,float ph) {
    float c=cosf(ph),s=sinf(ph);
    n->a[ch][0]+=dx*c; n->a[ch][1]+=dx*s;
    n->a[ch][2]+=dy*c; n->a[ch][3]+=dy*s;
    n->active=1;
}
static void prop_node(Node *n, int id, float x, float y, int w, int h) {
    float tip = (1 - y) * (1 - y);
    if (id == RIG_FLAME) {
        harmonic(n, 0, .043f * w * tip, .025f * h * tip, 9 * y - 4 * x);
        harmonic(n, 2, .020f * w * tip, .038f * h * tip, 15 * y + 9 * x);
        harmonic(n, 1, .014f * w * (1 - y), .019f * h * (1 - y), 11 * x - 5 * y);
    } else if (id == RIG_BAMBOO) {
        harmonic(n, 0, w * .087f * tip, h * .024f * tip, 1.7f * y);
        harmonic(n, 1, w * .023f * (1 - y), h * .011f * (1 - y), 3 * x + 4 * y);
    } else if (id == RIG_STANDARD) {
        float fly = smooth((x - .045f) / .95f);
        harmonic(n, 0, w * .034f * fly, h * .075f * fly, -7.6f * x + .8f * y);
        harmonic(n, 1, w * .012f * fly, h * .035f * fly, -12 * x + 1.45f * y);
    } else { /* Crow: the hinge is at the shoulder, not in the middle of the image. */
        float wing = smooth((.62f - y) / .56f);
        harmonic(n, 0, -w * .088f * wing, h * .182f * wing, 1.45f * (.6f - y));
        harmonic(n, 1, w * .012f * wing, h * .028f * wing, -2.9f * x);
    }
}
static void make_mesh(Living *l) {
    int id = l->model;
    for (int j = 0; j < l->ny; j++)
        for (int i = 0; i < l->nx; i++) {
            Node *n = &l->mesh[j * l->nx + i];
            float x = fminf(i * STEP, l->w) / (float)l->w, y = fminf(j * STEP, l->h) / (float)l->h;
            if (id >= RIG_FLAME)
                prop_node(n, id, x, y, l->w, l->h);
            else if (!id) {
                for (unsigned k = 0; k < sizeof title / sizeof title[0]; k++)
                    weigh(n, &title[k], x, y, l->w, l->h);
            } else {
                const float *p = pivot[id - 1];
                Pin torso = {.51f, .47f, .47f, .36f, (x - .50f) * 1.22f, -.74f, 0, .08f, 0, 0};
                Pin head = {p[0], p[1], .25f, .21f, -.38f, -.63f, 0, .12f, 0, 0};
                Pin tilt = {p[0], p[1], .25f, .21f, (y - p[1]) * .84f, (p[0] - x) * .88f, 0, .4f, 0, 3};
                weigh(n, &torso, x, y, l->w, l->h);
                weigh(n, &head, x, y, l->w, l->h);
                weigh(n, &tilt, x, y, l->w, l->h);
                for (int k = 0; k < 7; k++)
                    weigh(n, &personal[id - 1][k], x, y, l->w, l->h);
            }
        }
    for (int j = 0; j < l->ny - 1; j++)
        for (int i = 0; i < l->nx - 1; i++) {
            Node *n = l->mesh + j * l->nx + i;
            l->tiles[j * (l->nx - 1) + i] =
                (unsigned char)(n[0].active || n[1].active || n[l->nx].active || n[l->nx + 1].active);
        }
}
static unsigned mix8(unsigned a, unsigned b, int f) {
    const unsigned m = 0xff00ff;
    return ((((a & m) * (256 - f) + (b & m) * f) >> 8) & m) |
           (((((a >> 8) & m) * (256 - f) + ((b >> 8) & m) * f)) & 0xff00ff00u);
}
static unsigned sample(const unsigned *p, int w, int h, int x, int y) {
    x = x<0 ? 0 : x>(w - 1) * 256 - 1 ? (w - 1) * 256 - 1 : x;
    y = y<0 ? 0 : y>(h - 1) * 256 - 1 ? (h - 1) * 256 - 1 : y;
    const unsigned *q = p + (y >> 8) * w + (x >> 8);
    return mix8(mix8(q[0], q[1], x & 255), mix8(q[w], q[w + 1], x & 255), y & 255);
}
static void eyes_init(Living *l) {
    float sx = (float)l->w / fullWidth[l->model], sy = (float)l->h / fullHeight[l->model];
    for (int i = 0; i < 2; i++) {
        Lid p = lids[l->model][i];
        if (!p.x1)
            continue;
        Eye *e = l->eye + l->neye;
        e->x1 = (p.x1 - origin[l->model]) * sx;
        e->x2 = (p.x2 - origin[l->model]) * sx;
        e->y1 = p.y1 * sy;
        e->y2 = p.y2 * sy;
        e->depth = p.depth * sy;
        e->x = (int)floorf(e->x1) - 2;
        e->y = (int)floorf(fminf(e->y1, e->y2) - 2.1f * e->depth) - 2;
        e->w = (int)ceilf(e->x2) - e->x + 4;
        e->h = (int)ceilf(fmaxf(e->y1, e->y2) + e->depth * 1.65f) - e->y + 4;
        if (e->w > 64 || e->h > 40 || e->x < 0 || e->y < 0 || e->x + e->w >= l->w || e->y + e->h >= l->h)
            continue;
        for (int j = 0; j < e->h; j++)
            memcpy(e->tile + j * e->w, l->ink + (e->y + j) * l->w + e->x, e->w * 4);
        l->neye++;
    }
}
static void eyelids(Living *l, float blink) {
    for (int i = 0; i < l->neye; i++) {
        Eye *e = &l->eye[i];
        for (int j = 0; j < e->h; j++) {
            unsigned *out = l->ink + (e->y + j) * l->w + e->x;
            memcpy(out, e->tile + j * e->w, e->w * 4);
            if (blink < .001f)
                continue;
            for (int k = 1; k < e->w - 1; k++) {
                float t = (k + e->x - e->x1) / (e->x2 - e->x1);
                if (t <= 0 || t >= 1)
                    continue;
                float shape = 4 * t * (1 - t),
                      top = e->y1 + (e->y2 - e->y1) * t - e->y - .32f * e->depth * shape;
                float bottom = top + e->depth * 1.34f * shape, upper = top - 1.75f * e->depth;
                float lid = top + (bottom - top) * blink * .97f;
                if (j <= upper || j >= bottom)
                    continue;
                float sy = j < lid ? upper + (j - upper) * (top - upper) / (lid - upper)
                                   : top + (j - lid) * (bottom - top) / (bottom - lid);
                out[k] = sample(e->tile, e->w, e->h, k * 256, (int)(sy * 256));
            }
        }
    }
}
Living *living_new(int model, int w, int h, const uint32_t *p) {
    if (model < 0 || model >= RIG_COUNT || !p || w < 2 || h < 2)
        return NULL;
    Living *l = calloc(1, sizeof *l);
    if (!l)
        return NULL;
    l->w = w;
    l->h = h;
    l->model = model;
    l->nx = (w + STEP - 1) / STEP + 1;
    l->ny = (h + STEP - 1) / STEP + 1;
    l->mesh = calloc(l->nx * l->ny, sizeof(Node));
    l->tiles = calloc(l->nx * l->ny, 1);
    l->ink = malloc((size_t)w * h * 4);
    l->dst = malloc((size_t)w * h * 4);
    if (!l->mesh || !l->tiles || !l->ink || !l->dst) {
        living_free(l);
        return NULL;
    }
    memcpy(l->ink, p, (size_t)w * h * 4);
    memcpy(l->dst, p, (size_t)w * h * 4);

    make_mesh(l);
    if (model < RIG_FLAME) eyes_init(l);
    return l;
}
void living_free(Living *l) {
    if (!l)
        return;
    free(l->ink);
    free(l->dst);
    free(l->tiles);
    free(l->mesh);
    free(l);
}
const uint32_t *living_frame(Living *l, float seconds) {
    if (!l)
        return NULL;
    int id = l->model, w = l->w, h = l->h;
    float t = seconds + .31f * id, rate = id == 4 ? .87f : id == 5 ? 1.08f : 1;
    if (id == RIG_FLAME)
        rate = 3.05f;
    if (id == RIG_STANDARD)
        rate = 1.82f;
    if (id == RIG_RAVEN)
        rate = 5.30f;
    const float periods[NCH] = {3.95f, 3.07f, 4.81f, 5.90f, 7.1f};
    float ss[NCH], cc[NCH];
    for (int i = 0; i < NCH; i++) {
        float a = t * TAU * rate / periods[i];
        ss[i] = sinf(a);
        cc[i] = cosf(a);
    }
    float beat = fmodf(t + .73f * id, 4.72f + .19f * id);
    float blink = beat < 1.75f   ? 0
                  : beat < 1.85f ? smooth((beat - 1.75f) / .10f)
                                 : 1 - smooth((beat - 1.85f) / .17f);
    eyelids(l, blink);
    for (int k = 0; k < l->nx * l->ny; k++) {
        Node *n = l->mesh + k;
        if (!n->active)
            continue;
        float x = 0, y = 0;
        for (int c = 0; c < NCH; c++) {
            float *a = n->a[c];
            x += a[0] * ss[c] + a[1] * cc[c];
            y += a[2] * ss[c] + a[3] * cc[c];
        }
        n->du = (int)(x * 256);
        n->dv = (int)(y * 256);
    }
    for (int by = 0; by < h; by += STEP)
        for (int bx = 0; bx < w; bx += STEP) {
            int cell = (by / STEP) * (l->nx - 1) + bx / STEP;
            if (!l->tiles[cell])
                continue;
            Node *n = l->mesh + (by / STEP) * l->nx + bx / STEP;
            int tw = w - bx < STEP ? w - bx : STEP, th = h - by < STEP ? h - by : STEP;
            for (int yy = 0; yy < th; yy++) {
                int dy = by + yy;
                int ax = n->du + (n[l->nx].du - n->du) * yy / th,
                    ay = n->dv + (n[l->nx].dv - n->dv) * yy / th;
                int ex = n[1].du + (n[l->nx + 1].du - n[1].du) * yy / th,
                    ey = n[1].dv + (n[l->nx + 1].dv - n[1].dv) * yy / th;
                int du = (ex - ax) * 16 / tw, dv = (ey - ay) * 16 / tw;
                int u = bx * 4096 + ax * 16, v = dy * 4096 + ay * 16;
                int k = dy * w + bx;
                for (int xx = 0; xx < tw; xx++, k++, u += 4096 + du, v += dv) {
                    int qx = u >> 4, qy = v >> 4;
                    unsigned ink = sample(l->ink, w, h, qx, qy);
                    l->dst[k] = ink;
                }
            }
        }
    return l->dst;
}
