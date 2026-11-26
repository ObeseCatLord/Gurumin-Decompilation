// Enemy voices with a cooldown (EN build, JP twin enemy_voice.cpp): each kind of talking enemy keeps a cooldown counter in
// one word of the AI state of its own slot or of a neighbour, and a voice only plays while that counter is zero.
#include "char.h"
#include "char_inline.h"
#include "enemy_voice.h"

// Enemy kinds that talk (CharRecord::kind), and the state word each keeps its cooldown in.
const int KIND_TALKER_100 = 100;
const int KIND_TALKER_46 = 0x46;
const int KIND_TALKER_69 = 0x69;
const int KIND_TALKER_2D = 0x2d;
const int KIND_TALKER_3F = 0x3f;
const int KIND_TALKER_43 = 0x43;
const int WORD_KIND_100 = 10;
const int WORD_KIND_46 = 11;
const int WORD_KIND_69 = 8;
const int WORD_KIND_2D = 1;
const int WORD_KIND_3F = 9;
const int WORD_KIND_43 = 15;

// Cooldowns in frames that a played voice starts.
const int COOLDOWN_10 = 10;
const int COOLDOWN_15 = 15;
const int COOLDOWN_20 = 20;
const int COOLDOWN_25 = 25;
const int COOLDOWN_30 = 30;
const int COOLDOWN_45 = 45;
const int COOLDOWN_55 = 55;
const int COOLDOWN_60 = 60;
const int COOLDOWN_75 = 75;
const int COOLDOWN_105 = 105;
const int COOLDOWN_120 = 120;
const int COOLDOWN_135 = 135;
const int COOLDOWN_285 = 285;
const int COOLDOWN_9 = 9;

