#include "game.h"
#include "game_audio.h"
#include "game_assets.h"
#include "hxcmod.h"
#include <memory.h>

SoundMixerConfig volatile SoundConfig = {
    .VolumeMusic = 0.0f
};

SFXChannelStatus sfxChannels[SFX_CHANNELS_COUNT];

modcontext _modctx;
int _modctx_initialized = 0;

void GameAudio_update(AudioContext *audioContext)
{
    char *buffer = audioContext->outBuffer;
    unsigned int frames = audioContext->frames; 
    int sampleRate = audioContext->sampleRate;
    int sampleSize = audioContext->sampleSize;

    if (!_modctx_initialized)
    {
        _modctx_initialized = 1;
        if (!hxcmod_init(&_modctx)) {
            LOG("Failed to initialize hxcmod");
            _modctx_initialized = -1;
        }
        hxcmod_setcfg(&_modctx, sampleRate, 0, 1);
        // if (!hxcmod_load(&_modctx, (unsigned char*)moddata_nitabrowski, moddata_nitabrowski_size))
        // {
        //     LOG("Failed to load mod data");
        //     _modctx_initialized = -1;
        // }
    }

    SFXInstruction *sfxInstruction = &audioContext->inSfxInstructions[0];
    switch (sfxInstruction->type)
    {
        case SFXINSTRUCTION_TYPE_PLAY:
            const char *mod_data; 
            int mod_data_size;
            if (GameAssets_getMusic(sfxInstruction->id, &mod_data, &mod_data_size))
            {
                if (!hxcmod_load(&_modctx, (unsigned char*)mod_data, mod_data_size))
                {
                    LOG("Failed to load mod data");
                    sfxChannels[0].currentVolume = 0;
                    sfxChannels[0].flagIsPlaying = 0;
                }
                else
                {
                    sfxChannels[0].currentVolume = sfxInstruction->updateMask & SFXINSTRUCTION_UPDATE_MASK_VOLUME ? sfxInstruction->volume : 255;
                    sfxChannels[0].currentPitch = sfxInstruction->updateMask & SFXINSTRUCTION_UPDATE_MASK_PITCH ? sfxInstruction->pitch : 100;
                    sfxChannels[0].flagIsPlaying = 1;
                }
                LOG("Playing music %d at %d", sfxInstruction->id, sfxChannels[0].currentVolume);
            }
            break;
        case SFXINSTRUCTION_TYPE_PAUSE:
            LOG("Pausing music %d", sfxChannels[0].id);
            sfxChannels[0].flagIsPlaying = 0;
            break;
        case SFXINSTRUCTION_TYPE_RESUME:
            LOG("Resuming music %d", sfxChannels[0].id);
            sfxChannels[0].flagIsPlaying = 1;
            break;
        case SFXINSTRUCTION_TYPE_UPDATE:
            LOG("Updating music %d", sfxChannels[0].id);
            sfxChannels[0].currentVolume = sfxInstruction->updateMask & SFXINSTRUCTION_UPDATE_MASK_VOLUME ? sfxInstruction->volume : sfxChannels[0].currentVolume;
            sfxChannels[0].currentPitch = sfxInstruction->updateMask & SFXINSTRUCTION_UPDATE_MASK_PITCH ? sfxInstruction->pitch : sfxChannels[0].currentPitch;
            break;
    }

    memset(buffer, 0, frames * sampleSize / 8);
    if (sfxChannels[0].flagIsPlaying)
    {
        hxcmod_fillbuffer(&_modctx, (msample*)buffer, frames, 0);
        if (sampleSize == 16)
        {
            short *buf = (short*) buffer;
            uint8_t volume = sfxChannels[0].currentVolume;
            for (unsigned int i = 0; i < frames; i+=1)
            {
                buf[i] = (short)(buf[i] * volume >> 8);
            }
        }
    }

    audioContext->outSfxChannelStatus[0] = sfxChannels[0];
}
// static msample wave_buffer[CONFIG_WAVE_BUFFER_SIZE];

