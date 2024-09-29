#include <memory.h>
#include <stdlib.h>

#include "atlas.h"
#include "game_scenes.h"
#include "game_renderobjects.h"
#include "game_assets.h"

#include "TE_math.h"

static uint8_t *_renderObjectMemory = NULL;
static uint32_t _renderObjectMemoryMaxSize = 0;
static uint32_t _renderObjectMemoryPointer = 0;
static RenderPrefab *_renderPrefab = NULL;
static RenderPrefab *_firstRenderPrefab = NULL;

void *RenderObject_malloc(uint32_t size)
{
    if (_renderObjectMemoryPointer + size > _renderObjectMemoryMaxSize)
    {
        LOG("RenderObject_malloc: Out of memory");
        TE_Panic("Out of memory");
        return NULL;
    }

    if (size == 0) return NULL;

    // LOG("RenderObject_malloc: %d", size);

    void *ptr = _renderObjectMemory + _renderObjectMemoryPointer;
    _renderObjectMemoryPointer += size;
    _renderObjectMemoryPointer = ALIGN_VALUE4(_renderObjectMemoryPointer);
    return ptr;
}

RenderPrefab* RenderPrefab_getFirstById(uint16_t id)
{
    RenderPrefab *prefab = _firstRenderPrefab;
    while (prefab != NULL)
    {
        if (prefab->id == id)
        {
            return prefab;
        }
        prefab = prefab->next;
    }
    return NULL;
}

RenderPrefab* RenderPrefab_create(RenderObjectCounts counts)
{
    RenderPrefab *prefab = (RenderPrefab*) RenderObject_malloc(sizeof(RenderPrefab));
    if (prefab == NULL)
    {
        TE_DebugRGB(1,1,1);
        LOG("Failed to allocate memory for RenderPrefab");
        return NULL;
    }

    prefab->renderObjectSpritesMaxCount = counts.spriteMaxCount;
    prefab->renderObjectAtlasBlitsMaxCount = counts.atlasBlitMaxCount;
    prefab->renderObjectAtlasBlitSkewsXMaxCount = counts.atlasBlitSkewXMaxCount;
    prefab->renderObjectAtlasBlitSkewsYMaxCount = counts.atlasBlitSkewYMaxCount;
    prefab->renderObjectPrefabInstancesMaxCount = counts.prefabInstanceMaxCount;
    prefab->renderObjectFunctionCallsMaxCount = counts.functionCallMaxCount;
    
    prefab->renderObjectSprites = (RenderObjectSprite*) RenderObject_malloc(sizeof(RenderObjectSprite) * counts.spriteMaxCount);
    prefab->renderObjectAtlasBlits = (RenderObjectAtlasBlit*) RenderObject_malloc(sizeof(RenderObjectAtlasBlit) * counts.atlasBlitMaxCount);
    prefab->renderObjectAtlasBlitSkewsX = (RenderObjectAtlasBlitSkew*) RenderObject_malloc(sizeof(RenderObjectAtlasBlitSkew) * counts.atlasBlitSkewXMaxCount);
    prefab->renderObjectAtlasBlitSkewsY = (RenderObjectAtlasBlitSkew*) RenderObject_malloc(sizeof(RenderObjectAtlasBlitSkew) * counts.atlasBlitSkewYMaxCount);
    prefab->renderObjectPrefabInstances = (RenderObjectPrefabInstance*) RenderObject_malloc(sizeof(RenderObjectPrefabInstance) * counts.prefabInstanceMaxCount);
    prefab->renderObjectFunctionCalls = (RenderObjectFunctionCall*) RenderObject_malloc(sizeof(RenderObjectFunctionCall) * counts.functionCallMaxCount);
    
    prefab->next = _firstRenderPrefab;
    _firstRenderPrefab = prefab;

    return prefab;
}

void RenderPrefab_clear(RenderPrefab *prefab)
{
    prefab->renderObjectSpritesCount = 0;
    prefab->renderObjectAtlasBlitsCount = 0;
    prefab->renderObjectAtlasBlitSkewsXCount = 0;
    prefab->renderObjectAtlasBlitSkewsYCount = 0;
    prefab->renderObjectPrefabInstancesCount = 0;
}

uint8_t RenderPrefab_getColorAt(RenderPrefab *prefab, int16_t x, int16_t y, uint32_t *color, uint8_t *z)
{
    uint32_t pixels[1] = {0};
    TE_Img tmp = (TE_Img){.data = pixels, .p2width = 0, .p2height = 0};
    RenderPrefab_update(prefab, NULL, &tmp, -x, -y, 0);
    if (color) *color = pixels[0] & 0x00FFFFFF;
    if (z) *z = pixels[0] >> 24;
    return pixels[0] != 0;
}

