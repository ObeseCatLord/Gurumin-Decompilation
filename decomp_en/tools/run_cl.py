"""run_cl.py <program.exe> [args...] : run a 32-bit VS2013 tool (cl.exe) with a legacy ANSI code page.

Why: VS2013's compiler front end (c1.dll / c1xx.dll) stops with "fatal error C1001 ... p0io.c, line 2807" on every
file when Windows uses UTF-8 as the system code page (the "Beta: Use Unicode UTF-8" option, ACP 65001). VS .NET 2002
is not affected. This wrapper starts the program under the Win32 debug API, and when the 32-bit kernelbase.dll /
kernel32.dll load it overwrites GetACP and GetOEMCP with `mov eax, <code page>; ret`. Then it detaches and returns
the program's exit code. Nothing on disk is changed. Code page: GURUMIN_EN_ACP (default 932, Japanese).

On a machine whose system code page is not UTF-8 the wrapper is not needed (build.bat calls cl directly then).
"""
import ctypes, os, struct, subprocess, sys
from ctypes import wintypes as w

k32 = ctypes.WinDLL("kernel32", use_last_error=True)

DEBUG_ONLY_THIS_PROCESS = 0x2
CREATE_DEFAULT_ERROR_MODE = 0x04000000
EXCEPTION_DEBUG_EVENT, CREATE_PROCESS_DEBUG_EVENT, EXIT_PROCESS_DEBUG_EVENT, LOAD_DLL_DEBUG_EVENT = 1, 3, 5, 6
DBG_CONTINUE, DBG_EXCEPTION_NOT_HANDLED = 0x00010002, 0x80010001
STATUS_BREAKPOINT, STATUS_WX86_BREAKPOINT = 0x80000003, 0x4000001F
PATCHED_FUNCTIONS = ("GetACP", "GetOEMCP")


class STARTUPINFO(ctypes.Structure):
    _fields_ = [("cb", w.DWORD), ("lpReserved", w.LPWSTR), ("lpDesktop", w.LPWSTR), ("lpTitle", w.LPWSTR),
                ("dwX", w.DWORD), ("dwY", w.DWORD), ("dwXSize", w.DWORD), ("dwYSize", w.DWORD),
                ("dwXCountChars", w.DWORD), ("dwYCountChars", w.DWORD), ("dwFillAttribute", w.DWORD),
                ("dwFlags", w.DWORD), ("wShowWindow", w.WORD), ("cbReserved2", w.WORD),
                ("lpReserved2", ctypes.c_void_p), ("hStdInput", w.HANDLE), ("hStdOutput", w.HANDLE),
                ("hStdError", w.HANDLE)]


class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [("hProcess", w.HANDLE), ("hThread", w.HANDLE), ("dwProcessId", w.DWORD), ("dwThreadId", w.DWORD)]


class DEBUG_EVENT(ctypes.Structure):
    # the union is read by hand: the first fields of each member are all we need
    _fields_ = [("dwDebugEventCode", w.DWORD), ("dwProcessId", w.DWORD), ("dwThreadId", w.DWORD),
                ("pad", w.DWORD), ("u", ctypes.c_ubyte * 160)]


k32.WaitForDebugEvent.argtypes = [ctypes.POINTER(DEBUG_EVENT), w.DWORD]
k32.ContinueDebugEvent.argtypes = [w.DWORD, w.DWORD, w.DWORD]
k32.WriteProcessMemory.argtypes = [w.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p]
k32.GetFinalPathNameByHandleW.argtypes = [w.HANDLE, w.LPWSTR, w.DWORD, w.DWORD]
k32.CloseHandle.argtypes = [w.HANDLE]
k32.GetExitCodeProcess.argtypes = [w.HANDLE, ctypes.POINTER(w.DWORD)]
k32.FlushInstructionCache.argtypes = [w.HANDLE, ctypes.c_void_p, ctypes.c_size_t]


