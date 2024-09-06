#ifndef __GAME_SCENES_H__
#define __GAME_SCENES_H__

#include <inttypes.h>
#include "engine_main.h"
#include "TE_Image.h"
#include "game.h"

#define SCENE_1_PULLING_THE_CART 1
#define SCENE_2_ARRIVING_AT_HOME 2
#define SCENE_3_CHASING_THE_LOOT 3

#define FADEIN_FLAG 1
#define FADEOUT_RIGHT_TO_LEFT 0
#define FADEIN_LEFT_TO_RIGHT 1
#define FADEOUT_LEFT_TO_RIGHT 2
#define FADEIN_RIGHT_TO_LEFT 3

#define ITEM_SLOT_LEFT_HAND 1
#define ITEM_SLOT_RIGHT_HAND 2


typedef struct Condition
{
    uint8_t type;
    union {
        struct RectCondition {
            uint8_t npcIds[4];
            int16_t x;
            int16_t y;
            int16_t width;
            int16_t height;
        } npcsInRect;
        struct WaitCondition {
            float duration;
        } wait;
    };
} Condition;

#define CONDITION_TYPE_PLAYER_IN_RECT 1
#define CONDITION_TYPE_PRESS_NEXT 2
#define CONDITION_TYPE_NPCS_IN_RECT 3
#define CONDITION_TYPE_WAIT 4


typedef struct ScriptedAction
{
    int16_t startPlotIndex;
    int16_t endPlotIndex;
    uint8_t actionType;
    float actionStartTime;
    union {
        struct SpeechBubbleData {
            const char *text;
            uint8_t speaker:5;
            uint8_t isRelative:1;
            int16_t speechBubbleX;
            int16_t speechBubbleY;
            uint8_t speechBubbleWidth;
            uint8_t speechBubbleHeight;
            int8_t arrowXOffset;
            int8_t arrowYOffset;
        } speechBubble;
        struct PlayerControlsData {
            uint8_t enabled;
        };
        struct ProceedPlotConditionData {
            uint8_t setPlotIndex;
            Condition condition;
        };
        struct SetNPCTargetData {
            uint8_t id;
            int16_t x;
            int16_t y;
        } npcTarget;
        
        struct SetPlayerTargetData {
            int16_t targetX;
            int16_t targetY;
            uint8_t setX;
            uint8_t setY;
        } playerTarget;

        struct SetFlagsData {
            uint32_t setFlags;
            uint32_t mask;
        };
        struct SetNPCHealthData {
            uint8_t id;
            float health;
        } npcHealth;
        struct SceneFadeOutData {
            uint8_t type;
            uint8_t nextPlotIndex;
            float duration;
            float beginDelay;
            float finishDelay;
        } sceneFadeOut;
        struct TitleScreenData {
            const char *titleText;
            const char *subText;
            uint8_t nextPlotIndex;
            uint8_t fillBlackBackground;
        } titleScreen;
        struct LoadSceneData {
            uint8_t sceneId;
        } loadScene;
        struct ClearScreenData {
            uint32_t color;
            uint8_t z;
        } clearScreen;
        struct NPCSpawnData {
            uint8_t id;
            uint8_t characterType;
            int16_t x;
            int16_t y;
            int16_t targetX;
            int16_t targetY;
        } npcSpawn;
        struct SetItemData {
            uint8_t charId;
            int8_t leftItemIndex;
            int8_t rightItemIndex;
        } setItem;
        struct AnimationPlaybackData {
            uint8_t animationId;
            uint8_t maxLoopCount;
            uint8_t z;
            int16_t x;
            int16_t y;
            uint32_t tintColor;
            float delay;
            float speed;
        } animationPlayback;
        struct SetEnemyCallback {
            uint8_t id;
            TookDamageCallbackData callback;
        } setEnemyCallback;
        struct CustomActionCallback {
            void(*callback)(RuntimeContext *ctx, TE_Img *screenData, struct ScriptedAction *callbackData);
            union {
                void *dataPointer;
                Character *characterPointer;
                Enemy *enemyPointer;
            };
            uint8_t flag;
            int16_t x, y;
        } customCallback;
    };
} ScriptedAction;

