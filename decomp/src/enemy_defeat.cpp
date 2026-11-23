// Defeated enemies: the character is taken out of the world and leaves its loot behind.

#include "vec4.h"

#include "anim_model.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "debris.h"
#include "game.h"
#include "world.h"

#define STATE_FREE 0                       // DAT_10210d30 value of a character that has not been handled yet
#define STATE_FREE_DELAYED 1000            // the same, after a wait
#define STATE_DEFEATED 1
#define LOOT_STAT_BONUS 11                 // CharSlot::stats entry that, when set, raises the coins by half
#define OUT_OF_WORLD_HEIGHT -3390.0f       // matrix._43 of a character that was taken out of the world
#define LOOT_HEIGHT 50.0f                  // coins and items appear this far above the character
#define PUFF_PIECE 100                     // piece type of the burst of debris
#define BONUS_COIN_FACTOR 1.5f

// Gold-bonus bits of the worn helmet's face settings (DAT_104c02c8): coins x3, x2 and x1.5.
#define HELMET_GOLD_X3 0x30000000
#define HELMET_GOLD_X2 0x20000000
#define HELMET_GOLD_X1_5 0x10000000

// Debris piece types of the coin denominations.
#define COIN_1 0x51
#define COIN_5 0x52
#define COIN_10 0x53
#define COIN_50 0x54
#define COIN_100 0x55
#define COIN_500 0x56

// Roll limits (of 100) of the three item drops, checked in turn.
#define DROP_CHANCE_ITEM_C4 2
#define DROP_CHANCE_ITEM_C0 4
#define DROP_CHANCE_ITEM_BC 6

extern "C" int FUN_1013f880(void);   // the game's random number

// 0x100627d0 (Enemy_Defeat). Handles character `index` once it has been defeated (state 0 or 1000, else returns 0):
// zeroes its health, moves it far below the world, throws a burst of 15 debris pieces around its position, and the
// first time (state 0) marks its map id killed, tells it to react and clears the colour value of its model. Then
// drops its coins as debris pieces by denomination (500, 100, 50, 10, 5, 1), the amount raised by half when the
// slot's bonus stat is set and again by the helmet's gold bonus, and rolls once for one of its three items (rolls 0-1, 2-3
// and 4-5 of 100: the debris type in slot fields 0xc4, 0xc0 and 0xbc). Sets the state to 1. Returns 0.
extern "C" int FUN_100627d0(int index)
{
    if (DAT_10210d30[index] != STATE_FREE && DAT_10210d30[index] != STATE_FREE_DELAYED)
        return 0;
    Vec4 position;
    DAT_10daaae8[index].stats[STAT_HEALTH] = 0;
    DAT_10db6ee8[index].matrix._43 = OUT_OF_WORLD_HEIGHT;
    DAT_10db6ee8[index].HoldAtSpine(0, 0);
    position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    Vec4 direction;
    direction.Copy(position);
    direction.z += LOOT_HEIGHT;
    direction.Copy(direction - DAT_10836fc8[index]);
    FUN_1000df10(position.x + 5.0f, position.y, position.z + 50.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x, position.y + 5.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x - 4.0f, position.y, position.z + 50.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x + 8.0f, position.y + 5.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x - 5.0f, position.y, position.z + 50.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x, position.y + 7.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x - 5.0f, position.y - 5.0f, position.z + 50.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x + 5.0f, position.y, position.z + 45.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x - 4.0f, position.y + 7.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x + 5.0f, position.y - 5.0f, position.z + 60.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x - 5.0f, position.y + 10.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x + 10.0f, position.y - 5.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x - 5.0f, position.y - 10.0f, position.z + 40.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x + 4.0f, position.y + 7.0f, position.z + 55.0f, &direction, PUFF_PIECE, index);
    FUN_1000df10(position.x + 10.0f, position.y - 10.0f, position.z + 65.0f, &direction, PUFF_PIECE, index);
    if (DAT_10210d30[index] == STATE_FREE)
    {
        DAT_10874848.MarkKilled(DAT_10daaae8[index].routeRecord);
        DAT_10210b30[index] = 1;
        DAT_10db6ee8[index].root->M_10194730(0, -1);
    }
    int coins = DAT_10daaae8[index].coins;
    if (DAT_10daaae8[index].stats[LOOT_STAT_BONUS] != 0)
        coins = (int)(coins * BONUS_COIN_FACTOR);
    if ((DAT_104c02c8 & HELMET_GOLD_X3) == HELMET_GOLD_X3)
        coins = (int)(coins * 3.0f);
    else if ((DAT_104c02c8 & HELMET_GOLD_X2) != 0)
        coins = (int)(coins * 2.0f);
    else if ((DAT_104c02c8 & HELMET_GOLD_X1_5) != 0)
        coins = (int)(coins * BONUS_COIN_FACTOR);
    while (coins != 0)
    {
        if (coins >= 500)
        {
            FUN_1000dd90(position.x, position.y, position.z + LOOT_HEIGHT, COIN_500);
            coins -= 500;
        }
        else if (coins >= 100)
        {
            FUN_1000dd90(position.x, position.y, position.z + LOOT_HEIGHT, COIN_100);
            coins -= 100;
        }
        else if (coins >= 50)
        {
            FUN_1000dd90(position.x, position.y, position.z + LOOT_HEIGHT, COIN_50);
            coins -= 50;
        }
        else if (coins >= 10)
        {
            FUN_1000dd90(position.x, position.y, position.z + LOOT_HEIGHT, COIN_10);
            coins -= 10;
        }
        else if (coins >= 5)
        {
            FUN_1000dd90(position.x, position.y, position.z + LOOT_HEIGHT, COIN_5);
            coins -= 5;
        }
        else if (coins >= 1)
        {
            FUN_1000dd90(position.x, position.y, position.z + LOOT_HEIGHT, COIN_1);
            coins--;
        }
    }
    int roll = FUN_1013f880() % 100;
    int item;
    if (roll < DROP_CHANCE_ITEM_C4 && (item = DAT_10daaae8[index].m_00c4) != 0)
        FUN_1000dd90(position.x, position.y, position.z + LOOT_HEIGHT, item);
    else if (roll < DROP_CHANCE_ITEM_C0 && (item = DAT_10daaae8[index].m_00c0) != 0)
        FUN_1000dd90(position.x, position.y, position.z + LOOT_HEIGHT, item);
    else if (roll < DROP_CHANCE_ITEM_BC && (item = DAT_10daaae8[index].money) != 0)
        FUN_1000dd90(position.x, position.y, position.z + LOOT_HEIGHT, item);
    DAT_10210d30[index] = STATE_DEFEATED;
    return 0;
}
