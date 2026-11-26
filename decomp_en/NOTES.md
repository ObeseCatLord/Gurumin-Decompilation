# Gurumin EN (Steam, 2015) matching decompilation — notes

Confirmed findings for the English Steam exe (`re/bin/game_en_steam.exe`, 6,029,312 bytes, linker 12.0, timestamp
0x553e3cc7 = 2015-04-27, image base 0x400000, not relocated). The JP notes are in `../NOTES.md`; JP compiler habits do
**not** carry over (different compiler). Facts only; progress lives in `BATCHES.md` and git.

## Toolchain (2026-11-24)
- Rich header: game objects **41 C++ (prodid 0xe1) + 67 C (0xe0) built with cl 18.00.31101** (VS2013 Update 4),
  linker 0xde 12.00.31101. No LTCG objects (0xe4/0xe5 absent), so every object compiles on its own.
  Library objects: 0xe0/0xe1/0xdf build 21005 (VS2013 RTM CRT/MFC), 0xcf build 50929 (323), 0x83/0x93 build 30729
  (VS2008 SP1), 0x9c/0x7b/0xab/0xc7 older import libraries.
- Install: `E:\Microsoft Visual Studio 12.0\VC\bin` (cl 18.00.31101.0, c1/c1xx/c2 18.00.31101.0, link 12.00.31101.0).
  `env.bat` calls `vcvars32.bat` (Windows Kit 8.1 headers); set `GURUMIN_VC12` to another install folder.
- **UTF-8 system code page breaks VS2013:** with Windows' "Beta: Use Unicode UTF-8" option (ACP 65001, set on the
  author's PC) every compile stops with `fatal error C1001 ... p0io.c, line 2807`, even `/E` of an empty file. VS .NET
  2002 is not affected. `tools/run_cl.py` starts cl under the Win32 debug API, overwrites `GetACP`/`GetOEMCP` in the
  32-bit kernelbase/kernel32 with `mov eax, 932; ret` when they load, detaches, and returns cl's exit code. It runs cl
  directly when the ACP is not 65001. `GURUMIN_EN_ACP` picks another code page.

## Compile flags: `/O2 /Oy- /GS- /EHsc` (x86 defaults: /arch:SSE2, /fp:precise)
Found by matching small EN functions (`flag_tests/`, built with `build.bat ..\flag_tests\zz_flags.cpp`):

| EN function | what it tests | result |
|---|---|---|
| 0x5a2da0, 0x5a24d0 (ring-buffer pushes, `cmovg`) | /O2, frame pointer | MATCH with /Oy-; with /Oy the `push ebp; mov ebp, esp` frame disappears (DIFF) |
| 0x5ac910 (timer loop) | loop codegen | MATCH |
| 0x5a8cd0 (thiscall clamp counter) | thiscall, leaf without frame | MATCH (leaf functions without stack arguments get no frame even with /Oy-) |
| 0x6e2150 (`(float)sin(x)`) | SSE2: `cvtps2pd`, call into the SSE2 libm, `fld` return | MATCH: /arch:SSE2 (the VS2013 default) |
| 0x7cd180 (float to clamped int: `cvttss2si`, `cmovs`) | SSE2 conversion | MATCH |
| 0x5a0fa0 (constructor with an EH frame: members with/without destructors) | /EHsc, EH state numbering | MATCH |

- **/GS-:** no game function loads the security cookie (`xor eax, ebp` after `mov eax, [___security_cookie]`), even
  with 0x154-byte frames (0x5a5500); 928 library functions do. The default /GS would add cookies.
- **/Oy-:** every game function with stack arguments or locals keeps `push ebp; mov ebp, esp` (frame-pointer
  addressing `[ebp+8]`), unlike JP's `[esp+4]`.
