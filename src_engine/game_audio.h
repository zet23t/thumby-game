#ifndef __GAME_AUDIO_H__
#define __GAME_AUDIO_H__

#include "hxcmod.h"
#include <inttypes.h>
#include "engine_main.h"

typedef struct SoundMixerConfig
{
    float VolumeMusic;
} SoundMixerConfig;

extern SoundMixerConfig volatile SoundConfig;
void GameAudio_update(AudioContext *audioContext);

#endif