// Enemy_PlayVoice. Plays voice `sound` of enemy `index` unless its cooldown is running (`limited` voices are skipped
// entirely unless g_voiceLimitsOff is set). The kinds 100, 0x46, 0x69, 0x2d, 0x3f and 0x43 keep their cooldown in word
// 10, 11, 8, 1, 9 and 15 of the state of slot `index` - 1, + 1, + 2 or `index` itself; other kinds never speak.
// `reset` clears the cooldown first. After playing, the sound sets the cooldown (9 to 285 frames). Returns 0. Called by
// the enemy behaviours.
extern "C" int Enemy_PlayVoice(int index, int sound, int reset, int limited)
{
    if (g_voiceLimitsOff == 0 && limited != 0)
        return 0;
    int kind = g_charRecords[index].kind;
    int word;
    if (kind == KIND_TALKER_100)
        word = WORD_KIND_100;
    else if (kind == KIND_TALKER_46)
        word = WORD_KIND_46;
    else if (kind == KIND_TALKER_69)
        word = WORD_KIND_69;
    else if (kind == KIND_TALKER_2D)
        word = WORD_KIND_2D;
    else if (kind == KIND_TALKER_3F)
        word = WORD_KIND_3F;
    else if (kind == KIND_TALKER_43)
        word = WORD_KIND_43;
    else
        return 0;
    if (reset != 0)
    {
        if (kind == KIND_TALKER_69)
            g_charState[index - 1].words[word] = 0;
        else if (kind == KIND_TALKER_2D)
            g_charState[index + 2].words[word] = 0;
        else if (kind == KIND_TALKER_3F || kind == KIND_TALKER_43)
            g_charState[index].words[word] = 0;
        else
            g_charState[index + 1].words[word] = 0;
    }
    // The voice plays only when the cooldown word that belongs to the kind is zero.
    if (!(((kind == KIND_TALKER_3F || kind == KIND_TALKER_43) && g_charState[index].words[word] == 0) ||
          (kind == KIND_TALKER_2D && g_charState[index + 2].words[word] == 0) ||
          (kind == KIND_TALKER_69 && g_charState[index - 1].words[word] == 0) ||
          (kind != KIND_TALKER_69 && kind != KIND_TALKER_2D && kind != KIND_TALKER_3F && kind != KIND_TALKER_43 &&
           g_charState[index + 1].words[word] == 0)))
        return 0;
    Character_PlaySoundInline(index, sound, 1, 1);
    switch (sound)
    {
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x26:
    case 0x2a:
    case 0x2b:
    case 0x2d:
        g_charState[index - 1].words[word] = COOLDOWN_30;
        return 0;
    case 0x24:
    case 0x25:
    case 0x28:
    case 0x29:
        g_charState[index - 1].words[word] = COOLDOWN_15;
        return 0;
    case 0x27:
        g_charState[index - 1].words[word] = COOLDOWN_105;
        return 0;
    case 0x2c:
        g_charState[index - 1].words[word] = COOLDOWN_20;
        return 0;
    case 0x2e:
        g_charState[index - 1].words[word] = COOLDOWN_55;
        return 0;
    case 0x2f:
        g_charState[index - 1].words[word] = COOLDOWN_60;
        return 0;
    case 0x30:
        g_charState[index - 1].words[word] = COOLDOWN_120;
        return 0;
    case 0x31:
    case 0x32:
    case 0x33:
        g_charState[index + 2].words[word] = COOLDOWN_15;
        return 0;
    case 0x34:
    case 0x35:
    case 0x36:
        g_charState[index + 2].words[word] = COOLDOWN_9;
        return 0;
    case 0x37:
    case 0x3a:
        g_charState[index + 2].words[word] = COOLDOWN_45;
        return 0;
    case 0x38:
    case 0x39:
    case 0x3b:
        g_charState[index + 2].words[word] = COOLDOWN_60;
        return 0;
    case 0x3c:
    case 0x3d:
    case 0x3e:
        g_charState[index + 2].words[word] = COOLDOWN_20;
        return 0;
    case 0x3f:
        g_charState[index + 2].words[word] = COOLDOWN_120;
        return 0;
    case 0x40:
    case 0x41:
    case 0x42:
        g_charState[index].words[word] = COOLDOWN_45;
        return 0;
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x4e:
        g_charState[index].words[word] = COOLDOWN_30;
        return 0;
    case 0x46:
    case 0x4f:
        g_charState[index].words[word] = COOLDOWN_120;
        return 0;
    case 0x47:
    case 0x48:
    case 0x49:
        g_charState[index].words[word] = COOLDOWN_10;
        return 0;
    case 0x4a:
    case 0x4c:
    case 0x4d:
        g_charState[index].words[word] = COOLDOWN_15;
        return 0;
    case 0x4b:
        g_charState[index].words[word] = COOLDOWN_25;
        return 0;
    case 0x1e4:
    case 0x1ec:
    case 0x1ed:
    case 0x1f3:
        g_charState[index + 1].words[word] = COOLDOWN_45;
        return 0;
    case 0x1e5:
        g_charState[index + 1].words[word] = COOLDOWN_135;
        return 0;
    case 0x1e6:
    case 0x1e7:
    case 0x1e8:
    case 0x1e9:
    case 0x1ea:
    case 0x1f1:
    case 0x1f2:
    case 0x1f6:
    case 0x1f7:
    case 0x1f8:
    case 0x1fd:
        g_charState[index + 1].words[word] = COOLDOWN_15;
        return 0;
    case 0x1eb:
        g_charState[index + 1].words[word] = COOLDOWN_285;
        return 0;
    case 0x1ee:
    case 0x1ef:
    case 0x1f0:
    case 0x1f4:
    case 0x1f5:
    case 0x1fe:
        g_charState[index + 1].words[word] = COOLDOWN_30;
        return 0;
    case 0x1f9:
        g_charState[index + 1].words[word] = COOLDOWN_60;
        return 0;
    case 0x1fa:
    case 0x1fb:
    case 0x1fc:
        g_charState[index + 1].words[word] = COOLDOWN_10;
        return 0;
    case 0x1ff:
        g_charState[index + 1].words[word] = COOLDOWN_75;
        return 0;
    }
    return 0;
}