- **/MT:** CRT calls are direct `call` to the static CRT (no `__imp_`).
- Inlining is /O2's /Ob2: EN inlines more than JP (/Ob1): 12 JP functions were found inlined into their mapped
  caller, and EN functions are often larger than their JP twins. JP helper functions that do not exist in EN become
  `inline` functions in headers (helper recovery comes for free).
- EN EH prologues order the pushes differently from JP and between functions (`push -1; push handler; mov eax, fs:[0]`
  or `mov eax, fs:[0]; push -1; ...`): that is scheduling, not a flag.

## Game code vs library code (`tools/classify.py` -> `classify.tsv`)
24,656 functions in the exe. **Game: 2,034 functions, 2,080,360 bytes** (1,850 game + 184 EN-only launcher);
library: 22,607 (13,410 MFC, 8,515 CRT, 330 libjpeg, 334 libvorbis, 18 FID-named inside the game span); 14 thunks.
- 0x401000-0x59ffff: static MFC (Feature Pack; FID names, odd alignment, cookie prologues).
- 0x5a0000-0x7e1a3b: game span. Contains two third-party libraries compiled into the project with the same
  compiler (SSE2 code, no cookies): **IJG libjpeg 6b** 0x6f15b0-0x709c9f (`jpeg_CreateDecompress` checks version 62;
  "JPEGMEM"; only called from the Packs loader 0x7be0c0-0x7be690; in JP it lives in Packs.dll) and **libogg/libvorbis
  1.3.3** 0x7c6220-0x7e1a3b ("Xiph.Org libVorbis I 20120203"; JP: sound3d.dll). Both are excluded from the queue.
- 0x6e5d60-0x6f15af: the EN-only launcher/config dialog ("Gurumin Ver.1.4 (STEAM)", "Graphics / Audio", update
  server, key assignment warning); Falcom code on top of MFC. Game class, area `launcher`.
- 0x7e1a3c-end: CRT (`eh vector` iterators, string functions, SSE2 libm at 0x7e5060, ...).
- Packs (BMP/MMV/archive loading, 0x7a8000-0x7c0000) and SoundArc/sound3d (0x7b8000-0x7c6220) are compiled into the
  exe; in JP they were separate DLLs.

## JP -> EN map (`tools/map_jp_to_en.py` -> `jp_en_map.tsv`, `jp_en_coverage.txt`)
Features: referenced strings, float constants, integer immediates, library calls (imports and named CRT functions),
weighted by rarity; greedy one-to-one assignment, then call-graph propagation, then inlining detection. Score and
confidence levels are documented in the script.
- **1,143 of 2,034 EN game functions map to a JP twin** (1,833,307 of 2,080,360 bytes = 88 % of the code):
  883 high, 232 medium, 28 low; 795 by features, 348 by call graph; 1,004 from APP.dll (81 % of its 1,239 game
  functions), 111 from sound3d.dll, 25 from Packs.dll, 3 from SoundArc.dll.
- **657 EN functions have a matched JP twin** (344,136 EN bytes): the queue's first section.
- 891 EN functions (247,053 bytes) have no JP twin: 164 launcher, 13 Steam (SteamAPI calls/strings), 714 unknown
  (template/inline instances, functions split differently, the EN text/resolution changes, JP twins the map missed).
- Check against the older string-only map `docs/jp_en_map_full.tsv` (`tools/check_map.py`): 120 agree, 2 disagree
  (both call-graph guesses in the old map), 2 unmapped here.
- The JP Ghidra project has only 143 non-default signatures (types live in the decomp sources), so
  `ghidra/ApplyJpNames.java` mostly carries names (1,115 at level >= medium in a dry run) and plate comments.

## Porting JP source (first matches)
- `Node::Find` 0x7532c0 (JP 0x1019ece0, `src/scene_node.cpp`): the JP source compiled unchanged with VS2013 and
  matched; only the frame differs in the bytes (ebp frame), which the flags produce.
- `Map_RunFrameScript` 0x5dc5c0 (JP 0x10052480, `src/map_script.cpp`): the JP switch matched unchanged (jump table
  included in the 68 compared bytes); callee and global names carried over from the JP twins.
