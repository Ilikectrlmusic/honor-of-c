#include "animation.h"
#include "draw.h"
#include "motion_data.h"
#include "roster_motion.h"
#include <math.h>

#define PI 3.14159265f
#define TAU (2 * PI)
static float sat(float x) {
    return clampf(x, 0, 1);
}
static float ease(float t) {
    t = sat(t);
    return t * t * (3 - 2 * t);
}
static float fade(float t, float from) {
    return 1 - ease((t - from) / (1 - from));
}
static float seed(int i) {
    unsigned n = (unsigned)i * 747796405u + 2891336453u;
    n = ((n >> ((n >> 28) + 4)) ^ n) * 277803737u;
    return ((n >> 22) ^ n) / 4294967296.f;
}

static int sheet_for(int m){return m<9?ART_MOTION0+m/3:ART_MOTION3+(m-9)/3;}
static int height_for(int m){return m<9?motionHeight[m]:extraHeight[m-9];}
static int pivot_for(int m,int frame){return m<9?motionPivot[m][frame]:extraPivot[m-9][frame];}
void motion_frame(int model, int team, int row, int column, int facing, V feet, float height, float opacity) {
    int index = row * 6 + column;
    float scale = height * .94f / height_for(model);
    int pivot = pivot_for(model,index), flip = facing < 0;
    float x = feet.x - (flip ? 160 - pivot : pivot) * scale, y = feet.y - 157 * scale;
    int fr = (model % 3) * 6 + ((team == 1 && model != 8) ? 3 : 0) + row;
    art(sheet_for(model), x, y, 160 * scale, 160 * scale, 160.f * column, 160.f * fr, 160, 160,
        opacity, flip);
}
void motion_actor(const Unit *u, V feet, float h, float alpha) {
    int model = u->kind == HERO     ? (u->role<5?u->role:u->role+4)
                : u->kind == MELEE  ? 5
                : u->kind == RANGED ? 6
                : u->kind == SIEGE  ? 7
                                    : 8;
    int r = 1, f = 0, dir = u->dir;
    if (u->anim > 0 && u->actionTime > 0) {
        float t = sat(1 - u->anim / u->actionTime);
        r = u->pose == POSE_ULT ? 2 : 1;
        /* Sword, shield, spear and fist make contact on frame 3; bow releases on 4. */
        f = t < .15f ? 0 : t < .31f ? 1 : t < .50f ? 2 : t < .66f ? 3 : t < .84f ? 4 : 5;
        if (r == 2 && model == 1)
            f = t < .10f ? 0 : t < .21f ? 1 : t < .31f ? 2 : t < .41f ? 3 : t < .71f ? 4 : 5;
        if (model == 8) { /* lift, hold the stone aloft, commit at contact, recover */
            float sec = u->actionTime - u->anim;
            f = sec < .14f            ? 0
                : sec < .28f          ? 1
                : sec < STONE_CONTACT ? 2
                : sec < .84f          ? 3
                : sec < .95f          ? 4
                                      : 5;
        }
        dir = u->poseDir;
        if (r == 2 && model == 3 && t < .5f)
            feet.y -= sinf(2 * PI * t) * h * .18f;
    } else if (u->moving > 0) {
        r = 0;
        f = (int)(u->stride * 6) % 6;
    }
    if (u->kind == HERO && u->pose == POSE_ATTACK && u->anim > 0 && u->moving > 0) {
        /* Keep the authored upper-body swing (including the weapon and its
           contact frame). Flex the same painting below the knees instead of
           cutting in legs from a differently poised sprite. */
        float scale = h*.94f/height_for(model);
        int pivot = pivot_for(model,r*6+f), flip = dir < 0;
        float x = feet.x-(flip?160-pivot:pivot)*scale, y=feet.y-157*scale;
        float sx=160.f*f, sy=160.f*((model%3)*6+(u->team==1?3:0)+r);
        float knee=157.f-height_for(model)*.32f;
        float gait=sinf(TAU*u->stride);
        int atlas=sheet_for(model);
        art(atlas,x,y,160*scale,knee*scale,sx,sy,160,knee,alpha,flip);
        for(int j=0;j<8;j++){
            float a=knee+(160-knee)*j/8, b=knee+(160-knee)*(j+1)/8;
            float bend=gait*(j+.5f)*(j+.5f)/64.f;
            float width=(160+16*bend)*scale, drift=dir*3*bend*scale;
            art(atlas,x+(160*scale-width)*.5f+drift,y+a*scale,width,(b-a)*scale,
                sx,sy+a,160,b-a,alpha,flip);
        }
    } else motion_frame(model, u->team, r, f, dir, feet, h, alpha);
}
/* Starburst and directional particles use stable seeds; never touch the simulation RNG. */
static void glint(V p, float size, uint32_t c, float a) {
    float q[] = {p.x,
                 p.y - size,
                 p.x + size * .19f,
                 p.y - size * .2f,
                 p.x + size * .7f,
                 p.y,
                 p.x + size * .17f,
                 p.y + size * .15f,
                 p.x,
                 p.y + size,
                 p.x - size * .17f,
                 p.y + size * .15f,
                 p.x - size * .7f,
                 p.y,
                 p.x - size * .19f,
                 p.y - size * .18f};
    poly(q, 8, c, a);
}
static void scatter(V p, float radius, float t, int count, uint32_t c, float z) {
    for (int i = 0; i < count; i++) {
        float ang = TAU * seed(i + 5), v = radius * (.35f + .65f * seed(i + 19)),
              d = (1 - (1 - t) * (1 - t)) * v;
        float x = p.x + cosf(ang) * d, y = p.y + sinf(ang) * d * .76f - 19 * z * sinf(PI * t);
        line(x - cosf(ang) * 9 * z * (1 - t), y - sinf(ang) * 6 * z * (1 - t), x, y, c, (1 - t) * .9f,
             1.8f * z);
    }
}
static void rune(V a, float r, uint32_t c, float alpha) {
    oval(a.x, a.y, r, r, c, alpha * .065f, 1, 0);
    oval(a.x, a.y, r, r, c, alpha * .80f, 0, 1.6f);
    for (int j = 0; j < 4; j++) {
        float ang = j * PI * .5f;
        line(a.x + cosf(ang) * (r - 8), a.y + sinf(ang) * (r - 8), a.x + cosf(ang) * (r + 3),
             a.y + sinf(ang) * (r + 3), c, alpha, 2);
    }
}
static void arrows(V a, float r, float z, float t, int embedded) {
    for (int i = 0; i < 16; i++) {
        float ang = i * 2.399963f, rad = r * .90f * sqrtf((i + .5f) / 16.f);
        V target = add(a, vec(cosf(ang) * rad, sinf(ang) * rad));
        if (!embedded) {
            float k = sat((t - .58f - seed(i + 61) * .13f) / .29f);
            if (k <= 0)
                continue;
            float y = (1 - k) * 132 * z, x = (1 - k) * 48 * z;
            effect_stamp(1, target.x + x + 5 * z, target.y - y - 17 * z, 49 * z, 49 * z, .34f,
                         .52f + .42f * k);
            if (k > .96f)
                oval(target.x, target.y, 6 * z, 2.5f * z, 0xa1f1ae, .7f, 0, 1);
        } else {
            effect_stamp(1, target.x + 4 * z, target.y - 13 * z, 35 * z, 35 * z, .34f, .87f * fade(t, .19f));
        }
    }
}
void paint_flame_wave(const Shot *s, V p, float z) {
    float angle = atan2f(s->v.y, s->v.x), k = sat(1 - s->life / .62f), alpha = fade(k, .90f);
    V dir = norm(s->v);
    for (int j = 2; j > 0; j--) {
        float trail = j * 22 * z;
        effect_stamp(0, p.x - dir.x * trail, p.y - 19 * z - dir.y * trail, 132 * z, 148 * z, angle,
                     .13f * alpha);
    }
    effect_stamp(0, p.x, p.y - 19 * z, (120 + 18 * sinf(k * PI)) * z, 153 * z, angle, alpha);
    for (int j = 0; j < 7; j++) {
        float w = fmodf(k * 2.3f + seed(j + 16), 1), side = (seed(j + 40) - .5f) * 85 * z;
        V spark = {p.x - dir.x * w * 110 * z - dir.y * side,
                   p.y - 18 * z - dir.y * w * 110 * z + dir.x * side};
        oval(spark.x, spark.y, (2.6f - 1.6f * w) * z, (2.6f - 1.6f * w) * z, 0xffc276, (1 - w) * alpha, 1, 0);
    }
}
/* Draw in the projectile's own basis, so even a vertical shot keeps its
   arrowhead, crest or brush nib pointed along the real swept collision. */
