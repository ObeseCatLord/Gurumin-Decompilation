// The per-character behaviour dispatch for NPCs and scripted characters (0x100dfcf0): every 30 frames it puts the
// character's shadow on the ground, then runs the behaviour selected by its script parameter (CharState m_003c).

#include "node.h"
#include "map_vars.h"
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "model_bank.h"

#include "char_state.h"

#include "char_vars.h"


extern "C" int FUN_1001b5b0(int index);                    // Char_UnpackParameter
extern "C" int FUN_10043cc0(int index);                    // Char_ResetState
extern "C" int FUN_100b1520(int index);                    // Npc_WatchForPlayer
extern "C" int FUN_100b2900(int index, int mode);
extern "C" int FUN_100b1800(int index);
extern "C" int FUN_10044560(int index);                    // Enemy_Pendulum
extern "C" int FUN_100b3430(int index);                    // Npc_RiseWhenSeen
extern "C" int FUN_10044b10(int index);
extern "C" int FUN_100451c0(int index);
extern "C" int FUN_100b3770(int index);
extern "C" int FUN_100b4300(int index);                    // Npc_RaiseAlarm
extern "C" int FUN_100b4930(int index);                    // Npc_RideAlarm
extern "C" int FUN_1007b870(int index);
extern "C" int FUN_100b1ff0(int index);                    // Npc_FaceAndTalk
extern "C" int FUN_100b23b0(int index);                    // Npc_ChildFollow
int FUN_10043fc0(int index);                               // Npc_RideMarker
extern "C" int FUN_10045830(int index);                    // Npc_SleepUntilWoken
extern "C" int FUN_10045b80(int index);                    // Char_IdleWatchPlayer
int FUN_100b4f00(int index);                               // Npc_PopUp
extern "C" int FUN_100b5550(int index);                    // Npc_GreetWhenNear
extern "C" int FUN_10045c30(int index);                    // Npc_WaitFacingAway
extern "C" int FUN_100460d0(int index);
extern "C" int FUN_100469d0(int index);
extern "C" int FUN_100b58a0(int index, int wakeEnemies);   // Npc_TalkWhenNear
extern "C" int FUN_1001b7b0(int index);
extern "C" int FUN_10046f10(int index);                    // Enemy_AppearNearby
extern "C" int FUN_10047400(int index);                    // Char_UpdateGroup
extern "C" int FUN_1001b840(int index);                    // Char_RandomIdle
extern "C" int FUN_10047440(int index);
extern "C" int FUN_10047a10(int index);
extern "C" int FUN_100b5ae0(int index);                    // Npc_Idle
extern "C" int FUN_10048050(int index);
int FUN_10048b70(int index);                               // Char_SwingDoor
int FUN_100b5c30(int index);                               // Npc_TurnWhenSeen
extern "C" int FUN_10048e90(int index);                    // Enemy_DropIn
extern "C" int FUN_100b6290(int index);                    // Npc_RunToMarker
extern "C" int FUN_100b6740(int index);
extern "C" int FUN_10049240(int index);                    // Enemy_DropInWhenNear
extern "C" int FUN_1001b880(int index);                    // Npc_TalkFacingSimple
extern "C" int FUN_100b7360(int index);
extern "C" int FUN_100b7b10(int index);
extern "C" int FUN_10045ed0(int index);                    // Npc_ApproachPlayer
int FUN_100b5f80(int index);                               // Npc_AlertWhenSeen
extern "C" int FUN_100b84a0(int index);
int FUN_100b9150(int index);   // C++ linkage: the original has an EH frame
extern "C" int FUN_100b9930(int index);                    // Enemy_DragToPit

