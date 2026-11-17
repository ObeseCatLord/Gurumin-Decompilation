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
- 2026-11-16 compiler finding: **/GS proves cl 13.00 (VS .NET 2002).** The original prologue is `mov eax,cookie; xor eax,[esp+N]`
  (cookie XOR return address) and the epilogue tail-jumps to `__security_check_cookie`. VS2003 SP1 (13.10.6030) emits
  `xor eax,esp` + call; Toolkit 2003 (13.10.3052) emits no xor + call, otherwise the same schedule as the original.
  Consequence: every function with a local array (/GS-protected) is unmatchable with 13.10. Toolkit and SP1 give identical
  results on all other functions so far. Flags now `/O2 /Ob1 /Gy /GS` (`/Ob1` proven: non-`inline` callees are not inlined).
  Status: 113 matched, 10 open (`decomp/open.tsv`: 13.00 codegen/encoder/GS walls). A cl 13.00.9466 would clear most of them.
- Recurring cl 13.00 walls (see decomp/open.tsv): **any [esp] memory operand is encoded with a zero disp8** (`89 44 24 00` vs VS2003 `89 04 24`), so every function that touches a local at `[esp+0]` differs by one byte per access; /GS prologue/epilogue; occasional register/schedule choices. 13.10 cannot fix these from source. Current: ~157 matched, 25 open.
- **Inter-procedural register allocation** shows up in several functions (0x10161970, 0x101a2370, 0x101af420): the caller keeps a value in edx across a call to a callee known not to touch edx. That suggests /GL + /LTCG (available since VS .NET 2002) or intra-module IPRA; VS2003 per-file compilation cannot reproduce it. Marked `skip` in open.tsv.
- **/GX (C++ EH) is on in the original**: destructors with EH state frames match byte for byte with `/GX` and nothing regresses, so flags are now `/O2 /Ob1 /Gy /GS /GX`.
- Functions with an EH frame (`push -1 / push handler / fs:[0]`) must be written with **C++ linkage**: under /GX an `extern \"C\"` function is assumed not to throw and gets no frame. Map the mangled name to the address in `decomp/obj_symbols.tsv`.
- Another recurring 13.00 habit: when 0 is stored several times it is kept in a callee-saved register (`xor ebx,ebx`); VS2003 stores immediates. Seen in the enemy AI updaters 0x100e2080.. (source otherwise identical).
- At least one translation unit around 0x1013e6f0 was compiled **without optimisation** (/Od: ebp frames, every variable in memory, int3 padding between functions). VS2003 /Od lays the locals out differently, so those stay open for now (decomp/src_od/).
- 2026-11-16 status: 240 matched, 4 equivalent, 27 open of 1,212 game functions. All small (<=80 bytes) non-float functions are done; the open list is dominated by cl 13.00 walls (zero-register reuse, [esp] encoding, /GS, IPRA, /Od TU).
- cl 13.00 unrolls simple counted loops by 8 where VS2003 unrolls by 4 (0x101619a0).
- **Shared epilogue vs duplicated tails:** "VS2003 duplicates the return tail" was often a source spelling, not a wall.
  `if (g != 0 && (w = Get()) != 0) { ... } return 0;` gives the original's single epilogue (0x10001a90), and
  `if (g == 0) return 0; w = Get(); if (w != 0) { ...; return x; } return 0;` gives its `xor eax,eax` exit block
  (0x10001ba0, 0x10001bf0). Retry the open "tail duplication" entries with these forms.
- Anti-tamper stat guards decoded and matched (`src/anti_tamper.cpp`; see MODDING_MAP "Anti-cheat"); the checker
  0x1000e870 is 5 bytes off (edi/ebx swap in one loop). Counter 0x104c034c is a `short`, frame counter 0x104bf890 unsigned.
- **"Constant kept in a register" is not always a cl 13.00 habit.** VS2003 also loads the constant once (`mov eax,1` /
  `xor eax,eax`) when it fully unrolls a short constant-count loop or expands `memset`; it stores immediates only for
  separately written assignments. 0x100235e0 matched once its fills were written as `memset` and a `for` loop.
  Retry the open "zero/1.0f in a register" entries with loops.
- EH state stores that the original leaves out before a call (0x101728c0: no state update before `ModelState()`)
  match when that callee is declared `throw()`. The original compiler evidently knew the callee could not throw,
  which fits whole-program /GL+LTCG (see the IPRA note). Use `throw()` on such declarations only where the
  original shows the missing state store.