uint16_t RenderPrefab_addSprite(RenderPrefab *prefab, RenderObjectSprite sprite)
{
    if (prefab->renderObjectSpritesCount >= prefab->renderObjectSpritesMaxCount)
    {
        LOG("RenderPrefab_addSprite: Max sprite count reached");
        return 0;
    }

    prefab->renderObjectSprites[prefab->renderObjectSpritesCount] = sprite;
    return prefab->renderObjectSpritesCount++;
}

uint16_t RenderPrefab_addAtlasBlit(RenderPrefab *prefab, RenderObjectAtlasBlit atlasBlit)
{
    if (prefab->renderObjectAtlasBlitsCount >= prefab->renderObjectAtlasBlitsMaxCount)
    {
        LOG("RenderPrefab_addAtlasBlit: Max atlas blit count reached");
        return 0;
    }

    prefab->renderObjectAtlasBlits[prefab->renderObjectAtlasBlitsCount] = atlasBlit;
    return prefab->renderObjectAtlasBlitsCount++;
}

uint16_t RenderPrefab_addAtlasBlitSkewX(RenderPrefab *prefab, RenderObjectAtlasBlitSkew atlasBlit)
{
    if (prefab->renderObjectAtlasBlitSkewsXCount >= prefab->renderObjectAtlasBlitSkewsXMaxCount)
    {
        LOG("RenderPrefab_addAtlasBlitSkewX: Max atlas blit skew X count reached");
        return 0;
    }

    prefab->renderObjectAtlasBlitSkewsX[prefab->renderObjectAtlasBlitSkewsXCount] = atlasBlit;
    return prefab->renderObjectAtlasBlitSkewsXCount++;
}

uint16_t RenderPrefab_addAtlasBlitSkewY(RenderPrefab *prefab, RenderObjectAtlasBlitSkew atlasBlit)
{
    if (prefab->renderObjectAtlasBlitSkewsYCount >= prefab->renderObjectAtlasBlitSkewsYMaxCount)
    {
        LOG("RenderPrefab_addAtlasBlitSkewY: Max atlas blit skew Y count reached");
        return 0;
    }

    prefab->renderObjectAtlasBlitSkewsY[prefab->renderObjectAtlasBlitSkewsYCount] = atlasBlit;
    return prefab->renderObjectAtlasBlitSkewsYCount++;
}

uint16_t RenderPrefab_addPrefabInstance(RenderPrefab *prefab, RenderObjectPrefabInstance instance)
{
    if (prefab->renderObjectPrefabInstancesCount >= prefab->renderObjectPrefabInstancesMaxCount)
    {
        LOG("RenderPrefab_addPrefabInstance: Max prefab instance count reached");
        return 0;
    }

    prefab->renderObjectPrefabInstances[prefab->renderObjectPrefabInstancesCount] = instance;
    return prefab->renderObjectPrefabInstancesCount++;
}

uint16_t RenderPrefab_addFunctionCall(RenderPrefab *prefab, RenderObjectFunctionCall functionCall)
{
    if (prefab->renderObjectFunctionCallsCount >= prefab->renderObjectFunctionCallsMaxCount)
    {
        LOG("RenderPrefab_addFunctionCall: Max function call count reached");
        return 0;
    }

    prefab->renderObjectFunctionCalls[prefab->renderObjectFunctionCallsCount] = functionCall;
    return prefab->renderObjectFunctionCallsCount++;
}

