#include <atlas.h>
#include "TE_rand.h"
#include "game.h"
#include "game_character.h"

Character characters[MAX_ENEMYTYPES];
Projectile projectiles[PROJECTILE_MAX_COUNT];
Enemy enemies[MAX_ENEMIES];

Player player = {
    .x = 64,
    .y = 64,
    .dx = 0,
    .dy = 0,
    .dirX = 0,
    .dirY = 1,
    .maxHealth = 6,
    .health = 6
};

Item items[16];
Character playerCharacter;

TE_Img atlasImg;

int Characters_raycastCircle(Character* ignore, int x, int y, int radius, int16_t *outCenterX, int16_t *outCenterY, int16_t *outRadius)
{
    int i;
    for (i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].health > 0 && &enemies[i].character != ignore)
        {
            if (Character_raycastCircle(&enemies[i].character, x, y, radius, outCenterX, outCenterY, outRadius))
            {
                return i + 1;
            }
        }
    }
    if (&playerCharacter != ignore && Character_raycastCircle(&playerCharacter, x, y, radius, outCenterX, outCenterY, outRadius))
    {
        return 1;
    }
    return 0;
}