- Name C-linkage functions readably (`Map_RunFrameScript`, with a `_Map_RunFrameScript` row in `obj_symbols.tsv`),
  not `FUN_<JP address>`: `compare_obj.py` reads an address out of `FUN_xxxxxxxx` names, so a JP address there
  would compare against the wrong EN bytes.

## EN headers
`decomp_en/include/` holds the EN declarations; each type is declared once there. A JP header may be included from
the EN tree only once its layout is proven identical in EN (a matched EN function that uses its fields at the JP
offsets); record such headers here. None so far: `Node` is declared again in EN with only the methods used so far.

## Porting habits found in batches 1-2 (2026-11-24)
- **Map labels are often off by a neighbour** for tiny methods (same size, same callees): Save/Restore pairs swapped
  (Node flags and render mode), `Vec4::Distance` mapped to the default constructor, `Normalized3` to `Copy`,
  `Ime_IsOpen`/`TextInput_End` shorter in EN. Name every function by what the EN bytes do.
- Several JP `void` functions return `int 0` in EN (`xor eax, eax` before `ret`): `RenderQueue_Clear`,
  `AnimModel::ClearTrackB/C`, `ClearMotionKeys`. Declare them `int` and `return 0`.
- EN sums squares as `dy*dy + dx*dx + dz*dz` (JP source: `dz*dz + dy*dy + dx*dx`); JP helper calls that EN inlines
  can change scheduling, so write the arithmetic directly when the helper does not exist in EN.
- Tail-recursive JP loops: `Node::GetLinkedRoot` matches only written recursively (the compiler makes the loop).
  `Node::GetLastFrame` re-reads `childCount` in the loop condition (no hoisted local).
- File-local `static` helpers survive when the JP twin had them (`IndexLists_Free` in `FreeExtraIndexLists`).
- Node layout: the EN offsets of every field used so far equal the JP offsets (sizeof 0x18a0 from the effect-node
  pool stride); `decomp_en/include/node.h` carries the JP names.
- JP `memset` of small arrays often compiles to calls in EN while the original has inline (sometimes `movdqu`
  vectorised) stores: write the clear as a plain loop (`ModelBank::Reset`, `BufferPool_Clear`).
- EN often reorders stores relative to the JP source (pointer before count, reset fields first); follow the EN
  disassembly order. Separate early returns can matter where JP used one `||` test (`TexCache_Free`).
- Name constants by their compare value: `SCRIPT_FLAG_LAST = 3999` with `>`, because `>= 4000` changes the
  immediate.

