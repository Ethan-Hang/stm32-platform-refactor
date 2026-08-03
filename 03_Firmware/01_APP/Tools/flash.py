#!/usr/bin/env python3
"""Cross-platform JFlash driver for CMake download and flash-assets targets.

CMake owns all firmware and asset build dependencies. This script only
locates JFlash, converts host paths, validates prepared inputs, and invokes
JFlash with the requested image.

Supported hosts: native Windows, WSL, and native Linux. ``JFLASH_EXE`` or
``--jflash-exe`` overrides automatic discovery.
"""


import argparse
import glob
import os
import shutil
import subprocess
import sys


# --------------------------------------------------------------------------
# host detection
# --------------------------------------------------------------------------
def is_windows() -> bool:
    return os.name == "nt"


def is_wsl() -> bool:
    """True when running under the Linux-on-Windows subsystem.

    JFlash on WSL is still the *Windows* JFlash.exe under /mnt/c, reached via
    WSL's binfmt interop — so paths handed to it must be Windows form.
    """
    if is_windows():
        return False
    try:
        with open("/proc/version", "r", encoding="utf-8", errors="ignore") as fh:
            return "microsoft" in fh.read().lower()
    except OSError:
        return False


def runs_windows_jflash() -> bool:
    """True when the JFlash binary we drive is a Windows .exe (native Win or WSL)."""
    return is_windows() or is_wsl()


# --------------------------------------------------------------------------
# JFlash binary discovery
# --------------------------------------------------------------------------
# Candidate roots searched for SEGGER's JFlash, newest version first.  These
# are only defaults — JFLASH_EXE / --jflash-exe always win.
_WIN_GLOBS = [
    r"C:\Program Files\SEGGER\JLink*\JFlash.exe",
    r"C:\Program Files (x86)\SEGGER\JLink*\JFlash.exe",
]
_WSL_GLOBS = [
    "/mnt/c/Program Files/SEGGER/JLink*/JFlash.exe",
    "/mnt/c/Program Files (x86)/SEGGER/JLink*/JFlash.exe",
]


def _newest(matches):
    """Pick the highest JLink version dir (lexical sort is good enough for
    JLink_VNNNx names; mtime breaks ties / odd naming)."""
    if not matches:
        return None
    return sorted(matches)[-1]


def find_jflash(override: str) -> str:
    if override:
        return override
    env = os.environ.get("JFLASH_EXE", "").strip()
    if env:
        return env

    if is_windows():
        for pat in _WIN_GLOBS:
            hit = _newest(glob.glob(pat))
            if hit:
                return hit
    elif is_wsl():
        for pat in _WSL_GLOBS:
            hit = _newest(glob.glob(pat))
            if hit:
                return hit
    else:  # native Linux — SEGGER ships JFlashExe / JFlash on PATH
        for name in ("JFlashExe", "JFlash"):
            hit = shutil.which(name)
            if hit:
                return hit

    sys.exit(
        "flash.py: could not locate JFlash. Set JFLASH_EXE or pass "
        "--jflash-exe.\n"
        "  Windows: C:\\Program Files\\SEGGER\\JLink_Vxxx\\JFlash.exe\n"
        "  WSL:     /mnt/c/Program Files/SEGGER/JLink_Vxxx/JFlash.exe\n"
        "  Linux:   install J-Link, ensure JFlashExe is on PATH"
    )


# --------------------------------------------------------------------------
# path conversion — JFlash always wants paths in its own native form
# --------------------------------------------------------------------------
def to_jflash_path(path: str) -> str:
    """Resolve `path` to absolute, then convert to the form the JFlash binary
    expects: Windows form on native Windows / WSL, POSIX form on Linux."""
    absolute = os.path.abspath(path)

    if is_windows():
        # uv's Windows Python already produces backslash Windows paths.
        return absolute
    if is_wsl():
        try:
            win = subprocess.check_output(
                ["wslpath", "-w", absolute], text=True
            ).strip()
            return win if win else absolute
        except (OSError, subprocess.CalledProcessError):
            return absolute
    # native Linux JFlash takes the POSIX path as-is
    return absolute


# --------------------------------------------------------------------------
# JFlash invocation
# --------------------------------------------------------------------------
def run_jflash(jflash_exe, prj, open_arg, dry_run: bool) -> int:
    """Drive JFlash headless: open project, open image, auto (connect→erase→
    program→verify), then exit / close the window — no clicking required."""
    cmd = [
        jflash_exe,
        "-openprj" + to_jflash_path(prj),
        "-open" + open_arg,
        "-auto",
        "-exit",
    ]
    print("  JFlash: " + " ".join(cmd))
    if dry_run:
        return 0
    return subprocess.call(cmd)


# --------------------------------------------------------------------------
# subcommands
# --------------------------------------------------------------------------
def cmd_download(args) -> int:
    hex_path = os.path.join(args.build_dir, args.target + ".hex")

    if not os.path.isfile(hex_path):
        if args.dry_run:
            print("  >> (dry-run) firmware image would be: %s" % hex_path)
        else:
            sys.exit(
                "flash.py: firmware image not found: %s\n"
                "Build it first with: cmake --build --preset Debug" % hex_path
            )

    jflash = find_jflash(args.jflash_exe)
    # .hex carries its own load address → no ,0xADDR suffix.
    open_arg = to_jflash_path(hex_path)
    print("  JFlash: program %s into APP slot" % open_arg)
    return run_jflash(jflash, args.jflash_prj, open_arg, args.dry_run)


def cmd_assets(args) -> int:
    if not os.path.isfile(args.bin) and not args.dry_run:
        sys.exit("flash.py: assets image not found: %s" % args.bin)
    jflash = find_jflash(args.jflash_exe)
    # raw .bin → must specify the JLink load address (remapped by the .FLM).
    open_arg = to_jflash_path(args.bin) + "," + args.addr
    print("  JFlash: program %s @ %s" % (to_jflash_path(args.bin), args.addr))
    return run_jflash(jflash, args.jflash_prj, open_arg, args.dry_run)


# --------------------------------------------------------------------------
# CLI
# --------------------------------------------------------------------------
def main() -> int:
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument("--dry-run", action="store_true",
                   help="print what would run without invoking JFlash")
    p.add_argument("--jflash-exe", default="",
                   help="override JFlash binary (else $JFLASH_EXE or auto-detect)")
    sub = p.add_subparsers(dest="subcmd", required=True)

    d = sub.add_parser("download", help="flash <target>.hex into the APP slot")
    d.add_argument("--build-dir", required=True)
    d.add_argument("--target", required=True)
    d.add_argument("--jflash-prj", required=True)
    d.set_defaults(func=cmd_download)

    a = sub.add_parser("assets", help="flash a raw .bin to a JLink address")
    a.add_argument("--bin", required=True)
    a.add_argument("--addr", default="0x90000000")
    a.add_argument("--jflash-prj", required=True)
    a.set_defaults(func=cmd_assets)

    args = p.parse_args()
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
