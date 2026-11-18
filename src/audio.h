#ifndef RIFT_AUDIO_H
#define RIFT_AUDIO_H
void audio_open(void);
void audio_close(void);
void audio_event(int id);
void audio_pump(int menu);
void audio_mute(int mute);
#endif
