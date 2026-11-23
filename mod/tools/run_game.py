r"""run_game.py - start Gurumin through its launcher, take screenshots, close it (for testing the display mod).

    python mod/tools/run_game.py <game folder> <out prefix> [--windowed] [--shots 20,40] [--keys 30:enter,35:click600x226]
                                                            [--keep]

Starts game.exe, ticks the launcher's "Windowモード" box when --windowed is given, presses "ゲーム起動", then saves
<out prefix>_<seconds>.png at each time in --shots (seconds after the start button) and presses the given keys at
the given times (clickXxY = left click at client coordinates X, Y). The game is closed at the end unless --keep is given. Prints the mod's log (gurumin_mod.log).
"""
import ctypes
import ctypes.wintypes as wt
import os
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import capture_window  # noqa: E402

user32 = ctypes.windll.user32
BM_CLICK, BM_GETCHECK = 0x00F5, 0x00F0


def launcher_controls():
    """{text: hwnd} for the launcher's buttons, and the launcher window."""
    controls, launcher = {}, []

    @ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)
    def child(hwnd, _):
        text = ctypes.create_unicode_buffer(64)
        user32.GetWindowTextW(hwnd, text, 64)
        controls.setdefault(text.value, hwnd)
        return True

    @ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)
    def top(hwnd, _):
        text = ctypes.create_unicode_buffer(64)
        user32.GetWindowTextW(hwnd, text, 64)
        if user32.IsWindowVisible(hwnd) and text.value.startswith("Gurumin Ver"):
            launcher.append(hwnd)
            user32.EnumChildWindows(hwnd, child, 0)
        return True
    user32.EnumWindows(top, 0)
    return controls, (launcher[0] if launcher else None)


def click(hwnd, x, y):
    """Left click at client coordinates (the menus need the mouse)."""
    point = wt.POINT(x, y)
    user32.ClientToScreen(hwnd, ctypes.byref(point))
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.2)
    user32.SetCursorPos(point.x, point.y)
    time.sleep(0.2)
    user32.mouse_event(0x0002, 0, 0, 0, 0)
    time.sleep(0.08)
    user32.mouse_event(0x0004, 0, 0, 0, 0)
    time.sleep(0.3)


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    folder, prefix = sys.argv[1], sys.argv[2]
    args = sys.argv[3:]
    shots = [float(t) for t in args[args.index("--shots") + 1].split(",")] if "--shots" in args else [20]
    keys = []
    if "--keys" in args:
        for item in args[args.index("--keys") + 1].split(","):
            t, k = item.split(":")
            keys.append((float(t), k))
    subprocess.Popen([os.path.join(folder, "game.exe")], cwd=folder)
    controls, launcher = {}, None
    for _ in range(40):
        time.sleep(0.5)
        controls, launcher = launcher_controls()
        if "ゲーム起動" in controls:
            break
    else:
        sys.exit("launcher did not appear")
    if "--windowed" in args and "Windowモード" in controls:
        if user32.SendMessageW(controls["Windowモード"], BM_GETCHECK, 0, 0) == 0:
            user32.SendMessageW(controls["Windowモード"], BM_CLICK, 0, 0)
    user32.SetForegroundWindow(launcher)
    user32.PostMessageW(controls["ゲーム起動"], BM_CLICK, 0, 0)
    start = time.time()
    events = sorted([(t, "shot", None) for t in shots] + [(t, "key", k) for t, k in keys])
    for t, kind, key in events:
        delay = start + t - time.time()
        if delay > 0:
            time.sleep(delay)
        hit = capture_window.find_window("game.exe")
        if not hit:
            print("%5.1f s: no game window" % t)
            continue
        _, hwnd, width, height = hit
        if kind == "key" and key.startswith("click"):
            click(hwnd, *map(int, key[5:].split("x")))
            print("%5.1f s: %s" % (t, key))
        elif kind == "key":
            capture_window.press(hwnd, key)
            print("%5.1f s: pressed %s" % (t, key))
        else:
            path = "%s_%02d.png" % (prefix, int(t))
            capture_window.capture(hwnd, width, height, path)
            print("%5.1f s: %dx%d -> %s" % (t, width, height, path))
    if "--keep" not in args:
        subprocess.run(["taskkill", "/IM", "game.exe", "/F"], capture_output=True)
        time.sleep(1)
    log = os.path.join(folder, "gurumin_mod.log")
    if os.path.exists(log):
        print(open(log, encoding="utf-8", errors="replace").read())


if __name__ == "__main__":
    main()
