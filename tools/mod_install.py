r"""mod_install.py - put a mod's rebuilt files where the mod DLL loads them, or take them out again.

    python tools/mod_install.py install <mod out folder> <game folder> <mod name>
    python tools/mod_install.py remove <game folder> <mod name>

install copies <mod out folder> to <game>\mod\<mod name>\ (replacing an older copy of the same mod) and puts the mod
DLL (mod/build/d3d9.dll) and gurumin_mod.ini next to game.exe when they are not there yet. A d3d9.dll that is not the
mod's is kept as d3d9.dll.orig. The DLL replaces every game file that has a file of the same name under <game>\mod\
(docs/MODDING_FILES.md). remove deletes <game>\mod\<mod name>\ again; the DLL stays (mod\uninstall.bat removes it).
Nothing else in the game folder is changed.
"""
import os
import shutil
import sys

REPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MOD_DLL = os.path.join(REPO_DIR, "mod", "build", "d3d9.dll")
MOD_INI = os.path.join(REPO_DIR, "mod", "gurumin_mod.ini")
DLL_MARKER = b"gurumin_mod"


def safe_name(name):
    name = "".join(c for c in name if c not in '<>:"/\\|?*').strip(" .")
    if not name:
        sys.exit("mod name is empty")
    return name


def install_dll(game_dir):
    if not os.path.isfile(MOD_DLL):
        print("warning: the mod DLL is not built (mod/build.sh); the game will not load the mod without it")
        return
    dll = os.path.join(game_dir, "d3d9.dll")
    if os.path.isfile(dll):
        current = open(dll, "rb").read()
        if DLL_MARKER not in current and not os.path.exists(dll + ".orig"):
            shutil.copyfile(dll, dll + ".orig")
            print("kept the existing d3d9.dll as d3d9.dll.orig")
        if current == open(MOD_DLL, "rb").read():
            dll = None
    if dll:
        shutil.copyfile(MOD_DLL, dll)
        print("installed d3d9.dll")
    ini = os.path.join(game_dir, "gurumin_mod.ini")
    if not os.path.isfile(ini) and os.path.isfile(MOD_INI):
        shutil.copyfile(MOD_INI, ini)
        print("installed gurumin_mod.ini")


def main():
    args = sys.argv[1:]
    if len(args) == 4 and args[0] == "install":
        out_dir, game_dir, name = args[1], args[2], safe_name(args[3])
        if not os.path.isfile(os.path.join(game_dir, "game.exe")):
            sys.exit("no game.exe in " + game_dir)
        if not os.path.isdir(out_dir) or not os.listdir(out_dir):
            sys.exit("nothing to install: %s is empty (repackage first)" % out_dir)
        target = os.path.join(game_dir, "mod", name)
        if os.path.isdir(target):
            shutil.rmtree(target)
        shutil.copytree(out_dir, target)
        count = sum(len(files) for _d, _s, files in os.walk(target))
        print("copied %d file(s) to %s" % (count, target))
        install_dll(game_dir)
    elif len(args) == 3 and args[0] == "remove":
        game_dir, name = args[1], safe_name(args[2])
        target = os.path.join(game_dir, "mod", name)
        if os.path.isdir(target):
            shutil.rmtree(target)
            print("removed " + target)
        else:
            print("not installed: " + target)
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
