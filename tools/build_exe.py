"""build_exe.py - builds the modding GUI as one standalone Windows program, dist/GuruminModTools.exe.

    pip install pyinstaller pillow
    python tools/build_exe.py

The exe holds Python, Tkinter, Pillow, every tool in tools/, tools/native/iti_unpack.exe and the mod DLL
(mod/build/d3d9.dll with mod/gurumin_mod.ini) when they are built. It holds no game files.
"""
import glob
import os
import subprocess
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_DIR = os.path.dirname(TOOLS_DIR)
NAME = "GuruminModTools"


def tool_scripts():
    return sorted(p for p in glob.glob(os.path.join(TOOLS_DIR, "*.py"))
                  if not os.path.basename(p).startswith(("test_", "build_exe")))


def bundled_files():
    """(source, folder inside the exe) pairs; the exe keeps the repository's layout, so the tools find each other."""
    files = [(p, "tools") for p in tool_scripts()]
    optional = [(os.path.join(TOOLS_DIR, "native", "iti_unpack.exe"), os.path.join("tools", "native")),
                (os.path.join(REPO_DIR, "mod", "build", "d3d9.dll"), os.path.join("mod", "build")),
                (os.path.join(REPO_DIR, "mod", "gurumin_mod.ini"), "mod")]
    for src, dst in optional:
        if os.path.isfile(src):
            files.append((src, dst))
        else:
            print("not bundled (not built): %s" % os.path.relpath(src, REPO_DIR))
    return files


def main():
    modules = [os.path.splitext(os.path.basename(p))[0] for p in tool_scripts()]
    command = [sys.executable, "-m", "PyInstaller", "--noconfirm", "--clean", "--onefile", "--windowed",
               "--name", NAME, "--paths", TOOLS_DIR,
               "--distpath", os.path.join(REPO_DIR, "dist"),
               "--workpath", os.path.join(REPO_DIR, "build", "pyinstaller"),
               "--specpath", os.path.join(REPO_DIR, "build", "pyinstaller"),
               "--collect-submodules", "PIL"]
    for module in modules:
        command += ["--hidden-import", module]
    for src, dst in bundled_files():
        command += ["--add-data", "%s%s%s" % (src, os.pathsep, dst)]
    command.append(os.path.join(TOOLS_DIR, "gurumin_tools_gui.py"))
    subprocess.check_call(command)
    print("built %s" % os.path.join(REPO_DIR, "dist", NAME + ".exe"))


if __name__ == "__main__":
    main()
