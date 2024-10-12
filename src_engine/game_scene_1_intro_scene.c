#include "game_scenes.h"
#include "game_enemies.h"
#include "game_player.h"
#include "game_assets.h"
#include "game_environment.h"
#include "game_particlesystem.h"
#include "game_renderobjects.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>
#include <memory.h>

static int16_t _moveDistanceXRight = 0;
static int16_t _moveDistanceXLeft = 0;
static int16_t _movingStraightDistance = 0;
static int16_t _prevPlayerX = 0;
static int8_t _moveDir = 0;

void Scene_1_update(RuntimeContext *ctx, TE_Img *screenData)
{
    // Update scene 1
    int16_t cartX = player.x - 20;
    
    // draw path
    const uint16_t pathQHeight = 16;
    for (uint16_t x=0;x<128;x+=2)
    {
        uint16_t srcX = x % 16;
        int16_t y = 50 + (int16_t)(sin((x) * 0.05f) * 5.0f + x * 0.1f);

        TE_Img_blitEx(screenData, &atlasImg, x, y, 71 + srcX, 16, 2, pathQHeight, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
        TE_Img_blitEx(screenData, &atlasImg, x, y+pathQHeight, 71 + srcX, 16 + 24, 2, pathQHeight, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
    }
    // TE_Img_drawPatch9(screenData, &atlasImg, -8, 58, 140, 28, 64,16, 12, 12, (BlitEx){
    //     .blendMode = TE_BLEND_ALPHAMASK,
    //     .state = {
    //         .zCompareMode = Z_COMPARE_LESS_EQUAL,
    //         .zValue = 0,
    //     }
    // });

    int16_t cartY = 60 + (int16_t)(sin((cartX) * 0.05f) * 5.0f + cartX * 0.1f);
    int16_t cartYAnchor = 60 + (int16_t)(sin((cartX+6) * 0.05f) * 5.0f + (cartX + 6) * 0.1f);

    player.y = 54 + (int16_t)(sin((player.x) * 0.05f) * 5.0f + player.x * 0.1f);
    player.aimY = 1;
    player.dirY = 1;
    player.dy = 1;
    playerCharacter.y = player.y;
    playerCharacter.dirY = 1;

    Cart_draw(screenData, cartX, cartY,1, ctx);

    TE_Img_line(screenData, cartX+8, cartYAnchor, cartX+20, player.y + 6, DB32Colors[3], (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 83,
    });
    TE_Img_line(screenData, cartX+8, cartYAnchor + 1, cartX+20, player.y + 7, DB32Colors[2], (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 83,
    });

    const char *text = NULL;

    int16_t diff = floorf(player.x + .5f) - _prevPlayerX;
    _prevPlayerX = floorf(player.x + .5f);
    // every 30 pixels show a new message. First 10 pixels show nothing.
    if (diff > 0)
    {
        if (_movingStraightDistance < 0)
        {
            _movingStraightDistance = 0;
        }
        if (_movingStraightDistance + diff > 3 && _movingStraightDistance <= 3)
        {
            _moveDir = 1;
            int16_t remainder = _moveDistanceXRight % 30;
            if (remainder < 10)
            {
                _moveDistanceXRight -= remainder;
            }
            else
            {
                _moveDistanceXRight += 30 - remainder;
            }
        }
        _movingStraightDistance += diff;
        _moveDistanceXRight += diff;
    }
    else if (diff < 0)
    {
            // LOG("Moving straight distance: %d", _movingStraightDistance);
        if (_movingStraightDistance > 0)
        {
            _movingStraightDistance = 0;
        }
        if (_movingStraightDistance + diff < -3 && _movingStraightDistance >= -3)
        {

            _moveDir = -1;
            int16_t remainder = _moveDistanceXLeft % 30;
            if (remainder < 10)
            {
                _moveDistanceXLeft -= remainder;
            }
            else
            {
                _moveDistanceXLeft += 30 - remainder;
            }
        }
        _movingStraightDistance += diff;
        _moveDistanceXLeft += -diff;
    }

    const char *textMessageRight[] = {
        "The crusades were awesome.",
        "So. Much. Loot!",
        "Finally I can retire.",
        "I can now get the pool I always wanted!",
        "I wonder how my castle is doing.",
        "Before I left, the roof was leaking.",
        "Now I can afford to fix it!",
        "Heck, I will build a new one!",
        "Sir Robin's Manor!",
        "The air here smells like home.",
        "Yes, this is the right direction.",
        NULL
    };
    const char *textMessageLeft[] = {
        "Is the direction right?",
        "Why do my feet carry me back?",
        "Ah, when I stole the princess' diadem ...",
        "The princess looked so beautiful!",
        "But I think her father is still mad at me.",
        "Shouldn't have kissed her on the way out...",
        "... while carrying all his stuff, hehehe.",
        "Maybe that was a bit too cocky...",
        "I am pretty he'll have my head if I return.",
        "No, I think I should go the other way.",
        NULL
    };

    const char **textMessages = _moveDir > 0 ? textMessageRight : textMessageLeft;
    int16_t moveDist = _moveDir > 0 ? _moveDistanceXRight : _moveDistanceXLeft;
    while (*textMessages)
    {
        if (moveDist < 10)
        {
            break;
        }
        if (moveDist >= 10 && moveDist <= 30)
        {
            text = *textMessages;
            break;
        }
        textMessages++;
        if (!*textMessages)
        {
            text = textMessages[-1];
        }
        moveDist -= 30;
    }

    // if (player.x > 108)
    // {
    //     text = NULL;
    // }
    // else if (player.x > 80)
    // {
    //     text = "I can now get the pool I always wanted!";
    // }
    // else if (player.x > 60)
    // {
    //     text = "Finally I can retire.";
    // }
    // else if (player.x > 40)
    // {
    //     text = "So. Much. Loot!";
    // }
    // else if (player.x > 20)
    // {
    //     text = "The crusades were awesome.";
    // }

    if (text)
    {
        DrawSpeechBubble(screenData, 10,20, 108, 30, cartX + 20, player.y - 5, text);
    }

    if (!ctx->inputRight && !Menu_isActive())
    {
        DrawTextBlock(screenData, 70, 105, 48, 16, "Press >");
    }

    if (player.x < -8)
    {
        player.x = -8;
        playerCharacter.x = player.x;
    }
    if (cartX > 130)
    {
        Scene_init(2);
    }
}

void Scene_1_init()
{
    // Enemies_spawn(1, 28, 42);
    // Enemies_spawn(1, 44, 28);

    _moveDistanceXLeft = 0;
    _moveDistanceXRight = 0;
    _movingStraightDistance = 0;
    _prevPlayerX = player.x = -8;
    _moveDir = 0;
    playerCharacter.x = player.x;

    Environment_addTreeGroup(24, 30, 26, 5, 25);
    Environment_addTreeGroup(114, 30, 122, 5, 25);
    Environment_addTreeGroup(114, 125, 1252, 5, 25);
    Environment_addTreeGroup(24, 124, 99, 5, 20);
    // Environment_addTreeGroup(64, 84, 199, 3, 20);
}