## Launcher, Steam, sound engine (batches 8-10)
- **Launcher (MFC), 2026-11-25:** VS2013 Community Update 4 ships the MFC headers (`VC\atlmfc\include`, put on
  INCLUDE by `vcvars32.bat`), so the Falcom dialog classes of 0x6e5d60-0x6f15af are written against the real
  `afxwin.h`/`afxcmn.h`. They live in `include/launcher_dialog.h`, the only header that includes MFC (so no shared game
  header sees it), and one source file per dialog class (`src/launcher_*.cpp`, as in the original: a dialog's
  destructor is an out-of-line call from the page that opens it, which only happens across translation units).
  - **Defines:** none beyond the build's `/MT /D WIN32 /D NDEBUG /D _WINDOWS`. The exe is static MFC, ANSI (no
    `_UNICODE`, no `_MBCS`: `SendMessageA`, `char` strings); `_AFXDLL` must stay undefined. No `build.bat` change.
  - **CDialog layout:** the exe's MFC is 12.0.21005 (the MBCS libraries): `sizeof(CDialog)` is 0x94 and every Falcom
    dialog's first member is at 0x94. Update 4's `afxwin.h` adds `BOOL m_bClosedByEndDialog` (CDialog is 0x98). The
    header does `#define m_bClosedByEndDialog :0` around the includes (an unnamed zero-width bit-field) and
    `static_assert`s `sizeof(CDialog) == 0x94`. Control wrappers (CButton, CComboBox, CEdit, CSliderCtrl, CTabCtrl,
    CProgressCtrl) are 0x74 bytes and match as they are.
  - **Habits found:** (1) ClassWizard style: `IMPLEMENT_DYNAMIC` only on the classes with a `GetRuntimeClass` in the
    vtable (CAdvancedOptionDlg, CInstDlg, CKbDlg, CTitDlg, CCongigDlg, CTabCtrl2; not CVideoDlg, CNetDlg, COseDlg,
    CPadDlg). (2) An *explicit* empty `~Dlg() {}` stores the class's vtable pointer first (CAdvancedOptionDlg,
    CInstDlg, CKbDlg, CTitDlg match that way); an implicit destructor does not (CNetDlg, COseDlg, CVideoDlg, CPadDlg,
    CCongigDlg). (3) A destructor defined in the same file as its caller is inlined: `CVideoDlg::OnAdvancedOption`
    only matches with `CAdvancedOptionDlg` in another file. (4) Exchanged ints are zeroed in the constructor's
    member-initialiser list (interleaved with the control constructors in declaration order) or in the body (`-1`
    for "none"); an exchanged `CString` is `m_x = ""` in the body for CNetDlg but `m_x("")` in the list for CPadDlg.
    (5) The `int ids[12] = {...}` locals of the key-assignment checks compile to the original's SSE2 constant
    sequences unchanged. (6) `ON_WM_CTLCOLOR` handlers are `CDialog::OnCtlColor` + `SetBkMode(TRANSPARENT)`;
    `OnPaint` that only builds a `CPaintDC` matches as written. (7) Locals are declared at point of use
    (`CRect`/`RECT` initialisers vectorise where they stand). (8) `CREATESTRUCT::lpszClass` (not `lpszName`) is what
    `CCongigDlg::OnCreate` sets to "Falcom".
  - **MFC/ATL header instantiations:** the object files carry the CString/ATL/CWnd inline instantiations that cl
    emits from the headers; 28 of them are byte-identical to launcher-range functions (rows in `obj_symbols.tsv`
    under "MFC/ATL header instantiations"). They are library code, not Falcom source.
  - Not reconstructed yet: the launcher entry point 0x6e7470 and the ini loader 0x6e81d0 (large, global-heavy),
    `CCongigDlg::OnInitDialog` 0x6e9410, the Othello game logic (0x6ec360, 0x6ecb50, 0x6ecf40 and the mouse/paint
    handlers; the board pack loops need a hand-unrolled spelling, open.tsv), `CVideoDlg`'s preset handler 0x6f1340, the
    pad page's ~40 "Set" button handlers (0x6edf80-0x6ef760, missing from classify.tsv).
- Steam: no Steamworks SDK headers; `include/steam.h` declares only the interface slots used. Some slot calls compile
  as `call [edx+N]` where the original has `mov eax,[edx+N]; call eax` (open.tsv).
- Sound engine (JP sound3d unmatched) ported from Ghidra + EN bytes; MmioFile methods are header inlines whose
  standalone copies appear once out-of-line callers exist (no forcing tables). The Ogg reader functions hit register
  allocation walls (open.tsv).
- Map labels in the sound area name MFC/CRT FID functions for EN game code (DeleteValues, __strupr, ...): ignore them.

## Open: CreateShortcut calling convention (batch 12)
EN 0x7a9db0 matches only as a free `__stdcall` function (`src/shortcut.cpp`); as an `InstallerMenu` member it spills
`this` (frame 0x618 instead of 0x614). Its caller 0x7aa130 (`InstallerMenu::CreateDesktopShortcut`) loads ecx with
`this` before the call and matches only when the callee is declared as a thiscall member (`include/installer_menu.h`).
So the function has two declarations for now; find one form both functions accept before treating either as final.
