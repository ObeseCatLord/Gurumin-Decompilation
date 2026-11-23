r"""capture_window.py - screenshot of a running game window (Windows, needs Pillow), for testing the display mod.

    python mod/tools/capture_window.py game.exe out.png [--wait SECONDS] [--key enter|esc|space ...]

Finds the largest visible top-level window of the named process, optionally waits, presses keys (the window is
brought to the front first), and saves its client area with PrintWindow (works for windows behind others too).
"""
import ctypes
import ctypes.wintypes as wt
import sys
import time

user32, gdi32, kernel32 = ctypes.windll.user32, ctypes.windll.gdi32, ctypes.windll.kernel32
psapi = ctypes.windll.psapi
VK = {"enter": 0x0D, "esc": 0x1B, "space": 0x20, "up": 0x26, "down": 0x28, "left": 0x25, "right": 0x27, "z": 0x5A,
      "x": 0x58}


def process_name(pid):
    handle = kernel32.OpenProcess(0x1000, False, pid)   # PROCESS_QUERY_LIMITED_INFORMATION
    if not handle:
        return ""
    buffer = ctypes.create_unicode_buffer(260)
    size = wt.DWORD(260)
    kernel32.QueryFullProcessImageNameW(handle, 0, buffer, ctypes.byref(size))
    kernel32.CloseHandle(handle)
    return buffer.value.rsplit("\\", 1)[-1].lower()


def find_window(exe):
    found = []

    @ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)
    def callback(hwnd, _):
        if user32.IsWindowVisible(hwnd):
            pid = wt.DWORD()
            user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
            if process_name(pid.value) == exe.lower():
                r = wt.RECT()
                user32.GetClientRect(hwnd, ctypes.byref(r))
                found.append((r.right * r.bottom, hwnd, r.right, r.bottom))
        return True
    user32.EnumWindows(callback, 0)
    return max(found) if found else None


def press(hwnd, name):
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.2)
    code = VK[name]
    scan = user32.MapVirtualKeyW(code, 0)
    user32.keybd_event(code, scan, 0, 0)
    time.sleep(0.08)
    user32.keybd_event(code, scan, 2, 0)
    time.sleep(0.3)


def capture(hwnd, width, height, path):
    from PIL import Image
    window_dc = user32.GetDC(hwnd)
    memory_dc = gdi32.CreateCompatibleDC(window_dc)
    bitmap = gdi32.CreateCompatibleBitmap(window_dc, width, height)
    gdi32.SelectObject(memory_dc, bitmap)
    user32.PrintWindow(hwnd, memory_dc, 3)   # PW_CLIENTONLY | PW_RENDERFULLCONTENT
    info = (ctypes.c_uint32 * 10)(40, width, (-height) & 0xffffffff, 1 | (32 << 16), 0, 0, 0, 0, 0, 0)
    pixels = ctypes.create_string_buffer(width * height * 4)
    gdi32.GetDIBits(memory_dc, bitmap, 0, height, pixels, info, 0)
    Image.frombuffer("RGBA", (width, height), pixels, "raw", "BGRA", 0, 1).convert("RGB").save(path)
    gdi32.DeleteObject(bitmap)
    gdi32.DeleteDC(memory_dc)
    user32.ReleaseDC(hwnd, window_dc)


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    exe, out = sys.argv[1], sys.argv[2]
    args = sys.argv[3:]
    if "--wait" in args:
        time.sleep(float(args[args.index("--wait") + 1]))
    hit = find_window(exe)
    if not hit:
        sys.exit("no visible window of " + exe)
    _, hwnd, width, height = hit
    for i, a in enumerate(args):
        if a == "--key":
            press(hwnd, args[i + 1])
    capture(hwnd, width, height, out)
    print("captured %dx%d -> %s" % (width, height, out))


if __name__ == "__main__":
    main()
