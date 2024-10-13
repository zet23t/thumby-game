#include "game_scene_3.h"
#include "game_player.h"
#include "game_character.h"
#include "game_enemies.h"
#include "game_environment.h"
#include "game_enemies.h"
#include "game_assets.h"
#include "game_particlesystem.h"
#include "game.h"
#include "game_battle.h"
#include "game_battle_actions.h"

#include "TE_sdfmap.h"
#include "TE_math.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>

void Scene_played_through_init(uint8_t sceneId)
{
    player.x = 10;
    player.y = 60;
    playerCharacter.x = player.x;
    playerCharacter.y = player.y;
    playerCharacter.targetX = player.x;
    playerCharacter.targetY = player.y;
    uint8_t step = 0;
    ScriptedAction_addPlayerControlsEnabled(step, 0xff, 0);
    ScriptedAction_addSceneFadeOut(step, step, FADEIN_RIGHT_TO_LEFT, step + 1, 0.85f, 0.4f, 1.0f);
    ScriptedAction_addSetPlayerTarget(step, step+1, 64, 90, 1, 1);
    step++;
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition)
        { .type = CONDITION_TYPE_PLAYER_IN_RECT, 
            .npcsInRect = { .x = 62, .y = 80, .width = 10, .height = 20 } });
    step++;

    const char *outroTexts[] = {
        "I know this comes sudden ...",
        "... but you've reached the end.",
        "NO, Zet not done yet!",
        "He has big plans with me.",
        "(At least he keeps telling me.)",
        "But this is as far as he got.",
        "It's time to thank you for playing this far!",
        "More even, time to thank some people!",
        "(You can't start credits early enough)",
        "(Says the guy who never finishes a project)",
        "In general: Thanks to anyone giving me feedback, as you make me keep going.",
        "More in particular: The people hanging out in the PsyOp Discord.",
        "Specifically (in no particular order):",
        "Falco, Cypress, Haikuno, krzysztophoros, mickschen " TX_SPRITE(SPRITE_HEART, 3, 3),
        "Also, I'd like to thank Thomas P. for being around and always up for a talk about the stuff I make!",
        "And of course my Family, especially my wife, for putting up with me.",
        "Then there's raysan5 to thank, the creator of raylib, which is such a nice framework to work with.",
        "And then there's a thanks to TinyCircuits, without which I wouldn't have started this project.",
        "That's all for now - if you think I forgot you, just ping Zet. He's so forgetful.",
        0
    };

    for (int i=0;outroTexts[i];i++)
    {
        ScriptedAction_addSetPlayerPosition(step, step, 64, 90, 1, 1);
        ScriptedAction_addSpeechBubble(step, step, outroTexts[i], 0, 8, 4, 112, 60, 0, -10);
        ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
        step++;
    }

    ScriptedAction_addSetPlayerTarget(step, step+1, 64, 140, 1, 1);
}

void Scene_played_through_update(RuntimeContext *ctx, TE_Img *screen)
{
    

}