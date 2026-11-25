"""build_exe.py - builds the modding GUI as one standalone program: dist/GuruminModTools.exe on Windows,
dist/GuruminModTools on Linux (x64 or arm64, whichever machine builds it; PyInstaller does not cross-build).

    pip install pyinstaller pillow
    python tools/build_exe.py

The program holds Python, Tkinter, Pillow, every tool in tools/ and, when they are built, tools/native/iti_unpack.exe
(a Windows helper; on Linux the tools run it through Wine if Wine is installed), the JP mod DLL (mod/build/d3d9.dll
with mod/gurumin_mod.ini) and the EN Enhanced Rendering mod (mod_en/dist: d3d9.dll, GuruminModern.ini and the
GuruminVR host). It holds no game files.
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
    en_dist = os.path.join(REPO_DIR, "mod_en", "dist")      # ./mod_en/build.sh
    for name in ("d3d9.dll", "GuruminModern.ini", "GuruminModern-FXAA-LICENSE.txt"):
        optional.append((os.path.join(en_dist, name), os.path.join("mod_en", "dist")))
    for name in ("gurumin_vr_host.exe", "openxr_loader.dll", "OpenXR-LICENSE.txt"):
        optional.append((os.path.join(en_dist, "GuruminVR", name), os.path.join("mod_en", "dist", "GuruminVR")))
    for src, dst in optional:
        if os.path.isfile(src):
            files.append((src, dst))
        else:
            print("not bundled (not built): %s" % os.path.relpath(src, REPO_DIR))
    return files


def main():
    modules = [os.path.splitext(os.path.basename(p))[0] for p in tool_scripts()]
    exe_name = NAME + (".exe" if sys.platform == "win32" else "")
    command = [sys.executable, "-m", "PyInstaller", "--noconfirm", "--clean", "--onefile", "--windowed",
               "--name", NAME, "--paths", TOOLS_DIR,
               "--distpath", os.path.join(REPO_DIR, "dist"),
               "--workpath", os.path.join(REPO_DIR, "build", "pyinstaller"),
               "--specpath", os.path.join(REPO_DIR, "build", "pyinstaller"),
               "--collect-submodules", "PIL", "--hidden-import", "tkinter"]
    for module in modules:
        command += ["--hidden-import", module]
    for src, dst in bundled_files():
        command += ["--add-data", "%s%s%s" % (src, os.pathsep, dst)]
    command.append(os.path.join(TOOLS_DIR, "gurumin_tools_gui.py"))
    subprocess.check_call(command)
    print("built %s" % os.path.join(REPO_DIR, "dist", exe_name))


if __name__ == "__main__":
    main()