void paint_roster_shot(int role,const Shot *s,V p,float z) {
    V d=mul(norm(s->v),z), n=vec(-d.y,d.x);
    float flight=dist(s->start,s->p);
    uint32_t c=s->color;
    if(role==5){
        V last=add(p,mul(d,9));
        for(int j=0;j<6;j++){
            V tip=add(sub(p,mul(d,j*7.f)),mul(n,sinf(j*3.5f+flight*.07f)*3.5f));
            line(last.x,last.y,tip.x,tip.y,c,.76f,5*z);
            line(last.x,last.y,tip.x,tip.y,0xeceaff,.97f,1.5f*z);last=tip;
        }
    } else if(role==6){
        for(int j=0;j<2;j++){
            V last=p;
            for(int k=0;k<=12;k++){
                float a=(k-6)*.19f;
                V pt=add(p,add(mul(d,cosf(a)*12-11*j),mul(n,sinf(a)*(23-j*4))));
                if(k)line(last.x,last.y,pt.x,pt.y,j?c:0xc8fff3,.86f,(j?1.6f:3)*z);
                last=pt;
            }
        }
    } else if(role==7){
        glow(p.x,p.y,17*z,15*z,0xff8b38,.84f);
        oval(p.x,p.y,8*z,8*z,0x92411b,1,1,0);
        oval(p.x,p.y,7*z,7*z,0xffca7a,.98f,0,2*z);
        for(int k=0;k<6;k++){
            V v=add(sub(p,mul(d,8+5*k)),mul(n,sinf(k*2+flight*.07f)*4));
            oval(v.x,v.y,(3-k*.32f)*z,(3-k*.32f)*z,0xffa942,1-k*.12f,1,0);
        }
    } else if(role==8){
        V tail=sub(p,mul(d,25)), l=add(sub(p,mul(d,9)),mul(n,4)),r=sub(sub(p,mul(d,9)),mul(n,4));
        line(tail.x,tail.y,p.x,p.y,c,.88f,2*z);
        float q[]={p.x,p.y,l.x,l.y,r.x,r.y};poly(q,3,0xe5faff,1);
    } else {
        V last=add(p,mul(d,6));
        for(int k=0;k<12;k++){
            V tip=add(sub(p,mul(d,2.8f*k)),mul(n,sinf(flight*.045f+k*.58f)*k*.55f));
            float width=(1+6*sinf((k+1)*PI/13))*z;
            line(last.x,last.y,tip.x,tip.y,0xc9d9ca,.85f,width+2*z);
            line(last.x,last.y,tip.x,tip.y,0x233536,1,width);last=tip;
        }
    }
}
void paint_ultimate(const Fx *f, V a, V b, float z, int front) {
    float t = sat(1 - f->life / f->total), r = f->size * z;
    int cast = f->kind == FX_ULT_CAST;
    uint32_t col = f->color, team = teamColor[f->team];
    if (f->n >= 5) {
        if(!front) {
            if(cast) rune(a,r,team,.66f);
            float radius=r*(cast?.84f+.16f*t:.23f+.79f*ease(t));
            oval(a.x,a.y,radius,radius*.84f,col,cast?.35f+.35f*t:(1-t)*.78f,0,(cast?1.6f:4.2f)*z);
            if(!cast) glow(a.x,a.y,r*.86f,r*.65f,col,(1-t)*.37f);
            return;
        }
        float energy=cast?.24f+t*.55f:fade(t,.18f);
        if(f->n==5){
            for(int j=0;j<3;j++){
                V last=a;
                for(int k=1;k<=5;k++){
                    V n={a.x+((j-1)*.17f*k+(k%2?.17f:-.12f))*r*(cast?.52f:1),
                         a.y-(k*.29f*r)*(cast?t:1-.33f*t)};
                    line(last.x,last.y,n.x,n.y,col,energy*.6f,(cast?2:7)*z);
                    line(last.x,last.y,n.x,n.y,0xfaf6ff,energy,(cast?1:2.4f)*z);last=n;
                }
            }
        } else if(f->n==6){
            for(int j=0;j<3;j++){
                float rad=r*(.42f+.24f*j),phase=t*1.65f+j*.43f;
                V prev=a;
                for(int k=0;k<16;k++){
                    float ang=phase+k*.183f;
                    V n={a.x+cosf(ang)*rad,a.y+sinf(ang)*rad*.58f-(cast?t:1-t)*r*.22f};
                    if(k)line(prev.x,prev.y,n.x,n.y,k>10?0xdbfff1:col,energy,(2+2*sinf(k*3.14f/16))*z);
                    prev=n;
                }
            }
        } else if(f->n==7){
            if(cast){
                V n={a.x+(1-t)*r*.50f,a.y-r*(1-t*t)*1.6f};
                line(n.x,n.y,n.x+13*z,n.y-36*z,0xff7d32,energy,6*z);
                glow(n.x,n.y,14*z,18*z,0xffe696,energy);glint(n,8*z,0xffd576,energy);
            }else{scatter(a,r,t,16,0xff9b36,z);glint(a,(1-t)*r*.56f,0xffdd8c,energy);}
        } else if(f->n==8){
            for(int j=0;j<8;j++){
                float ang=TAU*j/8, s=cast?.20f+.40f*t:.5f+.45f*ease(t);
                V tip={a.x+cosf(ang)*r*s,a.y+sinf(ang)*r*s*.76f-9*z};
                float dx=cosf(ang),dy=sinf(ang)*.76f;
                float q[]={a.x,a.y, a.x+dx*r*s*.55f-dy*7*z,a.y+dy*r*s*.55f+dx*7*z,
                           tip.x,tip.y,a.x+dx*r*s*.55f+dy*6*z,a.y+dy*r*s*.55f-dx*6*z};
                poly(q,4,col,energy*.75f);line(a.x,a.y,tip.x,tip.y,0xecffff,energy,1.5f*z);
            }
        } else {
            V prev=a;
            for(int j=0;j<22;j++){
                float k=j/21.f,ang=k*5.8f+t*2.2f,rad=r*(.25f+.58f*k)*(cast?.65f+t*.35f:1);
                V n={a.x+cosf(ang)*rad,a.y+sinf(ang)*rad*.68f};
                if(j){line(prev.x,prev.y,n.x,n.y,0xd3e8cd,.8f*energy,(2+11*sinf(k*3.14f))*z);
                      line(prev.x,prev.y,n.x,n.y,0x182b33,energy,(1+8*sinf(k*3.14f))*z);}
                prev=n;
            }
            if(!cast) scatter(a,r,t,9,0x668783,z);
        }
    } else if (f->n == 0) { // Vermilion: overhead draw, then a travelling fire crescent.
        float ang = atan2f(b.y - a.y, b.x - a.x);
        if (cast && front) {
            V hand = {a.x + cosf(ang) * 19 * z, a.y - 54 * z + sinf(ang) * 15 * z};
            glint(hand, (4 + 15 * t) * z, 0xffdc86, .35f + .64f * t);
        } else if (!cast && !front) {
            effect_stamp(5, a.x, a.y, r * (1.2f + t), r * (.76f + .4f * t), ang, fade(t, .21f) * .86f);
        } else if (!cast && front)
            scatter(a, r, t, 13, 0xffbc66, z);
    } else if (f->n == 1) { // Jade: skyward release, feather seal and staggered arrow shafts.
        if (!front) {
            if (cast)
                rune(a, r, team, .82f);
            effect_stamp(6, a.x, a.y, r * 2, r * 1.7f, 0, cast ? .53f + .34f * t : fade(t, .12f) * .8f);
            if (!cast)
                oval(a.x, a.y, r * (.52f + .52f * t), r * (.52f + .52f * t), col, (1 - t) * .75f, 0, 2 * z);
        } else {
            arrows(a, r, z, t, !cast);
            if (!cast)
                scatter(a, r, t, 12, 0xc3fcab, z);
        }
    } else if (f->n == 2) { // Lunar: fixed orbital seal, one falling comet, then a broken star.
        if (!front) {
            if (cast)
                rune(a, r, team, .77f);
            effect_stamp(7, a.x, a.y, r * (1.85f + .12f * t), r * (1.85f + .12f * t), .25f * t,
                         cast ? .45f + .43f * t : fade(t, .16f) * .8f);
        } else if (cast) {
            float fall = t * t;
            float x = a.x + (1 - fall) * 128 * z, y = a.y - (1 - fall) * 232 * z;
            /* The luminous head is at the lower-left of the texture, not its centre. */
            float w = (62 + 95 * t) * z;
            effect_stamp(2, x + w * .26f, y - w * .24f, w, w, 0, .38f + .62f * t);
        } else {
            glint(a, r * .69f * (1 - t), 0xf5dcff, fade(t, .12f) * .85f);
            oval(a.x, a.y, r * (.16f + .88f * ease(t)), r * (.16f + .88f * ease(t)), 0xd8aaff, (1 - t) * .9f,
                 0, (5 - 3 * t) * z);
            scatter(a, r, t, 22, 0xe4b5ff, z);
        }
    } else if (f->n == 3) { // Mountain: shield lift, grounded cracks and a rising wall of stone.
        if (cast && !front) {
            rune(b, r, team, .31f + .45f * t);
            effect_stamp(8, b.x, b.y, r * (1 + t * .5f), r * (1 + t * .5f), 0, t * .46f);
        } else if (!cast && !front) {
            effect_stamp(8, a.x, a.y, r * (1.53f + .48f * ease(t * 3)), r * (1.43f + .47f * ease(t * 3)), 0,
                         fade(t, .48f));
            oval(a.x, a.y, r * (.20f + .80f * ease(t * 2)), r * (.20f + .80f * ease(t * 2)), 0xecc87c,
                 fade(t, .12f) * .75f, 0, 2 * z);
        } else if (!cast && front) {
            for (int j = 0; j < 7; j++) {
                float ang = j * TAU / 7 + .2f, part = sat((t - (j % 3) * .035f) / .24f);
                float h = (55 + 35 * seed(j + 99)) * z * ease(part);
                V v = {a.x + cosf(ang) * r * .67f, a.y + sinf(ang) * r * .67f};
                effect_stamp(3, v.x, v.y - h * .39f, 88 * z, h, cosf(ang) * .22f, fade(t, .41f));
            }
            scatter(a, r, t, 13, 0xffcc7c, z);
        }
    } else { // Night: silhouettes left along the actual dash, then a crossing double-cut.
        if (cast && !front) {
            effect_stamp(9, a.x, a.y, 90 * z, 48 * z, 0, .39f + .37f * t);
        } else if (!cast && !front) {
            effect_stamp(9, a.x, a.y, r * 1.48f, r * .83f, 0, fade(t, .27f) * .76f);
            for (int j = 0; j < 3; j++) {
                V ghost = add(b, mul(sub(a, b), j * .28f + .08f));
                motion_frame(4, f->team, 2, 1 + j, a.x >= b.x ? 1 : -1, ghost, 83 * z,
                             (.16f + .07f * j) * fade(t, .12f));
            }
        } else if (!cast && front) {
            float grow = .70f + .39f * ease(t * 3);
            effect_stamp(4, a.x, a.y - 28 * z, r * 1.7f * grow, r * 1.5f * grow, -.11f, fade(t, .29f));
            scatter(a, r * .88f, t, 12, 0x94edff, z);
        }
    }
}
/* A layered fountain arrival: a grounded, rotating seal beneath the feet,
   shafts of light behind the body and a small spiral of motes in front. */
