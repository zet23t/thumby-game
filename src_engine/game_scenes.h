#ifndef __GAME_SCENES_H__
#define __GAME_SCENES_H__

#include <inttypes.h>
#include "engine_main.h"
#include "TE_Image.h"

void Scene_init(uint8_t sceneId);
void Scene_update(RuntimeContext *ctx, TE_Img *screen);

#endif