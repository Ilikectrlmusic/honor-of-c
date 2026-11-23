#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "game.h"
#define RATE 22050
#define CHUNK 512
#define TAU 6.28318530718f
enum { MUSIC = SOUND_COUNT, QUEUE = 6 };
typedef struct {
    const int16_t *pcm;
    int frames, channels;
} Clip;
static Clip bank[SOUND_COUNT + 1];
static short synth[VO_COUNTDOWN][RATE];
static short accent[3][RATE / 2];
static HWAVEOUT out;
static WAVEHDR heads[4];
static short buf[4][CHUNK * 2];
static struct {
    int id, pos;
} sfx[12];
typedef struct {
    int id;
    unsigned tick;
} Cue;
static Cue queue[QUEUE];
static int active = -1, speechPos, pending, mute, enabled, musicPos, sfxSlot;
static unsigned sampleTick;
static float musicGain, duck = 1;
static unsigned u32(const unsigned char *p) {
    return p[0] | (unsigned)p[1] << 8 | (unsigned)p[2] << 16 | (unsigned)p[3] << 24;
}
static int pcm_resource(int id, int res, int channels) {
    HRSRC r = FindResourceW(NULL, MAKEINTRESOURCEW(res), MAKEINTRESOURCEW(10));
    if (!r)
        return 0;
    unsigned size = SizeofResource(NULL, r);
    const unsigned char *p = LockResource(LoadResource(NULL, r));
    if (!p || size < 44 || memcmp(p, "RIFF", 4) || memcmp(p + 8, "WAVE", 4))
        return 0;
    int fmt = 0;
    for (unsigned i = 12; size - i >= 8;) {
        unsigned n = u32(p + i + 4);
        if (n > size - i - 8)
            return 0;
        const unsigned char *v = p + i + 8;
        if (!memcmp(p + i, "fmt ", 4) && n >= 16)
            fmt =
                v[0] == 1 && v[1] == 0 && v[2] == channels && v[3] == 0 && u32(v + 4) == RATE && v[14] == 16;
        if (!memcmp(p + i, "data", 4) && fmt && n >= 4) {
            bank[id] = (Clip){(const int16_t *)v, (int)n / (channels * 2), channels};
            return 1;
        }
        i += 8 + n;
        if ((n & 1) && i < size)
            i++;
    }
    return 0;
}
static int prepare(void) {
    int ok = pcm_resource(MUSIC, 390, 2);
    for (int i = VO_COUNTDOWN; i < SOUND_COUNT; i++)
        ok &= pcm_resource(i, 300 + i - VO_COUNTDOWN, 1);
    const float hz[] = {250,130,470,170,590,890,700,420,590,520,240,127,78,1650,294,392,494,247,330,370,523,220,587,277,110,62};
    const float dur[] = {.12f,.13f,.29f,.57f,.42f,.24f,.42f,.35f,.12f,.88f,.80f,.095f,.52f,.39f,.38f,.38f,.38f,.38f,.38f,.38f,.38f,.38f,.38f,.38f,.82f,.98f};
    _Static_assert(sizeof hz/sizeof *hz==VO_COUNTDOWN && sizeof dur/sizeof *dur==VO_COUNTDOWN, "missing synthesized sound");
    uint32_t rng = 132;
    for (int i = 0; i < VO_COUNTDOWN; i++) {
        int n = (int)(RATE * dur[i]);
        bank[i] = (Clip){synth[i], n, 1};
        float ph = 0, low = 0;
        for (int j = 0; j < n; j++) {
            float t = (float)j / RATE, x = t / dur[i];
            rng = rng * 1664525u + 1013904223u;
            float noise = (rng >> 8) / 8388608.f - 1;
            low += (noise - low) * .12f;
            float hz2 = hz[i] * (i < 2          ? 1.5f - x
                                 : i == S_SKILL ? 1 + x * .8f
                                 : i == S_ULT   ? .75f + x * .7f
                                 : i == S_WIN   ? (x < .33f   ? 1
                                                   : x < .64f ? 1.25f
                                                              : 1.5f)
                                 : i == S_LOSE  ? 1 - .4f * x
                                                : 1);
            ph += TAU * hz2 / RATE;
            float env = fminf(1, t * 180) * expf(-4.2f * x) * (1 - x);
            float y = (sinf(ph) + .32f * sinf(ph * 2.005f) + .15f * sinf(ph * 3)) * 2450 * env;
            if (i == S_ATTACK || i == S_HIT || i == S_ULT)
                y = .66f * y + noise * 1800 * env;
            if (i == S_AMBIENT)
                y *= .5f;
            if (i == S_STONE)
                y = env * (6800 * sinf(TAU * (49 * t + 1.6f * (1 - expf(-17 * t)))) + low * 8800 +
                           noise * 3600 * expf(-15 * t));
            if (i == S_COIN) {
                y = 0;
                for (int k = 0; k < 3; k++) {
                    float a = t - (float[]){0, .053f, .112f}[k];
                    if (a <= 0)
                        continue;
                    float h = (float[]){1340, 1810, 2220}[k];
                    y += 2350 * fminf(1, a * 510) * expf(-19 * a) *
                         (sinf(TAU * h * a) + .35f * sinf(TAU * h * 2.63f * a));
                }
            }
            if (i >= S_PICK0 && i <= S_PICK9) {
                /* Short pentatonic pluck and a delayed glass overtone; distinguish
                   a hero change from menu navigation without interrupting music. */
                float fundamental = hz[i];
                y = 3300 * env * (sinf(TAU * fundamental * t) + .56f * sinf(TAU * 2 * fundamental * t));
                float d = t - .065f;
                if (d > 0)
                    y += 2200 * fminf(1, d * 160) * expf(-13 * d) * (1 - x) *
                         sinf(TAU * fundamental * 1.5f * d);
            } else if (i == S_VS) {
                y = env * (6200 * sinf(TAU * (67 * t + 1.3f * (1 - expf(-16 * t)))) + 3800 * low) +
                    1600 * env * sinf(TAU * 440 * t);
            } else if (i == S_FRACTURE) {
                y=(7300*low+6600*sinf(TAU*(42*t+1.6f*(1-expf(-20*t)))))*env;
                float d=t-.065f;
                if(d>0)y+=expf(-6*d)*(1-x)*(2100*sinf(TAU*2105*d)+1550*sinf(TAU*2872*d)+1650*noise);
            }
            synth[i][j] = (short)y;
        }
    }
    /* Brief original event signatures underneath the announcer, rather than a
       beep before every line: a bronze strike, rally chimes, and a marching drum. */
    for (int j = 0; j < RATE / 2; j++) {
        float t = (float)j / RATE, on = fminf(1, t * 140), off = fminf(1, (.50f - t) * 65);
        float low = sinf(TAU * (61 * t + 1.35f * (1 - expf(-19 * t)))) * expf(-11 * t);
        float bronze = (sinf(TAU * 437 * t) + .38f * sinf(TAU * 1133 * t)) * expf(-10 * t);
        accent[0][j] = (short)(on * off * (4300 * low + 1670 * bronze));
        float ping = sinf(TAU * 846 * t) * expf(-18 * t);
        if (t > .105f)
            ping += .82f * sinf(TAU * 1128 * (t - .105f)) * expf(-13 * (t - .105f));
        accent[1][j] = (short)(on * off * 2300 * ping);
        accent[2][j] = (short)(on * off * (4600 * low + 880 * sinf(TAU * 144 * t) * expf(-10 * t)));
    }
    enabled = 1;
    return ok;
}
void audio_clear(void) {
    active = -1;
    pending = 0;
    speechPos = 0;
    for (int i = 0; i < 12; i++)
        sfx[i].id = -1;
}
static int priority(int id) {
    if (id == VO_VICTORY || id == VO_DEFEAT)
        return 5;
    if (id == VO_FIRST)
        return 4;
    if (id == VO_WELCOME || id == VO_MARCH || id == VO_COUNTDOWN || id == VO_RALLY)
        return 3;
    if (id >= VO_DOUBLE && id <= VO_ACED)
        return 3;
    if (id == VO_TOWERUP || id == VO_TOWERDOWN)
        return 2;
    return 1;
}
void audio_event(int id) {
    if (!enabled || mute || id < 0 || id >= SOUND_COUNT || !bank[id].pcm)
        return;
    if (id >= VO_COUNTDOWN) {
        if (priority(id) == 5) {
            active = -1;
            pending = 0;
        }
        /* If several opponents fall to one combo before the announcer can
           speak, say the highest earned multi-kill, not four obsolete lines. */
        if (id >= VO_DOUBLE && id <= VO_ENEMY_PENTA) {
            int base = id >= VO_ENEMY_DOUBLE ? VO_ENEMY_DOUBLE : VO_DOUBLE;
            for (int j = pending-1; j >= 0; --j)
                if (queue[j].id >= base && queue[j].id < id) {
                    memmove(queue+j, queue+j+1, (pending-j-1)*sizeof *queue);
                    pending--;
                }
        }
        /* Keep voices intelligible during team fights. Discard redundant tactical
           speech instead of playing the same stale line several times. */
        for (int i = 0; i < pending; i++)
            if (queue[i].id == id)
                return;
        if (pending == QUEUE) {
            if (priority(queue[pending - 1].id) >= priority(id))
                return;
            pending--;
        }
        int i = pending++;
        while (i && priority(queue[i - 1].id) < priority(id)) {
            queue[i] = queue[i - 1];
            i--;
        }
        queue[i] = (Cue){id, sampleTick};
        return;
    }
    int k = sfxSlot++ % 12;
    for (int i = 0; i < 12; i++)
        if (sfx[i].id < 0) {
            k = i;
            break;
        }
    sfx[k].id = id;
    sfx[k].pos = 0;
}
static void mix(short *dst, int frames, int menu) {
    float mg = menu && !mute ? .60f : 0;
    for (int j = 0; j < frames; j++, sampleTick++) {
        while (active < 0 && pending) {
            Cue cue = queue[0];
            memmove(queue, queue + 1, (--pending) * sizeof *queue);
            if (priority(cue.id) >= 4 || sampleTick - cue.tick < RATE * 4u) {
                active = cue.id;
                speechPos = 0;
            }
        }
        float v = 0, s = 0;
        if (active >= 0) {
            Clip c = bank[active];
            int sig = active == VO_FIRST ? 0 : active == VO_RALLY ? 1 : active == VO_MARCH ? 2 : -1;
            v = c.pcm[speechPos] * .86f;
            if (sig >= 0 && speechPos < RATE / 2)
                v += accent[sig][speechPos];
            if (++speechPos >= c.frames)
                active = -1;
        }
        float target = active >= 0 ? .47f : 1;
        duck += (target - duck) * (target < duck ? .0022f : .00015f);
        musicGain += (mg - musicGain) * (mg > musicGain ? .00010f : .00029f);
        for (int i = 0; i < 12; i++)
            if (sfx[i].id >= 0) {
                Clip c = bank[sfx[i].id];
                s += c.pcm[sfx[i].pos++];
                if (sfx[i].pos >= c.frames)
                    sfx[i].id = -1;
            }
        Clip m = bank[MUSIC];
        float l = 0, r = 0;
        if (m.pcm && (menu || musicGain > .0004f)) {
            l = m.pcm[2 * musicPos] * musicGain * duck;
            r = m.pcm[2 * musicPos + 1] * musicGain * duck;
            if (++musicPos >= m.frames)
                musicPos = 0;
        }
        s = s * duck + v;
        dst[2 * j] = mute ? 0 : (short)fminf(32500, fmaxf(-32500, s + l));
        dst[2 * j + 1] = mute ? 0 : (short)fminf(32500, fmaxf(-32500, s + r));
    }
}
void audio_open(void) {
    WAVEFORMATEX f = {WAVE_FORMAT_PCM, 2, RATE, RATE * 4, 4, 16, 0};
    if (waveOutOpen(&out, WAVE_MAPPER, &f, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        out = NULL;
        return;
    }
    prepare();
    audio_clear();
    for (int k = 0; k < 4; k++) {
        heads[k].lpData = (LPSTR)buf[k];
        heads[k].dwBufferLength = sizeof buf[k];
        if (waveOutPrepareHeader(out, &heads[k], sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
            waveOutWrite(out, &heads[k], sizeof(WAVEHDR));
    }
}
void audio_pump(int menu) {
    if (!out)
        return;
    for (int k = 0; k < 4; k++)
        if (heads[k].dwFlags & WHDR_DONE) {
            mix(buf[k], CHUNK, menu);
            waveOutWrite(out, &heads[k], sizeof(WAVEHDR));
        }
}
void audio_mute(int m) {
    mute = m;
    if (m)
        audio_clear();
}
void audio_close(void) {
    if (!out)
        return;
    waveOutReset(out);
    for (int k = 0; k < 4; k++)
        if (heads[k].dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(out, &heads[k], sizeof(WAVEHDR));
    waveOutClose(out);
    out = NULL;
    enabled = 0;
    audio_clear();
}
/* Embedded-asset/mixer regression, no audio endpoint or speakers required. */
int audio_check(void) {
    if (!prepare())
        return 0;
    audio_clear();
    if (bank[MUSIC].frames < RATE * 35)
        return 0;
    for (int i = VO_COUNTDOWN; i < SOUND_COUNT; i++) {
        if (bank[i].frames < RATE / 2)
            return 0;
        int loud = 0;
        for (int n = 0; n < bank[i].frames; n++)
            loud += abs(bank[i].pcm[n]) > 900;
        if (loud < RATE / 12)
            return 0;
    }
    musicPos = bank[MUSIC].frames - 10;
    mix(buf[0], CHUNK, 1);
    if (musicPos != CHUNK - 10)
        return 0;
    audio_event(VO_RALLY);
    audio_event(VO_FIRST);
    mix(buf[0], CHUNK, 0);
    if (active != VO_FIRST || pending != 1)
        return 0;
    audio_event(VO_VICTORY);
    mix(buf[0], CHUNK, 0);
    if (active != VO_VICTORY || pending)
        return 0;
    audio_mute(1);
    mix(buf[0], CHUNK, 1);
    for (int i = 0; i < CHUNK * 2; i++)
        if (buf[0][i])
            return 0;
    audio_mute(0);
    enabled = 0;
    return 1;
}

#ifdef RIFT_CAPTURE
int audio_capture_open(void) {
    audio_clear();
    return prepare();
}
void audio_capture_block(short *p, int count, int menu) {
    mix(p, count, menu);
}
#endif
