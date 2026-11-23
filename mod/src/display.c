// Resolution, Hor+ widescreen and a centred 4:3 HUD for Gurumin (JP PC, bin\APP.dll).
//
// What the game does (addresses in APP.dll, preferred base 0x10000000; see docs/DISPLAY_MOD.md):
//   - Game_Startup reads gurumin.def RESOLUTION into 0x10202b24/28 (width/height; 640x480, 800x600 or 1280x960),
//     then Render_CreateDevice calls Direct3DCreate9, sizes the window and fills the present parameters from them.
//   - The 3D projection is a square perspective times ScaleMatrix(1, 1.333, 1) (camera_setup.cpp): a fixed 4:3
//     aspect, built at six call sites that push 1.333 as Y scale and 1.0 as X scale.
//   - 2D drawing works in 640x480 coordinates scaled by the 2D canvas 0x10202b34/38 (set to the viewport size each
//     time the game sets a viewport) and draws pre-transformed (XYZRHW) vertices with DrawPrimitiveUP.
//   - World positions are put on screen with Project 0x101ae060 (to -1..1), then x*320+320.
//
// What this file changes:
//   - writes the configured resolution into 0x10202b24/28 when the game calls Direct3DCreate9;
//   - Hor+: the X scale pushed next to 1.333 becomes 1.333 / (width / height), so wider screens see more at the
//     sides with the same vertical view;
//   - centred HUD: while the viewport is the whole back buffer, the 2D canvas is pinned to a 4:3 width and every
//     pre-transformed vertex is moved right by the side gap (quads that cover the whole canvas, such as fades and
//     screen copies, are stretched to the full width instead); world-to-screen projections of 15 known functions
//     are widened by width / 4:3 width so markers, prompts and particles stay on their objects.
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <stdarg.h>
#include "display.h"

#define APP_TIMESTAMP 0x44604392u
#define APP_IMAGE_SIZE 0x1b5c000u
#define APP_PREFERRED_BASE 0x10000000u

#define VA_WIDTH 0x10202b24u
#define VA_HEIGHT 0x10202b28u
#define VA_CANVAS_WIDTH 0x10202b34u
#define VA_CANVAS_HEIGHT 0x10202b38u
#define VA_EFFECT_TEXTURE_SIZE 0x10f654ecu   // 256/512/1024 from RESOLUTION; size of the screen-copy texture
#define VA_PROJECT 0x101ae060u

// The six ScaleMatrix(1.0, 1.333, 1.0) calls (address of the `push 1.333` instruction).
static const DWORD projection_sites[] = {0x1000bc96, 0x10037e5c, 0x1013cfd1, 0x1013d06e, 0x1013d161, 0x1019439f};

// Functions that turn a Project() result into 640x480 screen coordinates (x*320+320): start, size in bytes.
static const DWORD screen_projection_functions[][2] = {
    {0x1000c390, 540},   {0x1000cfa1, 1656},  {0x1000d6b0, 666},   {0x1000e340, 1035},  {0x10038c10, 6818},
    {0x1008c680, 14341}, {0x1011a060, 14244}, {0x10162f50, 173},   {0x10165140, 202},   {0x10167d90, 2599},
    {0x1016fb80, 2545},  {0x10170580, 3679},  {0x101713f0, 3674},  {0x10187520, 3712},  {0x101890f0, 579},
    {0x10189530, 30969},
};

static HINSTANCE this_module;
static FILE *log_file;
static BYTE *app_base;

static struct
{
    int enabled, width, height, hor_plus, centre_hud, stretch_fullscreen, effect_texture, log, hd_movies;
} settings;

// Derived sizes, valid once display_before_direct3d has run.
static int screen_width, screen_height;
static int hud_width;          // 4:3 width of the centred HUD (= screen_width when the screen is 4:3 or narrower)
static float hud_gap;          // left offset of the centred HUD
static float world_widen;      // screen_width / hud_width
static int hud_active;         // 2D is being centred
static unsigned long moved_draws, stretched_draws, untouched_draws;

static void log_line(const char *format, ...)
{
    if (!log_file)
        return;
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fputc('\n', log_file);
    fflush(log_file);
}

static void path_next_to_module(const char *name, char *out)
{
    GetModuleFileNameA(this_module, out, MAX_PATH);
    char *slash = strrchr(out, '\\');
    strcpy(slash ? slash + 1 : out, name);
}

