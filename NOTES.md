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
- **Inline budget**: within one function cl 13.00 expands an inline method only a few times, then calls the
  out-of-line copy. Game::TestScriptFlag (0x100022b0, now inline in game.h) is expanded 3 times and called once in
  ForEachActiveTrigger (0x100552f0), expanded twice and called twice in RaiseGameOverEvents (0x10054a60). A known
  function called "sometimes" after identical inline code is an inline helper.
- Two short constant loops in a row get **fused** by cl 13.00 (stores interleaved); written-out stores keep the
  original order and let the compiler keep the constant in a register for the earlier loops too (EffectManager::Reset
  0x10184170, effect definitions 0x1016f900: the two five-entry -1 caches).
- A clamp that tests the upper bound twice (cmp 15 / test / cmp 15 again) is the classic `max(min(x, hi), lo)` macro
  pair with double evaluation; write the macros inline in the expression (0x10017fc0). `add reg, -5` instead of
  `sub reg, 5` came from the subtraction inside the macro argument.
- Texture_Load (0x1001e1a0) is also an inline helper: functions that load a fixed texture expand it, including
  three `strlen` loops over the literal (0x10050460). `include/texture_load.h` holds it as `LoadTexture`.
- **Jump threading** of character tests: with `char c = *text;` cl 13.00 knows after `c == '('` that `c != '"'`
  and jumps past the later tests (blocks moved, about 15 bytes longer). Writing `*text == ...` in every test leaves
  the threading out; the loads are still merged into one register later (ListBank::FindCase 0x100558d0,
  ListBank::FindLabel 0x10055aa0).

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
- Exception frames: with /GX (/EHsc) an `extern "C"` function gets no unwind frame for its locals with destructors.
  A function whose original has `push -1 / push handler / fs:[0]` must be a C++ function (Point_Transform 0x1001c510).
- Fully unrolled copies (28 byte stores, 5 dword stores) are written out in the source; cl 13.00 does not unroll a
  loop that long (DemoRecording::PlayFrame/RecordFrame 0x1015cf30/0x1015cd70).
- `x = (a + b) * c` with a store between the add and the multiply was two statements: `x = a + b; x *= c;`
  (Region_IsVisible 0x10021070). A float rounded to memory before fabs/atan2 means an inline float helper
  (AbsFloat in char_leash.cpp, Atan2 in enemy_moves.cpp), whose parameters are spilled.
- An `else` block placed first (right after a test) usually comes from else-if chains with one shared `return` at
  the end rather than early returns (ListBank::Evaluate 0x100caf90).
- A function that never uses `this` but is called with ecx set is a member function (TextureManager::OnLostDevice
  = Render_OnLostDevice); one called without ecx set from a method is a free or static function.
- Pass-through field compares: `0.0f < x ? x : 0` and `1.0f > a ? a : 1.0f` keep the original operand order of the
  compare; the mirrored spelling flips the fcomp operands.
- One helper applied twice to the same expression (`SignedField(code % 10000)` in a test, then again in the use):
  write the expression twice. CSE shares the idiv, the first inline call works on a copy and the second folds the
  dead remainder in place. A named `cost` local gives lea forms instead (Map_RouteLength 0x10049990).
- A pointer local into a table (`RoutePoint *point = &table[i]`) changes the order of the induction increments at
  the loop end; indexing the table directly can restore it (Map_RouteLength).
- `x = total + f(); return x;` against `total += f(); return total;`: the second rounds through total's own stack
  slot after the register pops.
- Arrays read at `index + 3` with scaled addressing (shorts, ints, 0x188 records) keep `index + 3` in a register only
  when it is a named local; byte arrays fold it into the displacement either way (Char_ClampStatus 0x1001a840).
- Tokenizer loops (ListBank::ReadWord and siblings 0x10035b10-0x10035fa0): test the characters through `*p` every
  time, not through a `char c = *p` local; with the local cl 13.00 threads jumps the original does not. The length
  limit is the loop condition at the bottom (`do { if (error) return 0; ... } while (length < 0x100);`), and each
  exit keeps its own copy of the terminating stores and `return 1`.
- A range test compiled as `fcomp 1.0; test ah,0x41; je skip` then `fcomp 0.9; test ah,5; jnp skip` is the negated
  form `if (!(z > 1.0 || z < 0.9))`; `z <= 1.0 && z >= 0.9` (in either operand order) gives `jp`/`test ah,1`
  (Drops_Update 0x1000c390). A float argument computed into a register and pushed as an int (`mov edx,[tmp]; push
  edx`) was a named local (`float alpha = ...;`) rather than an expression in the call.
- Two branches ending in the same call can be one shared call in the original's code because the compiler merged
  them: write the call in each branch (`if (a) { s = 1; Play(); } else if (b) { s = 0; Play(); }`), not an early
  `return` plus one call at the end; the merged tail schedules `this` differently (Npc_TalkFacing 0x1001c210).
