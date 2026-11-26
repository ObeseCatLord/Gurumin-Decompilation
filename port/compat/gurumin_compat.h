// Force-included in front of every decomp source compiled by the native port (gcc/clang: -include, MSVC: /FI).
// It supplies what Visual Studio .NET 2002 and the Win32 headers gave the original build, so that matched files
// from decomp/src compile unchanged on Linux x86_64 and aarch64. Nothing here changes the MSVC build of decomp/:
// that build never sees this directory.
#ifndef GURUMIN_PORT_COMPAT_H
#define GURUMIN_PORT_COMPAT_H

#if defined(_MSC_VER)

// Building with MSVC (x86 or x64): the keywords and types exist already.

#else

// Calling conventions. On non-x86 targets (and on x86_64) there is only one convention, so they are empty.
// On 32-bit x86 they keep their meaning.
#if defined(__i386__)
#define __cdecl __attribute__((cdecl))
#define __stdcall __attribute__((stdcall))
#define __thiscall __attribute__((thiscall))
#define __fastcall __attribute__((fastcall))
#else
#define __cdecl
#define __stdcall
#define __thiscall
#define __fastcall
#endif

#define __forceinline inline __attribute__((always_inline))
#define __declspec(x)

#endif

#if defined(__cplusplus)
#include <cstdint>
#else
#include <stdint.h>
#endif

// Win32 scalar types with their Win32 (ILP32/LLP64) sizes. LONG and DWORD are 32 bits there; on LP64 Linux
// `long` is 64 bits, so they are spelled with fixed-width types, never with `long`.
#if !defined(_WINDEF_) && !defined(_MSC_VER)
typedef int32_t BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef uint32_t UINT;
typedef int32_t INT;
typedef float FLOAT;
typedef char CHAR;
typedef void *HANDLE;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif

// _ftol: the CRT's float-to-int conversion the original compiler calls for (int)f. Plain C casts do the same
// truncation, so ported code only needs the name when a file calls it explicitly.
#if !defined(_MSC_VER)
static inline long long _ftol(double value)
{
    return (long long)value;
}
#endif

// The decomp assumes these sizes (MSVC on x86). Pointer size is NOT asserted: the port is 64-bit, and files that
// depend on 4-byte pointers (fixed-offset structs holding pointers, pointer/int casts) are kept out of the build.
#if defined(__cplusplus)
static_assert(sizeof(int) == 4, "the decomp assumes a 32-bit int");
static_assert(sizeof(short) == 2, "the decomp assumes a 16-bit short");
static_assert(sizeof(float) == 4, "the decomp assumes a 32-bit float");
static_assert((char)0x80 < 0, "compile with a signed char (-fsigned-char), as MSVC does");
#endif

#endif