void display_set_module(HINSTANCE instance)
{
    this_module = instance;
}

static void describe_address(void *address, char *out, size_t size)
{
    HMODULE module = NULL;
    char name[MAX_PATH] = "?";
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (const char *)address, &module))
    {
        GetModuleFileNameA(module, name, MAX_PATH);
        char *slash = strrchr(name, '\\');
        snprintf(out, size, "%s+%lx", slash ? slash + 1 : name, (unsigned long)((BYTE *)address - (BYTE *)module));
    }
    else
        snprintf(out, size, "%p", address);
}

// Logs the first access violations (the game may catch them and quit quietly).
static LONG CALLBACK log_exception(EXCEPTION_POINTERS *info)
{
    static int reported;
    DWORD code = info->ExceptionRecord->ExceptionCode;
    if ((code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_ILLEGAL_INSTRUCTION ||
         code == EXCEPTION_INT_DIVIDE_BY_ZERO) && reported < 8)
    {
        char where[MAX_PATH + 32];
        describe_address(info->ExceptionRecord->ExceptionAddress, where, sizeof where);
        log_line("exception %08lx at %s (data %p)", code, where,
                 info->ExceptionRecord->NumberParameters > 1 ? (void *)info->ExceptionRecord->ExceptionInformation[1]
                                                             : NULL);
        reported++;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void display_shutdown(void)
{
    if (log_file)
    {
        log_line("closed: %lu HUD draws moved, %lu stretched, %lu full-screen copies", moved_draws, stretched_draws, untouched_draws);
        fclose(log_file);
        log_file = NULL;
    }
}

static void read_settings(void)
{
    char ini[MAX_PATH], text[32];
    path_next_to_module("gurumin_mod.ini", ini);
    settings.enabled = GetPrivateProfileIntA("display", "enabled", 0, ini);
    settings.hd_movies = GetPrivateProfileIntA("movies", "hd", 1, ini);
    settings.width = GetPrivateProfileIntA("display", "width", 1280, ini);
    settings.height = GetPrivateProfileIntA("display", "height", 720, ini);
    settings.hor_plus = GetPrivateProfileIntA("display", "hor_plus", 1, ini);
    GetPrivateProfileStringA("display", "hud", "center", text, sizeof text, ini);
    settings.centre_hud = _stricmp(text, "stretch") != 0;
    settings.stretch_fullscreen = GetPrivateProfileIntA("display", "stretch_fullscreen", 1, ini);
    settings.effect_texture = GetPrivateProfileIntA("display", "effect_texture", 0, ini);
    settings.log = GetPrivateProfileIntA("mod", "log", 1, ini);
    if (settings.log)
    {
        char log_path[MAX_PATH];
        path_next_to_module("gurumin_mod.log", log_path);
        log_file = fopen(log_path, "w");
    }
}

static BYTE *app_address(DWORD va)
{
    return app_base + (va - APP_PREFERRED_BASE);
}

static int find_app(void)
{
    BYTE *base = (BYTE *)GetModuleHandleA("APP.dll");
    if (!base)
    {
        log_line("APP.dll is not loaded: nothing changed");
        return 0;
    }
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + ((IMAGE_DOS_HEADER *)base)->e_lfanew);
    if (nt->FileHeader.TimeDateStamp != APP_TIMESTAMP || nt->OptionalHeader.SizeOfImage != APP_IMAGE_SIZE)
    {
        log_line("APP.dll is not the known JP build (timestamp %08lx, size %lx): nothing changed",
                 nt->FileHeader.TimeDateStamp, nt->OptionalHeader.SizeOfImage);
        return 0;
    }
    app_base = base;
    log_line("APP.dll at %p", (void *)base);
    return 1;
}

static void write_code(void *address, const void *bytes, size_t size)
{
    DWORD old;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old);
    memcpy(address, bytes, size);
    VirtualProtect(address, size, old, &old);
    FlushInstructionCache(GetCurrentProcess(), address, size);
}