// 0x100dfcf0 (Char_RunBehaviour). Every 30 frames (staggered by index), for characters that are not active
// (m_2498 0) or idle, it marks m_0012 and with DAT_102022b8 set moves the shadow matrix to the ground below them
// (cast from 1000 units back along the model bank's direction to 10 above them; z -3500 without a hit). Then it
// unpacks the behaviour number on first use and runs that behaviour; while DAT_104c0094 is set only behaviours
// 0x21, 10 and 0x24 run, on maps 0x25c and 0x12d with DAT_10c655e0 set. Unknown numbers reset the character's
// state. Returns 0.
extern "C" int FUN_100dfcf0(int index)
{
    if ((DAT_10db6ee8[index].mode == 0 || DAT_10db6ee8[index].m_0267 == 1) && DAT_104c0380 % 30 == index % 30)
    {
        DAT_10db6ee8[index].m_0012 = 2;
        if (DAT_102022b8 != 0)
        {
            Vec4 from;
            Vec4 ground;
            Vec4 top = DAT_10db6ee8[index].matrix.GetTranslation();
            from.Copy(top);
            top.z += 10.0f;
            Vec4 direction = *DAT_10804bf8.LightDirection(&DAT_10db6ee8[index], &DAT_118e69e8, &DAT_118e6cb8);
            direction.Normalized3();
            float distance = 1000.0f;
            from.Copy(from - direction * distance);
            if (DAT_10255f08 != 0 && DAT_10255f08->CastSegment2(0, 0x4000000, &from, &top, &ground, 0) != 0)
            {
                ground.x = from.x;
                ground.y = from.y;
                DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
            }
            else
            {
                ground.x = from.x;
                ground.y = from.y;
                ground.z = -3500.0f;
                DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
            }
        }
    }
    if (DAT_1020b430[index].behaviour == 0)
        FUN_1001b5b0(index);
    if (DAT_104c0094 == 0 ||
        ((DAT_10c62ed0 == 0x25c || DAT_10c62ed0 == 0x12d) && DAT_10c655e0 != 0 &&
         (DAT_1020b430[index].behaviour == 0x21 || DAT_1020b430[index].behaviour == 10 || DAT_1020b430[index].behaviour == 0x24)))
    {
        switch (DAT_1020b430[index].behaviour)
        {
        case 1:
            FUN_100b1520(index);
            return 0;
        case 2:
        case 0x15:
            FUN_100b2900(index, 0);
            return 0;
        case 3:
            FUN_100b1800(index);
            return 0;
        case 4:
            FUN_100b2900(index, 1);
            return 0;
        case 5:
            FUN_10044560(index);
            return 0;
        case 6:
            break;
        case 7:
            FUN_100b3430(index);
            return 0;
        case 8:
            FUN_10044b10(index);
            return 0;
        case 9:
            FUN_100451c0(index);
            return 0;
        case 10:
            FUN_100b3770(index);
            return 0;
        case 0xb:
            FUN_100b4300(index);
            return 0;
        case 0xc:
            FUN_100b4930(index);
            return 0;
        case 0xd:
            FUN_1007b870(index);
            return 0;
        case 0xe:
            FUN_100b1ff0(index);
            return 0;
        case 0xf:
            FUN_100b23b0(index);
            return 0;
        case 0x10:
            FUN_10043fc0(index);
            return 0;
        case 0x11:
            FUN_10045830(index);
            return 0;
        case 0x12:
            FUN_10045b80(index);
            return 0;
        case 0x13:
            FUN_100b4f00(index);
            return 0;
        case 0x14:
            FUN_100b5550(index);
            return 0;
        case 0x16:
            FUN_10045c30(index);
            return 0;
        case 0x17:
            FUN_100460d0(index);
            return 0;
        case 0x18:
            FUN_100469d0(index);
            return 0;
        case 0x19:
            FUN_100b58a0(index, 1);
            return 0;
        case 0x1a:
            FUN_1001b7b0(index);
            return 0;
        case 0x1b:
            FUN_10046f10(index);
            return 0;
        case 0x1c:
            FUN_10047400(index);
            return 0;
        case 0x1d:
            FUN_1001b840(index);
            return 0;
        case 0x1e:
            FUN_10047440(index);
            return 0;
        case 0x1f:
            FUN_10047a10(index);
            return 0;
        case 0x20:
            FUN_100b5ae0(index);
            return 0;
        case 0x21:
            FUN_10048050(index);
            return 0;
        case 0x22:
            FUN_10048b70(index);
            return 0;
        case 0x23:
            FUN_100b5c30(index);
            return 0;
        case 0x24:
            FUN_10048e90(index);
            return 0;
        case 0x25:
            FUN_100b6290(index);
            return 0;
        case 0x26:
            FUN_100b6740(index);
            return 0;
        case 0x27:
            FUN_10049240(index);
            return 0;
        case 0x28:
            FUN_1001b880(index);
            return 0;
        case 0x29:
            FUN_100b7360(index);
            return 0;
        case 0x2a:
            FUN_100b7b10(index);
            return 0;
        case 0x2b:
            FUN_10045ed0(index);
            return 0;
        case 0x2c:
            FUN_100b5f80(index);
            return 0;
        case 0x2d:
            FUN_100b84a0(index);
            return 0;
        case 0x2e:
            FUN_100b9150(index);
            return 0;
        case 0x2f:
            FUN_100b58a0(index, 0);
            return 0;
        case 0x31:
            FUN_100b9930(index);
            return 0;
        default:
            FUN_10043cc0(index);
            break;
        }
    }
    return 0;
}
