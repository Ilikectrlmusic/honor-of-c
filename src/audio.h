#ifndef RIFT_AUDIO_H
#define RIFT_AUDIO_H
#ifdef RIFT_CAPTURE
int audio_capture_open(void);
void audio_capture_block(short *p, int count, int menu);
#endif
int audio_check(void);
void audio_clear(void);
void audio_open(void);
void audio_close(void);
void audio_event(int id);
void audio_pump(int menu);
void audio_mute(int mute);
#endif
