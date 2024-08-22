#include <atlas.h>
#include "TE_rand.h"
#include "game.h"


Character enemyCharacters[MAX_ENEMYTYPES];
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