static void patch_projection_aspect(void)
{
    float x_scale = 1.333f / ((float)screen_width / (float)screen_height);
    static const BYTE push_y[5] = {0x68, 0xbe, 0x9f, 0xaa, 0x3f};   // push 1.333f
    static const BYTE push_one[5] = {0x68, 0x00, 0x00, 0x80, 0x3f}; // push 1.0f
    int patched = 0;
    for (size_t i = 0; i < sizeof projection_sites / sizeof projection_sites[0]; i++)
    {
        BYTE *site = app_address(projection_sites[i]);
        if (memcmp(site, push_y, 5) != 0)
            continue;
        for (int k = 5; k < 18; k++)          // after the Y push: a lea, then the X push
            if (memcmp(site + k, push_one, 5) == 0)
            {
                write_code(site + k + 1, &x_scale, 4);
                patched++;
                break;
            }
    }
    log_line("Hor+: projection X scale %.4f at %d of 6 sites", x_scale, patched);
}

// ---- world-to-screen projections ------------------------------------------------------------------------------

typedef void(__cdecl *project_fn)(float *out, const float *point, const float *matrix);
static project_fn game_project;

static void __cdecl project_widened(float *out, const float *point, const float *matrix)
{
    game_project(out, point, matrix);
    out[0] *= world_widen;
}

static void redirect_screen_projections(void)
{
    game_project = (project_fn)app_address(VA_PROJECT);
    int redirected = 0;
    for (size_t f = 0; f < sizeof screen_projection_functions / sizeof screen_projection_functions[0]; f++)
    {
        BYTE *start = app_address(screen_projection_functions[f][0]);
        BYTE *end = start + screen_projection_functions[f][1] - 5;
        for (BYTE *p = start; p <= end; p++)
        {
            if (*p != 0xe8 || p + 5 + *(int *)(p + 1) != (BYTE *)game_project)
                continue;
            int rel = (int)((BYTE *)project_widened - (p + 5));
            write_code(p + 1, &rel, 4);
            redirected++;
            p += 4;
        }
    }
    log_line("HUD: %d world-to-screen projections widened by %.4f", redirected, world_widen);
}

// ---- Direct3D hooks -------------------------------------------------------------------------------------------

typedef HRESULT(WINAPI *create_device_fn)(IDirect3D9 *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *,
                                          IDirect3DDevice9 **);
typedef HRESULT(WINAPI *set_viewport_fn)(IDirect3DDevice9 *, const D3DVIEWPORT9 *);
typedef HRESULT(WINAPI *draw_up_fn)(IDirect3DDevice9 *, D3DPRIMITIVETYPE, UINT, const void *, UINT);
typedef HRESULT(WINAPI *draw_indexed_up_fn)(IDirect3DDevice9 *, D3DPRIMITIVETYPE, UINT, UINT, UINT, const void *,
                                            D3DFORMAT, const void *, UINT);
typedef HRESULT(WINAPI *set_fvf_fn)(IDirect3DDevice9 *, DWORD);
typedef HRESULT(WINAPI *set_shader_fn)(IDirect3DDevice9 *, IDirect3DVertexShader9 *);
typedef HRESULT(WINAPI *set_declaration_fn)(IDirect3DDevice9 *, IDirect3DVertexDeclaration9 *);

static create_device_fn real_create_device;
static set_viewport_fn real_set_viewport;
static draw_up_fn real_draw_up;
static draw_indexed_up_fn real_draw_indexed_up;
static set_fvf_fn real_set_fvf;
static set_shader_fn real_set_shader;
static set_declaration_fn real_set_declaration;

static int full_viewport;      // the current viewport is the whole back buffer (HUD space)
static DWORD current_fvf;
static int using_fvf = 1;
static int vertex_shader_set;

