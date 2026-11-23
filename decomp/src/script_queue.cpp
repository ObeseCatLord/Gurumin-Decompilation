// Queued event scripts: a ring of 8 entries (DAT_1020247c, read index DAT_104bfe84, write index DAT_104bfe88)
// naming scripts in the script text table DAT_10c5e1c4. Each queued script is loaded into the script compiler
// (DAT_104687b8) and its main() is run.

#include <string.h>

#include "listbank.h"
#include "char_vars.h"
#include "game_vars.h"
#include "script_vars.h"

extern "C" int DAT_1020247c[8];       // queued script numbers

// Runs every queued script in order and empties the queue. Returns 0.
extern "C" int FUN_100fc3e0(void)
{
    while (DAT_104bfe88 != DAT_104bfe84)
    {
        char *script = DAT_10c5e1c4[DAT_1020247c[DAT_104bfe84]];
        if (script != 0)
        {
            DAT_1049eae4 = 0;
            DAT_104687b8.LoadText(script, strlen(script));
            DAT_1049eae4 = 0;
            DAT_10492d88 = 1;
            DAT_1049eadc = 0;
            DAT_1032d69c = 0;
            DAT_104c0338 = 0;
            DAT_104687b8.CompileStatement("main();");
            DAT_104c0338 = 0;
            DAT_1032d69c = 1;
            DAT_10492d88 = 0;
        }
        DAT_104bfe84 = (DAT_104bfe84 + 1) & 7;
    }
    return 0;
}
