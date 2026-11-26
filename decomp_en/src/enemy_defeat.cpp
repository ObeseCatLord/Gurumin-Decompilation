// Defeated enemies (EN build): the character is taken out of the world and leaves its loot behind. Ported from the
// matched JP source decomp/src/enemy_defeat.cpp.
#include "anim_model.h"
#include "char.h"
#include "char_attach.h"
#include "debris.h"
#include "enemy_defeat.h"
#include "engine.h"
#include "game.h"
#include "node.h"
#include "vec4.h"

// Handles character `index` once it has been defeated (phase 0 or 1000, else returns 0): zeroes its health, moves it
// far below the world, throws a burst of 15 debris pieces around its position (offsets from the character in x, y
// and z are given at each call), and the first time marks its map id killed, tells it to react and clears the colour
// value of its model. Then drops its coins as debris pieces by denomination (500, 100, 50, 10, 5, 1), the amount
// raised by half when the slot's bonus stat is set and again by the helmet's gold bonus, and rolls once for one of its
// three items (rolls 0-1, 2-3 and 4-5 of 100). Sets the phase to 1. Returns 0. Called by the character updater.
extern "C" int Enemy_Defeat(int index)
{
    if (g_charPhase[index] != DEFEAT_PHASE_FRESH && g_charPhase[index] != DEFEAT_PHASE_FRESH_DELAYED)
        return 0;
    Vec4 position;
    g_charRecords[index].stats[STAT_HEALTH] = 0;
    g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] = OUT_OF_WORLD_HEIGHT;
    g_characterModels[index].HoldAtSpine(0, 0);
    position.Copy(g_characterModels[index].matrix.GetTranslation());
    Vec4 direction;
    direction.Copy(position);
    direction.z += LOOT_HEIGHT;
    direction.Copy(direction - g_charLootOrigin[index]);
    Debris_SpawnFromCharacter(position.x + 5.0f, position.y, position.z + 50.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x, position.y + 5.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x - 4.0f, position.y, position.z + 50.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x + 8.0f, position.y + 5.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x - 5.0f, position.y, position.z + 50.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x, position.y + 7.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x - 5.0f, position.y - 5.0f, position.z + 50.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x + 5.0f, position.y, position.z + 45.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x - 4.0f, position.y + 7.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x + 5.0f, position.y - 5.0f, position.z + 60.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x - 5.0f, position.y + 10.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x + 10.0f, position.y - 5.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x - 5.0f, position.y - 10.0f, position.z + 40.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x + 4.0f, position.y + 7.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    Debris_SpawnFromCharacter(position.x + 10.0f, position.y - 10.0f, position.z + 65.0f, &direction, PUFF_PIECE, index);
    if (g_charPhase[index] == DEFEAT_PHASE_FRESH)
    {
        g_game.MarkKilled(g_charRecords[index].routeRecord);
        g_charToldToReact[index] = 1;
        g_characterModels[index].root->SetColourB(DEFEAT_COLOUR_VALUE, DEFEAT_COLOUR_MASK);
    }
    int coins = g_charRecords[index].money;
    if (g_charRecords[index].stats[LOOT_STAT_BONUS] != 0)
        coins = (int)(coins * BONUS_COIN_FACTOR);
    if ((g_helmetFaceSettings & HELMET_GOLD_X3) == HELMET_GOLD_X3)
        coins = (int)(coins * HELMET_GOLD_X3_FACTOR);
    else if ((g_helmetFaceSettings & HELMET_GOLD_X2) != 0)
        coins = (int)(coins * HELMET_GOLD_X2_FACTOR);
    else if ((g_helmetFaceSettings & HELMET_GOLD_X1_5) != 0)
        coins = (int)(coins * BONUS_COIN_FACTOR);
    while (coins != 0)
    {
        if (coins >= COIN_VALUE_500)
        {
            Debris_Spawn(position.x, position.y, position.z + LOOT_HEIGHT, COIN_500);
            coins -= COIN_VALUE_500;
        }
        else if (coins >= COIN_VALUE_100)
        {
            Debris_Spawn(position.x, position.y, position.z + LOOT_HEIGHT, COIN_100);
            coins -= COIN_VALUE_100;
        }
        else if (coins >= COIN_VALUE_50)
        {
            Debris_Spawn(position.x, position.y, position.z + LOOT_HEIGHT, COIN_50);
            coins -= COIN_VALUE_50;
        }
        else if (coins >= COIN_VALUE_10)
        {
            Debris_Spawn(position.x, position.y, position.z + LOOT_HEIGHT, COIN_10);
            coins -= COIN_VALUE_10;
        }
        else if (coins >= COIN_VALUE_5)
        {
            Debris_Spawn(position.x, position.y, position.z + LOOT_HEIGHT, COIN_5);
            coins -= COIN_VALUE_5;
        }
        else if (coins >= COIN_VALUE_1)
        {
            Debris_Spawn(position.x, position.y, position.z + LOOT_HEIGHT, COIN_1);
            coins -= COIN_VALUE_1;
        }
    }
    int roll = Random() % DROP_ROLL_RANGE;
    int item;
    if (roll < DROP_CHANCE_ITEM_C4 && (item = g_charRecords[index].m_00c4) != 0)
        Debris_Spawn(position.x, position.y, position.z + LOOT_HEIGHT, item);
    else if (roll < DROP_CHANCE_ITEM_C0 && (item = g_charRecords[index].m_00c0) != 0)
        Debris_Spawn(position.x, position.y, position.z + LOOT_HEIGHT, item);
    else if (roll < DROP_CHANCE_ITEM_BC && (item = g_charRecords[index].m_00bc) != 0)
        Debris_Spawn(position.x, position.y, position.z + LOOT_HEIGHT, item);
    g_charPhase[index] = DEFEAT_PHASE_DONE;
    return 0;
}
