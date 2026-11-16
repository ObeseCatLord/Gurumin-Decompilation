# Gurumin (JP, 2006) matching decompilation — notes

Do not publish game files, extracted assets or decompiled code beyond what the project's license allows.

## Targets
Source: `Gurumin\Gurumin JP\` (retail JP install). Pristine copies + SHA-256 in `re\bin\` (never modify).

| Binary | Size | Linker/timestamp | Role |
|---|---|---|---|
| `game.exe` | 692,224 | 7.0 / 2006-05-08 | small launcher |
| `bin\APP.dll` | 2,375,680 | 7.0 / 2006-05-09 | **main game code (first target)**; D3D9 build |
| `bin\APP8.dll` | 2,359,296 | 7.0 / 2006-05-09 | same code, D3D8 build (`.data1` section extra) |
| `Packs/sound3d/Update2/SoundArc/PADDLL.dll` | small | 2004 | helper DLLs, later |

The English Steam `game.exe` (6 MB, linker 12.0 / VS2013, 2015) is a rebuild and **cannot be byte-matched** from this
toolchain; it is out of scope for matching. Modern-hardware work should target the JP code base (or a separate layer).

## Compiler (decoded Rich header)
APP.dll / APP8.dll: bulk of objects built with product build **9466** (Visual Studio .NET 2002, cl 13.00.9466, linker 7.00):
54 C++ (prod 0x40) + 141 C (0x1c) + 32 (0x1d, MASM/resource-ish) objects. A few objects (0x5d/0x5f/0x60) carry build 4035
(13.10.4035, a 2003-era pre-release/DDK), and some imported libs use 9162/9178/9210 (SDK/CRT libs).
Available here: VS .NET 2003 SP1 (cl 13.10.6030) and C++ Toolkit 2003 (13.10.3052). VC6 (12.00) is **not** the
build compiler. The user chose: try 2003 first; a 2002 (13.00.9466) toolchain would be the exact build — ask if findable.
Expect codegen differences between 13.00 and 13.10 (scheduling/inlining); judge each function empirically.

## Layout
- `re\` Ghidra project (`run_ghidra_export.bat` → `re\export\`), scripts copied from the TOD/TSH projects.
- `decomp\` source (`src\`), headers (`include\`), `env.bat` (VS2003 environment), build output (`build\`).

## Status
- 2026-10-29: skeleton created. Ghidra analysis of APP.dll done (`re\export\APP.dll\`: `functions.tsv`, `decompiled.c`,
  `data_refs.tsv`, `imports.tsv`, `exports.tsv`). Image base 0x10000000; one export `GuruMain` (0x1013dfb0); entry 0x101b5a8b.
  3,085 functions, ~1.85 MB of code. Imports: kernel32, user32, imm32, gdi32, winmm, shell32, ole32, advapi32,
  **d3d9 (Direct3DCreate9) and dinput8** only — rendering/input go through COM, so hook points for modernisation are few.
  Largest functions: 0x1011d810 (107 KB), 0x100d24a0 (55 KB), 0x10151ba0 (43 KB), 0x10104640 (33 KB), 0x100edea0 (32 KB).
  Next: locate the CRT boundary (entry 0x101b5a8b is late, CRT likely follows ~0x1018xxxx+), test-compile a few small
  functions with VS2003 to gauge 13.00-vs-13.10 divergence, then port the TSH tooling (gen/build/compare_obj).
- 2026-10-29: toolchain proven. `decomp\build.bat <file.cpp>` compiles with VS2003 (`/O2 /Gy`, C++) and byte-compares every
  `FUN_xxxxxxxx` / `M_xxxxxxxx` function with APP.dll (`tools\compare_obj.py`; relocations masked; the compared length includes
  the inter-function alignment padding, which also matches). Helpers: `tools\asm.py <addr>` (original), `tools\objasm.py <obj> <name>`
  (ours), `tools\fn.py <addr>` (Ghidra C). Run via `& .\build.bat` in PowerShell (cmd /c and bash fail to find it). Python:
  `C:\Users\Michael\AppData\Local\Programs\Python\Python313\python.exe` (has capstone).
  First matches in `src\app_0001.cpp`: 0x10001000, 0x10001010 (thiscall method; two separate `if (x == 0) return 0;` — the `&&`/`||`
  forms give a different block layout), 0x10001050. 3/3 trivial functions match with VS2003 `/O2` → 13.10 is viable at least for
  simple code; larger functions will show whether 13.00-vs-13.10 scheduling differs.
  CRT/statics start around 0x101b29e9 (`__security_check_cookie`, stdio, etc.); game code is 0x10001000–~0x101b0000;
  0x101d0000+ is ~1,200 tiny import thunks. `/GS` cookie present (VS2002+).
- Sample so far (6 small functions, `src\app_0001.cpp`, `src\app_0002.cpp`): 6/6 match with VS2003 `/O2 /Gy` (incl. calls, imports, globals).
  Needs `#include <windows.h>` (VS2003 PlatformSDK path in `env.bat` works). Callees' calling conventions come from the original
  (`ret N` = `__stdcall`; no stack cleanup after a call = callee cleans).
  **First divergence: 0x1000da40** (float: `(float)int` stored through an `fstp [esp+8]` / integer reload spill, stores
  of the same float twice). VS2003 `/O2`, `/Op`, `/fp:*` all give different code (`fild`/`fst` kept on the FPU stack). Likely
  13.00-vs-13.10 float codegen; unresolved. Float-heavy functions (math, camera, animation) may be unmatchable without cl 13.00.9466.
  Candidate next steps: (1) look for a VS .NET 2002 compiler (the exact build); (2) match the integer/string/COM-glue functions first
  with VS2003; (3) build a function-triage table (size, float ops, callees) to prioritise.
- Triage: `decomp\tools\triage.py` → `decomp\triage.tsv` (game code 0x10001000–0x101b0000 = 1,212 functions; 513 contain x87 ops;
  356 are ≤120 bytes without FP). Likely a single huge "Game" object (members at +0x3ee694 etc.), thiscall methods named `M_xxxxxxxx`.
- `src\app_0003.cpp`: 0x100022b0, 0x10002300, 0x10008430 match. Lessons: (1) trust the asm, not Ghidra's conditions
  (`-x < 3999 || x == -3999` was really `x = -x; if (x > 3999)`); (2) compare operand order matters (`x > right`, not `right < x`);
  (3) a shared `return 1` block can reveal the original's branch structure. Running tally: 9 of 10 attempted functions match
  (the one failure is the float function 0x1000da40).
- 2026-11-16: 19 functions match (`src\app_0001..0005.cpp`, `build_all.bat` totals them). Shared layout in `include\game.h`
  (hand-written `struct Game`, thiscall methods `M_xxxxxxxx`; slot arrays at +0x8c/+0xa4/+0x110, flag array +0x3ee694).
  Lessons: pointer-to-member needs a *complete* class or cl emits 16-byte MI pointers (`@28` instead of `@16`); `for (i = n-1; i >= 0; i--)`
  reproduces `dec eax; js` guards; a repeated "find slot" loop across four functions is one inlined `FindSlot`; branch order
  (`if (p) {...} else {...}` vs null-first) decides block layout. Still unmatched: 0x1000da40 (float).
