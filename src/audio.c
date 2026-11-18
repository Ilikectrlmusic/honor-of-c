#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "audio.h"
#define RATE 22050
#define CHUNK 512
#define TAU 6.28318530718f
static HWAVEOUT out;
static WAVEHDR heads[4];
static short buf[4][CHUNK];
static float clips[18][RATE];
static int sizes[18], mute, seq;
static unsigned tick;
static struct {
    int clip, p;
} voices[12];
void audio_open(void) {
    WAVEFORMATEX f = {WAVE_FORMAT_PCM, 1, RATE, RATE * 2, 2, 16, 0};
    if (waveOutOpen(&out, WAVE_MAPPER, &f, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        out = NULL;
        return;
    }
    const float freq[] = {250, 130, 470,     170,     590, 890, 700,     420,     590,
                          520, 240, 261.63f, 329.63f, 392, 440, 523.25f, 659.25f, 127};
    const float dur[] = {.12f, .13f, .29f, .57f, .42f, .24f, .42f, .35f, .12f,
                         .88f, .80f, .94f, .94f, .94f, .94f, .94f, .94f, .095f};
    uint32_t rand = 132;
    for (int i = 0; i < 18; i++) {
        sizes[i] = (int)(dur[i] * RATE);
        float ph = 0;
        for (int j = 0; j < sizes[i]; j++) {
            float t = (float)j / RATE, x = t / dur[i];
            rand = 1664525 * rand + 1013904223;
            float noise = (rand >> 8) / 8388608.f - 1;
            float hz = freq[i] * (i < 2     ? 1.5f - x
                                  : i == 2  ? 1 + x * .8f
                                  : i == 3  ? .75f + x * .7f
                                  : i == 9  ? (x < .33f   ? 1
                                               : x < .64f ? 1.25f
                                                          : 1.5f)
                                  : i == 10 ? 1 - .4f * x
                                            : 1);
            ph += TAU * hz / RATE;
            float env = fminf(1, t * 180) * expf(-4.2f * x) * (1 - x);
            float y = sinf(ph) + .32f * sinf(ph * 2.005f) + .15f * sinf(ph * 3);
            if (i == 0 || i == 1 || i == 3)
                y = .65f * y + noise * (1 - x) * .7f;
            clips[i][j] = y * env * (i >= 11 ? 1200.f : 2400.f);
        }
    }
    for (int k = 0; k < 4; k++) {
        heads[k].lpData = (LPSTR)buf[k];
        heads[k].dwBufferLength = sizeof buf[k];
        waveOutPrepareHeader(out, &heads[k], sizeof(WAVEHDR));
        waveOutWrite(out, &heads[k], sizeof(WAVEHDR));
    }
    for (int i = 0; i < 12; i++)
        voices[i].clip = -1;
}
void audio_event(int id) {
    if (!out || mute || id < 0 || id >= 18)
        return;
    int k = 0;
    for (int i = 0; i < 12; i++)
        if (voices[i].clip < 0) {
            k = i;
            break;
        }
    voices[k].clip = id;
    voices[k].p = 0;
}
void audio_pump(int menu) {
    if (!out)
        return;
    for (int k = 0; k < 4; k++)
        if (heads[k].dwFlags & WHDR_DONE) {
            if (menu && !mute && tick % 42 == 0) {
                static const int notes[] = {11, 13, 14, 12, 13, 15, 14, 12, 11, 13, 16, 15};
                audio_event(notes[seq++ % 12]);
            }
            tick++;
            for (int j = 0; j < CHUNK; j++) {
                float sum = 0;
                for (int i = 0; i < 12; i++)
                    if (voices[i].clip >= 0) {
                        int c = voices[i].clip;
                        sum += clips[c][voices[i].p++];
                        if (voices[i].p >= sizes[c])
                            voices[i].clip = -1;
                    }
                buf[k][j] = mute ? 0 : (short)fminf(28000, fmaxf(-28000, sum));
            }
            waveOutWrite(out, &heads[k], sizeof(WAVEHDR));
        }
}
void audio_mute(int m) {
    mute = m;
    if (m)
        for (int i = 0; i < 12; i++)
            voices[i].clip = -1;
}
void audio_close(void) {
    if (!out)
        return;
    waveOutReset(out);
    for (int k = 0; k < 4; k++)
        waveOutUnprepareHeader(out, &heads[k], sizeof(WAVEHDR));
    waveOutClose(out);
    out = NULL;
}