void paint_gate(V a, float z, float t, float scale, int front, int channel) {
    t = sat(t);
    float gain = channel ? (.38f + .55f * t) : (.32f + .68f * ease(t / .13f)) * fade(t, .46f);
    if (gain < .005f)
        return;
    float r = (channel ? 30 + 12 * t : 36 + 12 * ease(t * 3)) * z * scale;
    float h = (channel ? 90 + 141 * t : 211 + 54 * (1 - ease(t * 2.8f))) * z * scale;
    float phase = channel ? t * TAU * 2 : t * TAU * .67f;
    if (!front) {
        oval(a.x, a.y, r * 1.29f, r * .53f, 0x2b7dda, gain * .20f, 1, 0);
        oval(a.x, a.y, r, r * .41f, 0x8cefff, gain * .88f, 0, 2.2f * z);
        oval(a.x, a.y, r * .84f, r * .34f, 0x6db9ff, gain * .71f, 0, 1.2f * z);
        for (int i = 0; i < 12; i++) {
            float q = phase + i * TAU / 12, c = cosf(q), s = sinf(q);
            line(a.x + c * r * .88f, a.y + s * r * .36f, a.x + c * r * 1.08f, a.y + s * r * .44f, 0xd4ffff,
                 gain * .9f, 1.4f * z);
        }
        /* Cobalt outer veil, cyan shaft, and white core. Split the top
           and bottom fades: the bright section is above the body, too. */
        for (int j = 0; j < 4; j++) {
            float w = r * (.79f - .155f * j), shoulder = .69f * h;
            uint32_t hue = j < 2 ? 0x338cff : 0x81efff;
            float strength = (.13f + j * .048f) * gain;
            gradient(a.x - w, a.y - h, 2 * w, h - shoulder, hue, 0, hue, strength, 1);
            gradient(a.x - w, a.y - shoulder, 2 * w, shoulder, hue, strength, 0x52cfff, strength * .88f, 1);
        }
        for (int j = 0; j < 7; j++) {
            float dx = r * (j - 3) * .236f, v = (.78f + .22f * seed(j + 88)) * h;
            float base = a.y + r * .12f * cosf(j * 1.3f);
            float w = (j == 3 ? 2.7f : 1.15f) * z * scale;
            gradient(a.x + dx - w, base - v, 2 * w, v * .29f, 0x98efff, 0, 0xaffaff, gain * .77f, 1);
            gradient(a.x + dx - w, base - v * .71f, 2 * w, v * .71f, 0xaffaff, gain * .77f, 0xcbffff,
                     gain * .36f, 1);
        }
        if (!channel) {
            float arrival = ease(t / .44f), head = a.y - h * (1 - arrival);
            glint(vec(a.x, head), (8 + 6 * (1 - arrival)) * z * scale, 0xecffff, gain * (1 - arrival));
            line(a.x - 13 * z * scale, head, a.x + 13 * z * scale, head, 0xb2faff, gain * (1 - arrival),
                 1.2f * z);
        }
        if (!channel) {
            float pulse = ease(t * 2.7f);
            oval(a.x, a.y, (.3f + 1.18f * pulse) * r, (.3f + 1.18f * pulse) * r * .42f, 0xaaeaff,
                 gain * (1 - pulse), 0, 3 * z);
        }
    } else {
        for (int j = 0; j < 11; j++) {
            float k = fmodf((channel ? t * 2.7f : t * 1.2f) + seed(j + 42), 1);
            float th = phase + k * 3.8f + j * 2.4f;
            float x = a.x + cosf(th) * r * (.55f + .23f * k), y = a.y - h * k + sinf(th) * 8 * z;
            float a1 = sinf(PI * k) * gain;
            line(x, y - 6 * z, x, y + 3 * z, 0xa4e5ff, a1 * .88f, 1.15f * z);
            if (j % 3 == 0)
                glint(vec(x, y), 3.8f * z, 0xe4ffff, a1);
        }
    }
}
void paint_stone_tell(const Unit *u, V feet, V a, float z) {
    float elapsed = STONE_CONTACT - u->releaseIn;
    float k = sat(elapsed / STONE_CONTACT), r = STONE_RADIUS * z;
    int locked = elapsed >= STONE_TRACK;
    uint32_t c = locked ? 0xff815c : 0xffce72;
    oval(a.x, a.y, r, r, c, locked ? .18f : .105f, 1, 0);
    oval(a.x, a.y, r, r, 0x392c2b, .76f, 0, 4 * z);
    oval(a.x, a.y, r, r, c, .96f, 0, 1.8f * z);
    float inner = r * (1 - k);
    oval(a.x, a.y, inner, inner, 0xffdec1, .86f, 0, 1.65f * z);
    for (int i = 0; i < 4; i++) {
        float q = (i + .5f) * PI * .5f, dx = cosf(q), dy = sinf(q);
        line(a.x + dx * (r - 6 * z), a.y + dy * (r - 6 * z), a.x + dx * (r + 4 * z), a.y + dy * (r + 4 * z),
             c, .96f, 2.2f * z);
    }
    V d = norm(sub(a, feet));
    line(feet.x, feet.y, a.x - d.x * r, a.y - d.y * r, c, .58f, 1.5f * z);
    glint(a, (locked ? 7 : 4) * z, 0xffe4a6, .96f);
}
void paint_stone_hit(V a, float z, float t) {
    t = sat(t);
    float r = STONE_RADIUS * z, s = r * (.42f + .72f * ease(t * 2.3f));
    effect_stamp(8, a.x, a.y, 2 * s, 1.62f * s, 0, fade(t, .45f) * .96f);
    oval(a.x, a.y, s, s * .76f, 0xffcb7b, 1 - t, 0, (3.4f - 2 * t) * z);
    for (int j = 0; j < 8; j++) {
        float q = TAU * j / 8 + .25f, d = r * (.4f + .73f * t);
        V p = {a.x + cosf(q) * d, a.y + sinf(q) * d * .73f - sinf(t * PI) * (10 + 10 * seed(j + 3)) * z};
        float h = (3 + 3 * seed(j + 7)) * z * (1 - t);
        float pts[] = {p.x - h,        p.y,           p.x,           p.y - h * 1.5f,
                       p.x + h * 1.1f, p.y - h * .4f, p.x + h * .4f, p.y + h};
        poly(pts, 4, 0xe4b07e, 1 - t);
    }
}
void paint_coins(const Fx *f, V a, V target, float z) {
    float t = sat(1 - f->life / f->total), scale = f->size * z;
    int count = f->n < 10 ? 3 : f->n >= 100 ? 9 : 6;
    for (int i = 0; i < count; i++) {
        float k = sat((t - i * .012f) / (.99f - i * .012f));
        float q = TAU * (i * .618034f + seed(f->n) * .23f);
        V arc = vec(cosf(q) * (23 + 14 * seed(i + 13)) * scale, sinf(q) * 19 * scale);
        V start = add(a, vec(0, -10 * z));
        float spread = ease(k / .41f);
        V point = add(start, mul(arc, spread));
        point.y -= (14 + 26 * seed(i + 67)) * scale * sinf(PI * sat(k / .52f));
        float take = ease((k - .48f) / .52f);
        point = add(point, mul(sub(target, point), take));
        point.y -= sinf(PI * take) * 29 * scale;
        float alpha = (.36f + .64f * ease(k / .08f)) * fade(k, .90f);
        float ry = (4.6f + seed(i + 21) * 1.25f) * scale;
        float rx = ry * (.26f + .74f * fabsf(cosf((k * 2.1f + seed(i + 8)) * TAU)));
        if (take > .03f && take < .95f) {
            V d = norm(sub(target, point));
            line(point.x - d.x * 9 * scale, point.y - d.y * 9 * scale, point.x, point.y, 0xffbf45,
                 alpha * .55f, 1.7f * scale);
        }
        oval(point.x + .7f * scale, point.y + 1.1f * scale, rx + scale, ry + scale, 0x804512, alpha, 1, 0);
        oval(point.x, point.y, rx, ry, 0xffcb4f, alpha, 1, 0);
        oval(point.x, point.y, rx * .73f, ry * .75f, 0xffee98, alpha, 0, .85f * scale);
        box(point.x - rx * .24f, point.y - ry * .23f, rx * .48f, ry * .46f, 0xa6601f, alpha);
        if (i % 3 == 0)
            glint(vec(point.x - rx * .50f, point.y - ry * .62f), 3.2f * scale, 0xfff8c8, alpha);
    }
}
void paint_coin_amount(const Fx *f, V a, V target, float z) {
    float t = sat(1 - f->life / f->total), scale = f->size * z;
    wchar_t b[20];
    swprintf(b, 20, L"+%d", f->n);
    float y = a.y - (36 + 22 * ease(t)) * z, alpha = fade(t, .74f);
    a.x += (target.x > a.x ? -38 : 38) * scale;
    label(b, a.x - 51, y + 1, 102, 24, 18, 0x4a3018, alpha, F_BOLD, 1);
    label(b, a.x - 52, y, 102, 24, 18, 0xffdf71, alpha, F_BOLD, 1);
}