def export_rvas(path, names):
    """RVAs of the named exports of a PE file (forwarded exports are skipped)"""
    d = open(path, "rb").read()
    pe = struct.unpack_from("<I", d, 0x3C)[0]
    magic = struct.unpack_from("<H", d, pe + 24)[0]
    dirs = pe + 24 + (96 if magic == 0x10B else 112)
    exp_rva, exp_size = struct.unpack_from("<II", d, dirs)
    nsec = struct.unpack_from("<H", d, pe + 6)[0]
    opt = struct.unpack_from("<H", d, pe + 20)[0]
    sections = [struct.unpack_from("<IIII", d, pe + 24 + opt + 40 * i + 8) for i in range(nsec)]

    def off(rva):
        for vsize, va, rsize, raw in sections:
            if va <= rva < va + max(vsize, rsize):
                return raw + rva - va
        raise ValueError(rva)
    e = off(exp_rva)
    nfunc, nname, afunc, aname, aord = struct.unpack_from("<IIIII", d, e + 20)
    found = {}
    for i in range(nname):
        name_off = off(struct.unpack_from("<I", d, off(aname) + 4 * i)[0])
        name = d[name_off:d.index(b"\0", name_off)].decode()
        if name in names:
            ordinal = struct.unpack_from("<H", d, off(aord) + 2 * i)[0]
            rva = struct.unpack_from("<I", d, off(afunc) + 4 * ordinal)[0]
            if not exp_rva <= rva < exp_rva + exp_size:
                found[name] = rva
    return found


def image_path(handle):
    buf = ctypes.create_unicode_buffer(1024)
    if handle and k32.GetFinalPathNameByHandleW(handle, buf, 1024, 0):
        return buf.value.replace("\\\\?\\", "")
    return ""


def run(argv, code_page):
    patch = b"\xB8" + struct.pack("<I", code_page) + b"\xC3"  # mov eax, code_page ; ret
    si, pi = STARTUPINFO(), PROCESS_INFORMATION()
    si.cb = ctypes.sizeof(si)
    cmdline = subprocess.list2cmdline(argv)
    if not k32.CreateProcessW(None, ctypes.create_unicode_buffer(cmdline), None, None, True,
                              DEBUG_ONLY_THIS_PROCESS | CREATE_DEFAULT_ERROR_MODE, None, None,
                              ctypes.byref(si), ctypes.byref(pi)):
        sys.exit("run_cl: cannot start %s (error %d)" % (argv[0], ctypes.get_last_error()))
    k32.DebugSetProcessKillOnExit(False)
    event = DEBUG_EVENT()
    patched = set()
    while True:
        if not k32.WaitForDebugEvent(ctypes.byref(event), 0xFFFFFFFF):
            break
        status = DBG_CONTINUE
        code = event.dwDebugEventCode
        raw = bytes(event.u)
        if code == CREATE_PROCESS_DEBUG_EVENT:
            k32.CloseHandle(struct.unpack_from("<Q", raw, 0)[0])  # hFile
        elif code == LOAD_DLL_DEBUG_EVENT:
            hfile, base = struct.unpack_from("<QQ", raw, 0)
            path = image_path(hfile)
            k32.CloseHandle(hfile)
            low = path.lower()
            if "syswow64" in low and os.path.basename(low) in ("kernelbase.dll", "kernel32.dll"):
                for name, rva in export_rvas(path, PATCHED_FUNCTIONS).items():
                    k32.WriteProcessMemory(pi.hProcess, base + rva, patch, len(patch), None)
                    k32.FlushInstructionCache(pi.hProcess, base + rva, len(patch))
                    patched.add(name)
        elif code == EXCEPTION_DEBUG_EVENT:
            exc_code = struct.unpack_from("<I", raw, 0)[0]
            if exc_code not in (STATUS_BREAKPOINT, STATUS_WX86_BREAKPOINT):
                status = DBG_EXCEPTION_NOT_HANDLED
        elif code == EXIT_PROCESS_DEBUG_EVENT:
            k32.ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_CONTINUE)
            break
        k32.ContinueDebugEvent(event.dwProcessId, event.dwThreadId, status)
        if len(patched) == len(PATCHED_FUNCTIONS) and code == LOAD_DLL_DEBUG_EVENT:
            # both functions patched in kernelbase: stop debugging and let the program run freely
            k32.DebugActiveProcessStop(pi.dwProcessId)
            k32.WaitForSingleObject(pi.hProcess, 0xFFFFFFFF)
            break
    exit_code = w.DWORD(1)
    k32.GetExitCodeProcess(pi.hProcess, ctypes.byref(exit_code))
    k32.CloseHandle(pi.hThread)
    k32.CloseHandle(pi.hProcess)
    return exit_code.value


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    if k32.GetACP() != 65001:
        sys.exit(subprocess.call(sys.argv[1:]))
    sys.exit(run(sys.argv[1:], int(os.environ.get("GURUMIN_EN_ACP", "932"))))