void RenderPrefab_update(RenderPrefab *prefab, RuntimeContext *ctx, TE_Img *screen, int16_t offsetX, int16_t offsetY, int8_t offsetZ)
{
    for (int i = 0; i < prefab->renderObjectSpritesCount; i++)
    {
        RenderObjectSprite *sprite = &prefab->renderObjectSprites[i];
        TE_Sprite spriteData = GameAssets_getSprite(sprite->spriteIndex);
        BlitEx blitEx = sprite->blitEx;
        blitEx.state.zValue += offsetZ;
        TE_Img_blitSprite(screen, spriteData, sprite->x + offsetX, sprite->y + offsetY, blitEx);
    }

    TE_Img *atlas = GameAssets_getAtlasImg();

    for (int i = 0; i < prefab->renderObjectAtlasBlitsCount; i++)
    {
        RenderObjectAtlasBlit *atlasBlit = &prefab->renderObjectAtlasBlits[i];
        BlitEx blitEx = atlasBlit->blitEx;
        blitEx.state.zValue += offsetZ;
        TE_Img_blitEx(screen, atlas, atlasBlit->x + offsetX, atlasBlit->y + offsetY, 
            atlasBlit->srcX, atlasBlit->srcY, atlasBlit->width, atlasBlit->height, blitEx);
    }

    for (int i = 0; i < prefab->renderObjectAtlasBlitSkewsXCount; i++)
    {
        RenderObjectAtlasBlitSkew *atlasBlitSkew = &prefab->renderObjectAtlasBlitSkewsX[i];
        BlitEx blitEx = atlasBlitSkew->blitEx;
        blitEx.state.zValue += offsetZ;
        int8_t dir = atlasBlitSkew->y1 < atlasBlitSkew->y2 ? 1 : -1;
        int16_t shiftX = 0;
        int16_t diffX = atlasBlitSkew->x2 - atlasBlitSkew->x1;
        int16_t diffY = atlasBlitSkew->y2 - atlasBlitSkew->y1;
        // int16_t stepX = diffX / diffY;
        uint8_t count = absi(diffY);

        for (int y = atlasBlitSkew->y1, n = 0, v = 0; n < count; y+=dir, v++, n++)
        {
            // LOG("y: %d, v: %d", y, v);
            // calc x offset based on progression from y1 to y2
            shiftX = (diffX * v) / diffY;
            if (v >= atlasBlitSkew->height)
            {
                v -= atlasBlitSkew->height;
            }
            // TE_Debug_drawPixel(atlasBlitSkew->x1 + offsetX, y + offsetY, 0xFFFFff00);
            // TE_Debug_drawLine(atlasBlitSkew->x1 + offsetX + shiftX, y + offsetY, atlasBlitSkew->x1 + offsetX, y + offsetY, 0xFFFF0000);
            TE_Img_blitEx(screen, atlas, atlasBlitSkew->x1 + offsetX + shiftX, y + offsetY, 
                atlasBlitSkew->srcX, atlasBlitSkew->srcY + v, atlasBlitSkew->width, 1, blitEx);
        }
        // TE_Img_blitEx(screen, atlas, atlasBlitSkew->x1 + offsetX, atlasBlitSkew->y1 + offsetY, 
        //     atlasBlitSkew->srcX, atlasBlitSkew->srcY, atlasBlitSkew->width, atlasBlitSkew->height, blitEx);
        //todo
    }

    for (int i = 0; i < prefab->renderObjectAtlasBlitSkewsYCount; i++)
    {
        RenderObjectAtlasBlitSkew *atlasBlitSkew = &prefab->renderObjectAtlasBlitSkewsY[i];
        BlitEx blitEx = atlasBlitSkew->blitEx;
        blitEx.state.zValue += offsetZ;
        //todo
    }

    for (int i = 0; i < prefab->renderObjectFunctionCallsCount; i++)
    {
        RenderObjectFunctionCall *functionCall = &prefab->renderObjectFunctionCalls[i];
        functionCall->function(ctx, screen, functionCall->data, functionCall->x + offsetX, functionCall->y + offsetY, functionCall->z + offsetZ);
    }

    for (int i = 0; i < prefab->renderObjectPrefabInstancesCount; i++)
    {
        RenderObjectPrefabInstance *prefabInstance = &prefab->renderObjectPrefabInstances[i];
        RenderPrefab *subPrefab = prefabInstance->prefab;
        RenderPrefab_update(subPrefab, ctx, screen, offsetX + prefabInstance->offsetX, offsetY + prefabInstance->offsetY, offsetZ + prefabInstance->offsetZ);
    }
}

void RenderObject_init(uint32_t memorySize)
{
    _firstRenderPrefab = NULL;
    _renderObjectMemory = (uint8_t*) Scene_malloc(memorySize);
    _renderObjectMemoryPointer = 0;
    _renderPrefab = NULL;
    if (_renderObjectMemory == NULL && memorySize > 0)
    {
        _renderObjectMemoryMaxSize = 0;
        LOG("Failed to allocate memory (%d) for RenderObject", memorySize);
        return;
    }

    _renderObjectMemoryMaxSize = memorySize;
}

void RenderObject_setMain(RenderPrefab *prefab)
{
    _renderPrefab = prefab;
}

void RenderObject_clear()
{
    _renderPrefab = NULL;
    _firstRenderPrefab = NULL;
    _renderObjectMemoryPointer = 0;
    memset(_renderObjectMemory, 0, _renderObjectMemoryMaxSize);
}

void RenderObject_update(RuntimeContext *ctx, TE_Img *screen)
{
    if (_renderPrefab == NULL)
    {
        return;
    }
    RenderPrefab_update(_renderPrefab, ctx, screen, 0, 0, 0);
}