- Callee-saved push placement follows the early exits: a test written as its own `if (x == 0) return 0;` before the rest keeps the push after it (0x1015c9e0). Where the original pushes *before* the first test (0x100014b0, 0x10199be0) no spelling has been found yet.

## 2026-11-17: the original compiler (VS .NET 2002, cl 13.00.9466)
- The user installed VS .NET 2002 at `E:\Visual StudioNET 2002\`; `cl` reports 13.00.9466 and `link` 7.00.9466, the
  build in the Rich header. `decomp/env.bat` now uses it by default (`GURUMIN_VC` overrides).
- With unchanged sources: 334 matched (VS2003: 325 + 5 EQUIV). Every EQUIV (the `[esp+0]` disp8 encoding) and the /GS
  cookie function (0x1015fe20) match, so **all 190 /GS functions (1.07 MB of the 1.72 MB of code) are now in reach**.
  Two functions regressed because their source had been bent for VS2003; the plain forms match (0x10004650:
  `while (--count >= 0)`; 0x100235e0: seventeen separate `= 1` stores instead of a loop, which 13.00 turns into `rep stosd`).
- **The original used `/Op`** (improve float consistency): with it, three more functions match and none regress
  (0x100114a0's 1.0f kept in a register, KeyTrack::Insert's float parameters copied with integer moves, KeyBank::Clear).
  int-to-float conversions are rounded through the parameter's own stack slot (`fild [esp+8]; fstp [esp+8]`).
  Flags are now `/O2 /Ob1 /Op /Gy /GS /GX`.
- **Revisit every VS2003-era wall with the plainest source.** Workarounds written for 13.10 now hurt: the lazy-create
  getter 0x1017b1c0 and the shared-tail functions 0x10047400 / 0x100158b0 match as a plain `if`/`else` followed by the
  common code. The unroll-by-8 searches (0x100119d0, 0x10011aa0, 0x10148410, 0x100020d0) match as plain `for` loops.
- The remaining `open.tsv` reasons were written against VS2003; read them as "differs", not as a diagnosis.
- Still open under 13.00: the enemy AI updaters (0 kept in ebx, also used as the switch base: `sub eax,ebx`), register
  ties of a few bytes (0x1015cac0, 0x10169e10, 0x101992f0, 0x10034c80), 0x1000da40 (only the reload of the target).
- Tools: `tools/variants.py <file> <n> [flags]` builds `#if V == k` variants; `tools/pick.py <file> <k>` keeps one;
  `tools/sbs.py <obj> <symbol>` prints ours and the original side by side. From Git Bash set `MSYS_NO_PATHCONV=1`,
  or `/Op`-style flags are rewritten into paths.

- Float code now matches routinely under 13.00 + /Op. Patterns: an int-to-float assignment written plainly
  (`global = value;`) and repeated in a branch gives the original's single rounded temporary (0x1000da40 family);
  `if (cond) continue;` reproduces the NaN-aware `test ah,5 / jnp` of a negated float compare (0x100123d0); accessing
  `array[index].field` directly instead of through a local reference or pointer gives `lea reg,[reg+addr]`
  where a local gives `add reg,addr` (0x10016500, 0x1001a390). math.h's inline `acosf`/`atan2f` were emitted
  out of line (0x10001f60, 0x10001f80, symbols `_acosf`/`_atan2f`).
- /GS functions match like any other under 13.00 (0x10169780, 0x1013e0c0, 0x1005cdc0).
- **Functions with an EH frame need C++ linkage.** Under /GX (= /EHsc) a function *defined* `extern "C"` is
  treated as unable to throw, so its EH frame disappears. Name it `FUN_xxxxxxxx` without `extern "C"`
  (compare_obj and build_all accept the mangled `?FUN_xxxxxxxx@@...` names); declare C++ callees the same way.
- GameFile (0x101ad9e0 ctor, 0x101b2570 dtor -> File_Close) is 0x28 bytes: `FILE *handle; char name[0x24];`
  (Save_LoadSlot's frame). Save_LoadSlot builds one and never uses it.
- `strcmp(word, "literal")` is inlined as `repe cmpsb` with the literal's length including the NUL (0x100364a0);
  `memcmp` with the same length becomes dword/word compares instead.
- Pointer loops that end with a signed `jl` against the end address come from index loops (`for (i = 0; i < n; i++)
  a[i] = ...`), not pointer loops (0x1014b1c0).
- Local arrays are laid out in declaration order from the top of the frame down: the one declared first sits higher
  (ItmScramble: `table` declared before `key`).