- Several full calls in an if/else chain that differ in one argument compile to shared pushes before the branch,
  one differing push per branch and one call (cross-jumping); a conditional expression in the argument gives a
  select into a register instead (Char_SetHandTool 0x10012510, Screenshot_ReadBackBuffer 0x1013e9d0). A global read
  in several conditions of the chain loaded early into a register was a local (`int level = DAT_10c64ad4;`).
- A mode that is first resolved (`if (mode == -1) { if (...) mode = 0; else mode = 1; }`) and then dispatched lets
  the compiler thread the jumps straight into the attach/remove blocks; a conditional expression gives setcc.
- A /GS cookie (`push ecx` frame, cookie check before every return) in a function without any buffer means the
  original declared a character array it never used; an unused `char unused[8];` reproduces it
  (Helmet_FaceSettings 0x10014d30, commented in the source).
- A search loop whose found-branch's work sits at the end of the function and jumps back to a shared tail, while
  the not-found work falls through after the loop, was written with the whole tail (`...; return slot;`) inside the
  found-branch and again after the loop; the compiler cross-jumps the duplicate tail (Node::Load 0x101ac010).
- A destination computed before a branch that only picks the source (`mov edi,dest; test; je; mov esi,a / lea
  esi,b; rep movsd`) was an if/else with one full assignment per branch; the compiler hoists the common part. The
  conditional-expression form computes the source first (Node::SetupSkins 0x101ab9d0).
- Arguments pushed before a branch that only computes one argument (the sprite alpha in EffectManager::DrawSpriteA
  0x101890f0) come from one full call per if/else branch: the compiler hoists the shared pushes above the branch
  and merges the tails. A conditional expression as the argument is evaluated before all pushes instead.
- Bits 0-7 of a word stored as bytes with `mov al,dl; shr al,n; and al,1` (byte shifts) were read from the low byte:
  `((unsigned char)bits >> n) & 1`; `(bits >> n) & 1` shifts the whole register (InputState::SetButtons 0x1015cae0).
- A `<=`/`>=` test that lets NaN through (`test ah,0x41; jnp` / `test ah,1; je`) is the negation of the opposite
  comparison, written as an early exit `if (a <= b || c >= d) return 0;` (Box_Overlap 0x10191fd0).
- Two Vec4 locals sharing one stack slot (a frame 16 bytes smaller than ours) were in disjoint scopes: the copy
  used only to measure a distance sat in its own block (Char_CanSee 0x10043e20).
- A call with ecx loaded from a global object pointer and no `add esp` afterwards is a method of that object
  (thiscall), even when the pointer was only tested before: `DAT_10255f08->Raycast(...)`.
- Locals that overlap the stack slots of temporaries from an earlier block (Vec4 locals at offsets the original
  also uses for the smoothing temporaries) were declared in their own `{ }` scope after that block; function-scope
  locals declared later still get slots of their own (Sound_UpdateListener 0x10021c80).
- A clamp computed twice (`test; setl; dec; and` before and after the compare) is a `__max` expression repeated
  in a conditional: `__max(0, t - 5) > limit ? limit : __max(0, t - 5)` (Item_DrawCaption 0x1003c370).
  `__max(0, x)` gives `setl`, `__max(x, 0)` gives `setle`.
- `strlen(s) / 2` divides unsigned (`shr`); the original's `cdq; sub; sar` is `(int)strlen(s) / 2`.
- Range checks written as two statements (`if (i < 0) return 0; if (i > max) return 0;`) keep the shared
  `return 0` block inline right after the tests; `if (i < 0 || i > max) return 0;` moves it to the end of the
  function (Bgm_Play 0x1003aef0).
- An if/else whose then-branch is the common path (`if (attacking == 1) {...} else {...}`) puts the else-branch at
  the end; the same logic as early returns lays the blocks out the other way (Kind4a_Update 0x1009ddd0).
- A reloaded-pointer copy `T *p = param;` made before a loop moves the parameter loads above the first test
  (Rle_Decode 0x101612b0); a table indexed `[group * 256 + word]` should be declared `[groups][256]` when the
  original steps a row pointer (Text_Substitute 0x1000c020).
- An `inline` helper with a call and three stores was not inlined at /Ob1 here; duplicated statements matched.
- `cmp eax, ebx` (the zero register) after a call where cl 13.00 gives `test eax, eax`: the result went into a
  local first (`void *opened = file.Open(...); if (opened == 0)`) (MmvPlayer::Open 0x10160a90).
- A palette built with `movzx reg16` and `and reg, 0xfff8` works on unsigned shorts:
  `(unsigned short)(unsigned char)r & ~7` (MmvPlayer::Open).
