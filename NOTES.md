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
- **Callee register knowledge inside one file** (corrected 2026-11-17): the caller keeps a value in edx across a call
  when the callee is *defined earlier in the same translation unit* and the compiler can see it leaves edx alone.
  This is ordinary cl 13.00 behaviour, not /GL. Place such functions after their callee in one file: the Vec4
  operators next to `Vec4()` in vec4.cpp (0x101ac9e0..0x101acbc0), KeyBank's constructor there too (0x101af420),
  Node::M_1019d6a0 after M_1019d680 and Node::M_101a2370 after M_101944d0 in scene_node.cpp.
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
  probably because the callee was defined in the same file (see the callee note above). Use `throw()` on such
  declarations only where the
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
- Math library (0x101ac000-0x101ae000, src/vec_math.cpp): `sqrt`/`sin`/`cos` are calls under /Op (no fsqrt inlining);
  float arguments of an inline helper are copied to the stack in right-to-left argument order (`LengthSquared(x, y, z)`
  copies z, y, x). Chained assignments store right to left (`a = b = c = 0` stores c first), which explains odd store
  orders (0x101ae770). Stores otherwise keep source order, so write them in the original's order (0x101ae710).
- Vec4 operators that return by value (0x101ac9e0 +, 0x101acb00 *, 0x101acb60 /) keep `this` in edx across the
  `Vec4()` constructor call because `Vec4()` is defined in the same file (vec4.cpp); they all match there.
- An empty `switch` case still counts when the compiler picks the split value if its body is not empty in the source
  (WindowProc's `case WM_KEYDOWN:` with a dead `if`).
- /Od translation units (src_od/) are built with `/Od /Oi`: strcpy expands inline there (0x1013e6f0). build_all includes them.
- Functions called on freshly allocated memory without a null check are plain methods (`node->Init()`), not placement
  `new` (0x1000b9f0).
- **The compiler picks its own operand order for `+` and `*` of equal-weight terms**; source order and parentheses
  inside one expression do not steer it. When the original evaluates the "wrong" term first, the first term was a
  named temporary: `float cosY = cos(a) * y; float ny = cosY + sin(a) * x;` (Vec4::RotateZ/RotateY/RotateX
  0x101ac300/0x101ac3a0/0x101ac440). Plane::Distance (0x101ad240) still resists because a temporary adds a store there.
- `-z` as a multiplication operand keeps `fld z; fchs; fmulp` when written `(-z)` (RotateY/RotateX).
- A min/max assignment whose value is the variable itself (`x = 1 < x ? x : 1`) is folded away; when the original
  keeps the self-copy, the result went into a new local (`float clamped = ...`, Input::Rumble 0x10142700).
- Two `return f(..., a, ...)` / `return f(..., 0, ...)` calls in an if/else let the compiler share the pushes of the
  common arguments, including a pushed 0 that doubles as the second call's argument (Texture_Load 0x1001e1a0); a
  local or ternary does not.
- `char` padding arrays inside a local struct turn on the /GS cookie; pad local structs with `unsigned long` arrays
  (VideoSurface::CopyTo 0x1015c6a0). `DrawText` is a windows.h macro: do not use it as a method name.
- A float temporary that lands in a dead parameter's stack slot comes from an expression passed straight to a
  `const float &` parameter (`v * (1.0f / frames)`, 0x1000ec70) or from a local assigned after the parameter's
  last use (FrameBlend_Start 0x1013fb10); a named local computed earlier gets its own slot.
- Whether a callee is *defined* in the same translation unit changes stack-slot sharing: with Cross defined in
  vec_math.cpp, PlaneFromTriangle (0x101af360) gave the Cross return temporary its own slot (frame 0x10 too big);
  in vec_ops.cpp, where Cross is only declared, it matches. Use this as evidence of the original file split.
- Same-file knowledge also decides **stack-slot reuse**: when a local vector's address is passed to a function the
  compiler cannot see, the slot is never reused for later temporaries. With Vec3_Dot / Vec3_LengthSquared defined in
  vec4.cpp, the segment-distance functions (0x101acd90, 0x101af260) put their last temporary in the dead direction
  vector and match. A frame that is 0x10 too big is a hint to move the callees in.
- Texture_Load (0x1001e1a0) is also an inline helper: functions that load a fixed texture expand it, including
  three `strlen` loops over the literal (0x10050460). `include/texture_load.h` holds it as `LoadTexture`.

## Source style (2026-11-17)
- Comment generously: a header comment per file, and a comment above every function saying what it does, who calls
  it and what its fields, constants and magic numbers mean. Comments never change the compiled bytes. The user's
  rule: "you can never have too many code comments." `decomp/QUEUE.md` section 7 lists files still missing them;
  `tools/annotate.py <notes.py>` inserts comment blocks above given signatures.
- Prefer readable names over placeholders once a function's role is clear; map non-address names to their address in
  `obj_symbols.tsv`, then run `tools/sync_symbols.py` to copy them into `symbols.tsv`.
- Some engine functions were written in inline assembly (frame pointer plus `or reg,reg` tests a compiler never
  emits): Mmv_DecodeFrameRle 0x10160a20 matches as an `__asm` block. Locals go in declaration order below ebp; label
  placement must follow the original jumps exactly. Do not name variables used inside `__asm` `length`, `size` or
  `type` (MASM operators).
- Two separate early returns (`if (!p) return x; if (!*p) return x;`) and a combined `||` condition give different
  block layouts (Node::Find 0x10191a30 needs the separate form).
- Call arguments that are themselves calls are evaluated before the callee's `this` is loaded only when written
  into a local first (TextureObject::BindStage0 0x1014b670).
- Same object, several names: ListBank (listbank.cpp), ResourceBanks (resource_banks.cpp) and TokenReader
  (type_parse.cpp) are one class (the script parser and its buffer banks); SceneOwner (texture_mgr.cpp) is the
  CharModel inside Character, and its `scene` is the Playback object (model_state.cpp). Merge them into shared headers
  (QUEUE section 8).
- A pointer chosen in an if-chain and used once afterwards (`p = &a[i].f; ... if (*p > 0) (*p)--;`) gives
  `add reg, symbol`; the original's `lea reg, [reg + symbol]` into a shared tail comes from repeating the whole
  statement in every branch and letting the compiler merge the tails (0x1001d7d0).
- Sums of products are reordered by the compiler regardless of source order; only the grouping (parentheses)
  steers the evaluation order (Plane::IntersectRay 0x101ad2c0: y + (x + z)).
- A search loop whose found-branch only `break`s is unrolled by 8; putting the work inside the loop and returning
  from there keeps the simple loop (ModelBank::Add 0x1017c570).
- A function's stack frame size reveals the type of its locals: three floats computed into a 16-byte constructor-less
  local is a `float t[4]` (TransformNormal 0x101ae130), not a Vec3 or separate floats.
- TextureObject +0x08/+0x0c are the texture width and height in pixels (FUN_101616e0 divides pixel UVs by them).