static int pretransformed(void)
{
    return using_fvf && !vertex_shader_set && (current_fvf & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW;
}

static UINT vertex_count(D3DPRIMITIVETYPE type, UINT primitives)
{
    switch (type)
    {
    case D3DPT_LINELIST: return primitives * 2;
    case D3DPT_LINESTRIP: return primitives + 1;
    case D3DPT_TRIANGLELIST: return primitives * 3;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN: return primitives + 2;
    default: return primitives;
    }
}

// Copies `count` pre-transformed vertices and moves them into the centred HUD (or stretches a full-canvas quad).
static const void *centre_vertices(const void *vertices, UINT count, UINT stride)
{
    static BYTE *buffer;
    static size_t capacity;
    size_t size = (size_t)count * stride;
    if (size > capacity)
    {
        free(buffer);
        capacity = size * 2;
        buffer = malloc(capacity);
        if (!buffer)
        {
            capacity = 0;
            return vertices;
        }
    }
    float min_x = 1e30f, max_x = -1e30f, min_y = 1e30f, max_y = -1e30f;
    for (UINT i = 0; i < count; i++)
    {
        const float *v = (const float *)((const BYTE *)vertices + i * stride);
        if (v[0] < min_x) min_x = v[0];
        if (v[0] > max_x) max_x = v[0];
        if (v[1] < min_y) min_y = v[1];
        if (v[1] > max_y) max_y = v[1];
    }
    // Already as wide as the back buffer (a copy of a full-screen render target): leave it alone.
    if (min_x <= 1.0f && max_x >= screen_width - 1.0f)
    {
        untouched_draws++;
        return vertices;
    }
    // As wide as the 4:3 HUD (fades, cutscene letterbox bars, banners): stretch it to the full width.
    int full_canvas = settings.stretch_fullscreen && min_x <= 1.0f && max_x >= hud_width - 1.0f;
    memcpy(buffer, vertices, size);
    for (UINT i = 0; i < count; i++)
    {
        float *v = (float *)(buffer + i * stride);
        v[0] = full_canvas ? v[0] * world_widen : v[0] + hud_gap;
    }
    if (full_canvas)
        stretched_draws++;
    else
        moved_draws++;
    return buffer;
}

static HRESULT WINAPI hook_set_viewport(IDirect3DDevice9 *device, const D3DVIEWPORT9 *viewport)
{
    HRESULT result = real_set_viewport(device, viewport);
    full_viewport = viewport && viewport->X == 0 && viewport->Y == 0 && (int)viewport->Width == screen_width &&
                    (int)viewport->Height == screen_height;
    if (full_viewport && hud_active)
    {
        // The game has just set its 2D canvas to the viewport size: draw 2D in a 4:3 area instead.
        *(int *)app_address(VA_CANVAS_WIDTH) = hud_width;
        *(int *)app_address(VA_CANVAS_HEIGHT) = screen_height;
    }
    return result;
}

static HRESULT WINAPI hook_draw_up(IDirect3DDevice9 *device, D3DPRIMITIVETYPE type, UINT primitives,
                                   const void *vertices, UINT stride)
{
    if (hud_active && full_viewport && pretransformed() && vertices)
        vertices = centre_vertices(vertices, vertex_count(type, primitives), stride);
    return real_draw_up(device, type, primitives, vertices, stride);
}

static HRESULT WINAPI hook_draw_indexed_up(IDirect3DDevice9 *device, D3DPRIMITIVETYPE type, UINT min_index,
                                           UINT vertex_total, UINT primitives, const void *indices, D3DFORMAT format,
                                           const void *vertices, UINT stride)
{
    if (hud_active && full_viewport && pretransformed() && vertices)
    {
        // Only vertices min_index..min_index+vertex_total-1 are used; copy from 0 so the indexes stay valid.
        vertices = centre_vertices(vertices, min_index + vertex_total, stride);
    }
    return real_draw_indexed_up(device, type, min_index, vertex_total, primitives, indices, format, vertices, stride);
}

static HRESULT WINAPI hook_set_fvf(IDirect3DDevice9 *device, DWORD fvf)
{
    current_fvf = fvf;
    using_fvf = 1;
    return real_set_fvf(device, fvf);
}

static HRESULT WINAPI hook_set_shader(IDirect3DDevice9 *device, IDirect3DVertexShader9 *shader)
{
    vertex_shader_set = shader != NULL;
    return real_set_shader(device, shader);
}

static HRESULT WINAPI hook_set_declaration(IDirect3DDevice9 *device, IDirect3DVertexDeclaration9 *declaration)
{
    using_fvf = declaration == NULL;
    return real_set_declaration(device, declaration);
}

static void *swap_vtable_entry(void **vtable, int index, void *hook)
{
    void *original = vtable[index];
    if (original == hook)
        return NULL;
    DWORD old;
    VirtualProtect(&vtable[index], sizeof(void *), PAGE_READWRITE, &old);
    vtable[index] = hook;
    VirtualProtect(&vtable[index], sizeof(void *), old, &old);
    return original;
}

static void hook_device(IDirect3DDevice9 *device)
{
    void **vtable = *(void ***)device;
    if (vtable[47] == (void *)hook_set_viewport)
        return;
    real_set_viewport = (set_viewport_fn)swap_vtable_entry(vtable, 47, (void *)hook_set_viewport);
    real_draw_up = (draw_up_fn)swap_vtable_entry(vtable, 83, (void *)hook_draw_up);
    real_draw_indexed_up = (draw_indexed_up_fn)swap_vtable_entry(vtable, 84, (void *)hook_draw_indexed_up);
    real_set_declaration = (set_declaration_fn)swap_vtable_entry(vtable, 87, (void *)hook_set_declaration);
    real_set_fvf = (set_fvf_fn)swap_vtable_entry(vtable, 89, (void *)hook_set_fvf);
    real_set_shader = (set_shader_fn)swap_vtable_entry(vtable, 92, (void *)hook_set_shader);
    log_line("device hooks installed");
}

// The game sizes its window as 2*W minus the current client size, which only works when the window starts W wide.
static void size_window_client(HWND window, int width, int height)
{
    RECT rect = {0, 0, width, height};
    DWORD style = (DWORD)GetWindowLongA(window, GWL_STYLE), ex_style = (DWORD)GetWindowLongA(window, GWL_EXSTYLE);
    AdjustWindowRectEx(&rect, style, GetMenu(window) != NULL, ex_style);
    SetWindowPos(window, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    GetClientRect(window, &rect);
    log_line("window client %ldx%ld", rect.right, rect.bottom);
}

static HRESULT WINAPI hook_create_device(IDirect3D9 *d3d, UINT adapter, D3DDEVTYPE type, HWND window, DWORD flags,
                                         D3DPRESENT_PARAMETERS *parameters, IDirect3DDevice9 **device)
{
    if (parameters && parameters->Windowed)
        size_window_client(parameters->hDeviceWindow ? parameters->hDeviceWindow : window, screen_width,
                           screen_height);
    HRESULT result = real_create_device(d3d, adapter, type, window, flags, parameters, device);
    log_line("CreateDevice %ux%u windowed=%d flags=%lx -> %08lx", parameters ? parameters->BackBufferWidth : 0,
             parameters ? parameters->BackBufferHeight : 0, parameters ? parameters->Windowed : -1, flags, result);
    if (SUCCEEDED(result) && device && *device && hud_active)
        hook_device(*device);
    return result;
}

void display_hook_direct3d(void *d3d)
{
    if (!app_base || !settings.enabled)
        return;
    void **vtable = *(void ***)d3d;
    if (vtable[16] != (void *)hook_create_device)
        real_create_device = (create_device_fn)swap_vtable_entry(vtable, 16, (void *)hook_create_device);
}

// ---- startup --------------------------------------------------------------------------------------------------

static int next_power_of_two(int value)
{
    int size = 256;
    while (size < value && size < 8192)
        size *= 2;
    return size;
}

void display_before_direct3d(void)
{
    static int done;
    if (done)
        return;
    done = 1;
    read_settings();
    log_line("gurumin_mod 0.2.0");
    if (settings.log)
        AddVectoredExceptionHandler(1, log_exception);
    if (!find_app())
        return;
    if (settings.hd_movies)
        movies_install(app_base, log_line);
    if (!settings.enabled)
    {
        log_line("display: off");
        return;
    }

    screen_width = settings.width > 0 ? settings.width : GetSystemMetrics(SM_CXSCREEN);
    screen_height = settings.height > 0 ? settings.height : GetSystemMetrics(SM_CYSCREEN);
    int game_width = *(int *)app_address(VA_WIDTH), game_height = *(int *)app_address(VA_HEIGHT);
    *(int *)app_address(VA_WIDTH) = screen_width;
    *(int *)app_address(VA_HEIGHT) = screen_height;
    log_line("resolution %dx%d (gurumin.def asked for %dx%d)", screen_width, screen_height, game_width, game_height);

    int effect = settings.effect_texture > 0 ? settings.effect_texture
                                             : next_power_of_two(screen_width > screen_height ? screen_width
                                                                                              : screen_height);
    *(int *)app_address(VA_EFFECT_TEXTURE_SIZE) = effect;
    log_line("screen-copy texture %d", effect);

    if (settings.hor_plus)
        patch_projection_aspect();

    hud_width = screen_height * 4 / 3;
    if (hud_width > screen_width)
        hud_width = screen_width;
    hud_gap = (screen_width - hud_width) / 2.0f;
    world_widen = (float)screen_width / (float)hud_width;
    hud_active = settings.centre_hud && hud_width < screen_width;
    log_line("HUD %s: 4:3 width %d, gap %.1f", hud_active ? "centred" : "stretched", hud_width, hud_gap);
    if (hud_active)
        redirect_screen_projections();
}