- Reads from a buffer with a moving cursor whose position is also counted in an address-taken int (`p += n;
  offset += n;` per field) explain a run of stores to that int between the copies (Save_Load 0x1003b2c0).
- A big sparse `switch` lays out its case blocks in source order; recover that order from the original's block
  addresses (Map_SetEntryFlags 0x10021380).
- cl 13.00 turns plain fill loops into `rep stosd`, packing shorts into dwords (`for (i < 0x44c) a[i] = -11` gives
  `mov eax, 0xfff5fff5; mov ecx, 0x226; rep stosd`), so a fill of a value memset cannot make is still a loop
  (Game_Reset 0x1003e710).
- Ghidra drops `this` for thiscall callees whose ecx is set well before the call (`mov ecx, 0x104687b8` six
  stores earlier): an unexplained early `mov ecx, imm` is the object of the next call.
- Long store-only functions can be generated from the pseudocode (store widths from the asm, `Fill` targets from
  the `mov ecx` before each call) and then fixed by comparing instruction offsets to find the first divergence.
- A local `char table[256][4] = { "..", ... }` compiles to a word load and a byte load from each literal plus a
  zero store for the 4th byte (no block copy); replay the stores against the image to recover the strings
  (Text_ToFullWidth 0x1015d520). Test order matters: `(c >= 0x7f && c < 0xa0) || c >= 0xe0` is not the same layout
  as its negation with the branches swapped.
- `(flags & 0x80) == 0x80` on an unsigned value compiles to `test al, al / jns`; on an `int` it stays
  `and ecx, 0x80 / cmp cl, 0x80` (Ability_GetDescription 0x100144c0, whose parameter is therefore `int`).
- A loop that clears two arrays of the same length (`for (i < 0x42) { a[i] = 0; ...rest... }`) is split by cl:
  the plain clear becomes a `rep stosd` scheduled into the code before the loop, which changes the loop's registers;
  a separate `memset` before the loop does not give the same code (Map_ResetOnEnter 0x10023900).
- Small local char buffers: the declared size changes the frame layout. `char number[8]` (only five bytes used)
  matched WorldRenderer::LoadCharacter where `[5]`/`[6]` put it below another local.
- Structures with many member arrays (WorldRenderer, 0x1410d0 bytes) are easiest to lay out with a generator that
  takes (offset, size, declaration) rows and emits the padding; a constructor that constructs the members in order
  checks the layout (world_ctor.cpp).
- Texture loads by literal name (`FUN_10174ce0(FUN_1013f8a0(name, 0), 0, 0)`) come from a one-line inline helper
  (`LoadIti`, face_anim.cpp / bs003_variant.cpp); the full `LoadTexture` of texture_load.h is not inlined at /Ob1.
  Repeated loads at the end of switch cases are cross-jumped into one call, so write each case in full.
- Locals that live only in one block should be declared in that block: cl reuses their stack slot for later
  spills (SystemSav_ReadRecord 0x101603c0 matched its frame once `zero`/`word` moved into their loops).
- Three FILETIMEs in ascending address order with an 8-byte gap after the last-write time are an array
  `FILETIME times[4]` used as times[0..2] (SystemSav_WriteRecord 0x1015fea0); separate variables are ordered differently.
- Copying a byte buffer once per loop iteration into an `int c` local gives the original's movsx/xor/add sequence
  in cl's 8x-unrolled checksum loops; two reads of `name[i]` do not.
- Comparison operand order is visible: `count > to` gives `cmp eax(count), reg / jle`, `to < count` gives
  `cmp reg, [count] / jge` (Node::BlendFrames 0x1019b630). Matrices passed by value are copied with `rep movsd`
  onto the stack in argument order.
- Reading the three triangle indices into locals once, and evaluating `MAX(...) + MIN(...)` in that order, matched
  the original's load and compare sequence in Node::BuildCollisionSpheres 0x1019e120 (MIN/MAX macros evaluate twice).
- A setter that loads the stored value before the index (`mov eax, [esp+4]; mov edx, [esp+8]`) when its twin loads
  the index first stores a float: float copies go through integer registers in argument order (0x1015cac0).
- Walls written down as "VS2003 does X" deserve a retry with cl 13.00: statement order of independent stores
  does change the output (0x10034c80 matched with the two stores swapped, 0x10166870 with `m_00b8 = 0` first),
  and a call that keeps `this` in ecx is a method call (MeshInstance::M_10169e10 calls Upload, not a free function).
- `int left = counter; if (left != 0) counter = left - 1;` gives `dec eax / mov [counter], eax` where
  `counter--` copies to another register first (0x1000ed20). A clamp written as two ternaries through a second
  variable (`int c = x > 0 ? x : 0; int slot = c > 3 ? 3 : c;`) gives the branchless low clamp and a
  `mov esi, 3 / cmp / jg / mov esi, eax` high clamp (0x1016b1e0).
