#ifndef __GAME_RENDEROBJECTS_H__
#define __GAME_RENDEROBJECTS_H__

#include "TE_Image.h"

typedef struct RenderObjectSprite
{
    int16_t x;
    int16_t y;
    BlitEx blitEx;
    uint8_t spriteIndex;
} RenderObjectSprite;

typedef struct RenderObjectAtlasBlit
{
    int16_t x;
    int16_t y;
    uint16_t srcX;
    uint16_t srcY;
    int16_t width;
    int16_t height;
    BlitEx blitEx;
} RenderObjectAtlasBlit;

typedef struct RenderObjectAtlasBlitSkew
{
    int16_t x1;
    int16_t x2;
    int16_t y1;
    int16_t y2;
    uint16_t srcX;
    uint16_t srcY;
    int16_t width;
    int16_t height;
    BlitEx blitEx;
} RenderObjectAtlasBlitSkew;

typedef struct RenderObjectFunctionCall
{
    void (*function)(RuntimeContext *ctx, TE_Img* screen, void *data, int16_t x, int16_t y, int8_t z);
    void *data;
    int16_t x;
    int16_t y;
    int8_t z;
} RenderObjectFunctionCall;

typedef struct RenderPrefab RenderPrefab;

typedef struct RenderObjectPrefabInstance
{
    RenderPrefab *prefab;
    int16_t offsetX, offsetY;
    int8_t offsetZ;
} RenderObjectPrefabInstance;

typedef struct RenderObjectCounts
{
    uint8_t spriteMaxCount;
    uint8_t atlasBlitMaxCount;
    uint8_t atlasBlitSkewXMaxCount;
    uint8_t atlasBlitSkewYMaxCount;
    uint8_t prefabInstanceMaxCount;
    uint8_t functionCallMaxCount;
} RenderObjectCounts;

#define OBJECT_LIST(type, name) \
    uint8_t name##Count;        \
    uint8_t name##MaxCount;     \
    type *name;

typedef struct RenderPrefab {
    uint16_t id;
    RenderPrefab *next;
    OBJECT_LIST(RenderObjectSprite, renderObjectSprites)
    OBJECT_LIST(RenderObjectAtlasBlit, renderObjectAtlasBlits)
    OBJECT_LIST(RenderObjectAtlasBlitSkew, renderObjectAtlasBlitSkewsX)
    OBJECT_LIST(RenderObjectAtlasBlitSkew, renderObjectAtlasBlitSkewsY)
    OBJECT_LIST(RenderObjectPrefabInstance, renderObjectPrefabInstances)
    OBJECT_LIST(RenderObjectFunctionCall, renderObjectFunctionCalls)
} RenderPrefab;

void *RenderObject_malloc(uint32_t size);
RenderPrefab* RenderPrefab_getFirstById(uint16_t id);
RenderPrefab* RenderPrefab_create(RenderObjectCounts counts);
void RenderPrefab_update(RenderPrefab *prefab, RuntimeContext *ctx, TE_Img *screen, int16_t offsetX, int16_t offsetY, int8_t offsetZ);
uint16_t RenderPrefab_addSprite(RenderPrefab *prefab, RenderObjectSprite sprite);
uint16_t RenderPrefab_addAtlasBlit(RenderPrefab *prefab, RenderObjectAtlasBlit atlasBlit);
uint16_t RenderPrefab_addAtlasBlitSkewX(RenderPrefab *prefab, RenderObjectAtlasBlitSkew atlasBlit);
uint16_t RenderPrefab_addAtlasBlitSkewY(RenderPrefab *prefab, RenderObjectAtlasBlitSkew atlasBlit);
uint16_t RenderPrefab_addPrefabInstance(RenderPrefab *prefab, RenderObjectPrefabInstance instance);
uint16_t RenderPrefab_addFunctionCall(RenderPrefab *prefab, RenderObjectFunctionCall functionCall);
uint8_t RenderPrefab_getColorAt(RenderPrefab *prefab, int16_t x, int16_t y, uint32_t *color, uint8_t *z);
void RenderPrefab_clear(RenderPrefab *prefab);
void RenderObject_init(uint32_t memorySize);
void RenderObject_update(RuntimeContext *ctx, TE_Img *screen);
void RenderObject_setMain(RenderPrefab *prefab);
void RenderObject_clear();
#endif