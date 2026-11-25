# Native port scaffold (`port/`)

This is the start of a native (non-Windows) build of Gurumin, built for **Linux x64 and Linux arm64**. It grows
with the decompilation: each matched file in `decomp/src` that is also portable gets compiled here unchanged and
exercised by native tests.

**It is not playable.** The JP decompilation covers about 20% of APP.dll's code (EN about 7%), and the parts that
make a game run (window, Direct3D renderer, DirectInput, sound, scene and script loop) are still Windows code or
not decompiled yet. The scaffold is the place where matched code is compiled portably and proven to run on other
CPUs, so the port can grow one file at a time.

## What is in it

| Path | Contents |
|---|---|
| `CMakeLists.txt` | `gurumin_core` (decomp files), `gurumin_formats` (port's own readers), tests, `gurumin_port` shell |
| `compat/gurumin_compat.h` | force-included before every decomp file: calling-convention keywords (empty on x86_64/aarch64), Win32 types with Win32 sizes (`DWORD`/`LONG` are 32-bit), `_ftol`, size checks |
| `compat/windows.h` | stand-in `<windows.h>` with only those types |
| `compat/engine_memory.cpp` | storage for the engine's allocator pointers (`engine_memory.h`), pointed at malloc/free/memcpy |
| `compat/standins.cpp` | **port stand-ins**: native versions of four functions the compiled files call but the JP decomp does not define yet (see below) |
| `formats/` | port code (not decomp code): `.itm` trailer reader from `docs/ITM_FORMAT.md`, case-insensitive game-folder paths |
| `tests/core_tests.cpp` | 160 checks of decomp functions against hand-computed values |
| `tests/formats_tests.cpp` | checks of the port's readers on synthetic files |
| `app/main.cpp` | `gurumin_port`, a minimal SDL2 shell |

The decomp sources and headers are only read. Nothing in `port/` changes the MSVC build of `decomp/` or `decomp_en/`.

## Build

Needs CMake 3.16+, a C++17 compiler (gcc or clang; MSVC is intended to work but has not been tried, and note that `compat/` is a
public include directory, so its `windows.h` would shadow the real one there) and, for the shell, SDL2
(`libsdl2-dev` on Debian/Ubuntu, `sdl2` or `sdl2-compat` on Arch). Without SDL2 the shell is skipped and the library
and tests still build.

```sh
cmake -S port -B build/port -G Ninja      # -G Ninja is optional
cmake --build build/port
ctest --test-dir build/port --output-on-failure
```

Run the shell against your own installed game (it only reads the folder):

```sh
build/port/gurumin_port --help
build/port/gurumin_port --probe                         # headless self-test of the compiled decomp code
build/port/gurumin_port --probe --game "/path/to/Gurumin"   # plus a report of the game folder
build/port/gurumin_port --game "/path/to/Gurumin"       # status window; Escape quits
```

The probe reads `bin/map.itm` (its `.itm` trailer, and its check values through the decomp's own
`File_Checksum`, `file_check.cpp`), and summarises `3ddata/`, `wav/`, `bgm/` and `bin/`. The window shows one bar
per check (green ok, red failed); the details are printed to the terminal. `--probe` exits 0 when the self-test
passes, whether or not a game folder was given, so CI can run it without game files.

**Linux x64 and arm64.** GitHub Actions (`.github/workflows/port.yml`) builds and tests on `ubuntu-latest` (x64) and
`ubuntu-24.04-arm` (arm64) and uploads `gurumin-port-linux-x64` and `gurumin-port-linux-arm64`. The arm64 build is
the one for aarch64 handhelds such as the Steam Frame, which runs SteamOS on arm64. The Steam Deck is x86_64 and uses
the x64 build. (Both run the shell only; there is no game to run yet.)

## Which decomp files are compiled

Match figures are from `progress/jp/report.json` (bytes matched / functions matched). The tests assert only on
functions with a 100% entry there; functions not yet matched are compiled (they are the decomp's current drafts)
but not asserted.

| File | Matched | What it is |
|---|---|---|
| `vec4.cpp` | 47% bytes, 28/36 functions | Vec4 constructors and operators, segment and line distances, TransformCoord, Project, QuatFromMatrix, KeyBank and KeyTrack::Reserve, key sampling (drafts) |
| `vec_math.cpp` | 92%, 25/27 | lengths, distances, boxes, rotations, Cross, Normalize, quaternion multiply, Nlerp, Slerp, Matrix4 constructor, Translate, SetRotationZ |
| `vec_ops.cpp` | 100%, 2/2 | TriangleNormal, PlaneFromTriangle |
| `point_transform.cpp` | 100%, 1/1 | Point_Transform |
| `box_overlap.cpp` | 47%, 3/5 | box/cylinder overlap tests (Node methods that do not touch `this`) |
| `math_float.cpp` | (data) | out-of-line acosf/atan2f/... pointers |
| `containers.cpp` | 75%, 27/29 | vertex packs, index lists, KeyTrack/KeyBank, MotionList, Matrix4 SetIdentity/SetTranslation/... |
| `vertex.cpp` | 100%, 2/2 | Vertex setters |
| `rle_decode.cpp` | 100%, 1/1 | run-length unpacking of the game's bitmaps |
| `text_fullwidth.cpp` | 100%, 1/1 | Shift-JIS full-width conversion |
| `ability_text.cpp` | 100%, 1/1 | equipment ability descriptions |
| `file_check.cpp` | 100%, 2/2 | data-file check values and record reads (stdio) |
| `input_state.cpp` | 100%, 1/1 | button press/hold edges |
| `flag_list.cpp` | 100%, 1/1 | id registry |

### Port stand-ins

`compat/standins.cpp` defines four functions that matched code calls but that the JP decomp has not reconstructed
yet. They are hand-written, **not decompiled and not checked against APP.dll**, and tests that go through them are
in a separate "stand-in dependent" section:

- `Matrix4::~Matrix4` (empty; 0x1013eda0)
- `Matrix4::operator*` (row-vector matrix product; 0x101b0670, written after the EN build's reconstruction)
- `Vec4::Transform` (provisional: (x, y, z, w) times the matrix)
- `KeyTrack::AddKey` (append a key)

They are weak symbols on gcc/clang. Delete each one when the decomp defines it.

### Compiler settings that matter

- `-fsigned-char`: MSVC's `char` is signed; aarch64's default is unsigned. Key flags, button bytes and Shift-JIS
  code depend on it (the compat header asserts it).
- `-fno-strict-aliasing`: the decomp reads 16-bit words through byte pointers (`rle_decode.cpp`,
  `text_fullwidth.cpp`).
- `-ffp-contract=off`: no fused multiply-add, so x64 and arm64 compute the same floats.
- `vec4.cpp` takes the address of temporaries (`Vec3_LengthSquared(&(a - *this))`), an MSVC extension; the
  temporary lives to the end of the full expression, so the code is correct. That one file is built with
  `-fpermissive` (gcc) or `-Wno-address-of-temporary` (clang).
- `-Werror=int-to-pointer-cast`: a file that casts between pointers and ints does not belong here.

### 64-bit notes

Headers document field offsets for the 32-bit original (`KeyTrack` "0x04: frames", `VertexPack` "0x70:
vertices", ...). Natively compiled code uses member names, so the different layout on 64-bit is harmless as long as
nothing copies such structs by a fixed size. The tests assert the fixed sizes the compiled code does rely on:
`sizeof(Vertex) == 0x70` (containers.cpp allocates `count * 0x70`), `sizeof(Vec4) == 16`, `sizeof(Matrix4) == 64`.

Other headers the build consumes with 32-bit layouts: `node.h` (pointer fields at documented 32-bit offsets; only
`box_overlap.cpp`'s methods that ignore `this` are used), `mesh.h` (`IndexList` is documented as 8 bytes, it is 16 on
64-bit), `flag_list.h` (`FlagOwner` pads 0x3eda08 bytes to stand in for a 32-bit game object). None of them is
copied by a fixed size in the compiled code.

Known issue in a compiled draft (not matched yet, not tested): `MotionList::FrameAt` reads an uninitialised
`length` when the list is empty (gcc warns).

Matched original behaviour worth knowing: `KeyTrack::Reserve` allocates with `new[]` and `KeyTrack::Free` releases
with `free()`, exactly as the original does. That is harmless with glibc and these trivially destructible element
types, but AddressSanitizer reports it.

## How the list grows

A file joins `GURUMIN_CORE_DECOMP_FILES` in `CMakeLists.txt` when it is **matched and portable**:

1. it has matched functions in `progress/jp/report.json` (prefer files whose functions are all matched);
2. it compiles here without edits (only `compat/` may change);
3. it does not depend on 32-bit pointers (pointer fields in fixed-size records copied by size, pointer/int casts),
   absolute game addresses, game data globals (`DAT_...` tables and state filled by the game), inline asm, Win32 or
   Direct3D;
4. everything it calls links, from the list or from a stand-in;
5. it gets tests for its matched functions in `tests/`.

Files left out for now, and why:

- **Game data globals:** `projection.cpp` (fog state in `camera_vars.h`), `path_format.cpp` (folder-name table and
  rotating buffers in `map_vars.h`, and it builds `\`-separated paths), `type_parse.cpp` (needs GameFile and
  ListBank), `texture_slots.cpp`, `render_state.cpp`, `screen_vertices.cpp`, `volume_fade.cpp`, `event_slots.cpp`,
  `pickups.cpp`, `inventory.cpp`, `slot_defaults.cpp`, `map_entry_flags.cpp`, `helmet_face.cpp`: they read or write
  engine state at fixed APP.dll addresses, which a port has to own first (a port-side globals module would let them in).
- **Not matched yet:** `circle_intersect.cpp`, `bmp_flip.cpp`, `image_halve.cpp`, `text_token.cpp`
  compile and link but have no matched function yet.
- **Large engine objects:** `app_0004.cpp`, `menu_items.cpp`, `demo_record.cpp`, `node_colours.cpp`, ... compile but
  operate on `Game`, `Node` or recorder objects whose layout is 32-bit and whose other methods are not portable yet.
- **Platform code:** anything including `d3d9_min.h`, `dinput_min.h`, `ddraw_min.h`, DirectShow or Win32 windows
  and files; `mmv_texreg.cpp` (inline asm).
- **Unresolved calls:** most other files call functions defined in files that are not portable yet.

The trial that picked this list compiled every `decomp/src` file permissively (`-fpermissive -w`, empty stub
`windows.h`/`mmsystem.h`/`mbctype.h`; about 175 of 465 got through that), then kept the ones whose undefined symbols
resolve inside the set, then built the survivors with the real flags above.