#define MAX_SCRIPTED_ACTIONS 128

typedef struct ScriptedActions
{
    ScriptedAction actions[MAX_SCRIPTED_ACTIONS];
    uint8_t currentPlotIndex;
    uint8_t startedTimerPlotIndex;
    uint32_t flags;
    float plotIndexStartTime;
} ScriptedActions;


void Scene_init(uint8_t sceneId);
void Scene_setStep(uint8_t step);
uint8_t Scene_getStep();
uint8_t Scene_getMaxStep();
void Scene_update(RuntimeContext *ctx, TE_Img *screen);
void ScriptedAction_init();
ScriptedAction* ScriptedAction_addSpeechBubble(uint8_t stepStart, uint8_t stepStop, const char *text, uint8_t speaker, int16_t speechBubbleX, int16_t speechBubbleY, uint8_t speechBubbleWidth, uint8_t speechBubbleHeight, int8_t arrowXOffset, int8_t arrowYOffset);

void ScriptedAction_addPlayerControlsEnabled(uint8_t stepStart, uint8_t stepStop, uint8_t enabled);
void ScriptedAction_addSetPlayerTarget(uint8_t stepStart, uint8_t stepStop, int16_t x, int16_t y, uint8_t setX, uint8_t setY);
void ScriptedAction_addSetPlayerPosition(uint8_t stepStart, uint8_t stepStop, int16_t x, int16_t y, uint8_t setX, uint8_t setY);
void ScriptedAction_addSetNPCTarget(uint8_t stepStart, uint8_t stepStop, uint8_t id, int16_t x, int16_t y);
void ScriptedAction_addProceedPlotCondition(uint8_t stepStart, uint8_t stepStop, uint8_t setPlotIndex, Condition condition);
void ScriptedAction_addSetFlags(uint8_t stepStart, uint8_t stepStop, uint32_t setFlags, uint32_t mask);
void ScriptedAction_addSetNPCHealth(uint8_t stepStart, uint8_t stepStop, uint8_t id, float health);
void ScriptedAction_addSetItem(uint8_t stepStart, uint8_t stepStop, uint8_t charId, int8_t leftItemIndex, int8_t rightItemIndex);
void ScriptedAction_addSceneFadeOut(uint8_t stepStart, uint8_t stepStop, uint8_t type, uint8_t nextPlotIndex, float duration, float beginDelay, float finishDelay);
void ScriptedAction_addTitleScreen(uint8_t stepStart, uint8_t stepStop, const char *text, const char *subtitle, uint8_t fillBlackBackground, uint8_t nextPlotIndex);
void ScriptedAction_addLoadScene(uint8_t stepStart, uint8_t stepStop, uint8_t sceneId);
ScriptedAction* ScriptedAction_addCustomCallback(uint8_t stepStart, uint8_t stepStop, void(*callback)(RuntimeContext *ctx, TE_Img *screenData, ScriptedAction *callbackData));
void ScriptedAction_addClearScreen(uint8_t stepStart, uint8_t stepStop, uint32_t color, uint8_t z);
void ScriptedAction_addNPCSpawn(uint8_t stepStart, uint8_t stepStop, uint8_t npcId, uint8_t characterType, 
    int16_t x, int16_t y, int16_t targetX, int16_t targetY);
void ScriptedAction_addSetEnemyCallback(uint8_t stepStart, uint8_t stepStop, uint8_t id, TookDamageCallbackData callback);
void ScriptedAction_addAnimationPlayback(uint8_t stepStart, uint8_t stepStop, uint8_t animationId, int16_t x, int16_t y, uint8_t z,
    float delay, float speed, uint8_t loop, uint32_t tintColor);
void ScriptedAction_addJumpStep(uint8_t stepStart, uint8_t stepStop, uint8_t stepTo);

void ScriptedAction_update(RuntimeContext *ctx, TE_Img *screenData);

uint8_t Scene_getCurrentSceneId();

void DrawSpeechBubble(TE_Img *screenData, int16_t x, int16_t y, int16_t width, int16_t height, int16_t arrowX, int16_t arrowY, const char *text);
void DrawNextButtonAction(RuntimeContext *ctx, TE_Img *screenData);

#endif