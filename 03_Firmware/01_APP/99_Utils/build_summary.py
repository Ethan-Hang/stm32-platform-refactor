#!/usr/bin/env python3
"""Build summary tool for the CMake/Ninja firmware build.

Runs ``cmake --build`` for a configured binary directory, streams the log,
measures elapsed time, counts warnings and errors, and prints a summary.
"""

import os
import re
import subprocess
import sys
import time


def main():
    # Parse arguments  ---------------------------------------------------
    cmake_exe = "cmake"
    binary_dir = "build/Debug"
    build_log = os.path.join("build", "build.log")
    target = "firmware"
    jobs = os.cpu_count() or 4

    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--cmake-exe" and i + 1 < len(args):
            cmake_exe = args[i + 1]
            i += 2
        elif args[i] == "--binary-dir" and i + 1 < len(args):
            binary_dir = args[i + 1]
            i += 2
        elif args[i] == "--build-log" and i + 1 < len(args):
            build_log = args[i + 1]
            i += 2
        elif args[i] in ("--target",) and i + 1 < len(args):
            target = args[i + 1]
            i += 2
        elif args[i] in ("--jobs", "-j") and i + 1 < len(args):
            try:
                jobs = int(args[i + 1])
            except ValueError:
                pass
            i += 2
        else:
            i += 1

    os.makedirs(os.path.dirname(build_log) or ".", exist_ok=True)

    # Fresh log file  ----------------------------------------------------
    with open(build_log, "w") as f:
        f.write("")

    t0 = time.time()

    cmd = [
        cmake_exe,
        "--build",
        binary_dir,
        "--target",
        target,
        "--parallel",
        str(jobs),
    ]

    # Stream the child's output live (tee-style) instead of capturing it all and
    # dumping at exit. The old subprocess.run(capture_output=True) buffered the
    # entire build in memory and re-emitted it in one burst here, which (a) lost
    # the progressive "watch it compile" output and (b) slammed a large block to
    # the terminal right before this process exits — racing VSCode's "terminal
    # will be reused, press any key to close" banner and intermittently
    # truncating the tail. Reading line-by-line and flushing each line keeps the
    # output on screen as it happens and leaves nothing buffered at exit.
    #
    # stderr is merged into stdout so on-screen ordering matches the native
    # build, while a single stream avoids pipe deadlocks.
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
        bufsize=1,  # line-buffered
    )

    captured = []
    with open(build_log, "a") as f:
        assert proc.stdout is not None
        for line in proc.stdout:
            sys.stdout.write(line)
            sys.stdout.flush()
            f.write(line)
            captured.append(line)
    returncode = proc.wait()

    t1 = time.time()
    elapsed = int(t1 - t0 + 0.5)

    # Count warnings / errors (same regex logic as build_summary.ps1)  --
    combined = "".join(captured)
    warnings = len(re.findall(r"(?im)(^|[^A-Za-z0-9_])warning:", combined))
    errors = len(re.findall(r"(?im)(^|[^A-Za-z0-9_])error:", combined))

    mins = elapsed // 60
    secs = elapsed % 60
    print()
    print("=== Build Summary ===")
    print(f"Compile time: {mins:02d}:{secs:02d} ({elapsed}s)")
    print(f"Warnings: {warnings}")
    print(f"Errors  : {errors}")

    # Nothing should be buffered at this point (each line was flushed as read),
    # but flush once more before exit for good measure.
    sys.stdout.flush()
    sys.exit(returncode)


if __name__ == "__main__":
    main()
