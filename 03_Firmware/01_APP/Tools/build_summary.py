#!/usr/bin/env python3
"""Build summary tool — cross-platform replacement for build_summary.ps1.

Runs ``make build-core``, captures the log, measures elapsed time,
counts warnings and errors, and prints a one-line summary.
"""

import sys
import os
import re
import time
import subprocess


def main():
    # Parse arguments  ---------------------------------------------------
    make_exe  = 'make'
    build_dir = 'build'
    build_log = os.path.join(build_dir, 'build.log')
    target    = 'build-core'
    jobs      = os.cpu_count() or 4

    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] in ('--make-exe',) and i + 1 < len(args):
            make_exe = args[i + 1]; i += 2
        elif args[i] in ('--build-dir',) and i + 1 < len(args):
            build_dir = args[i + 1]; build_log = os.path.join(build_dir, 'build.log'); i += 2
        elif args[i] in ('--build-log',) and i + 1 < len(args):
            build_log = args[i + 1]; i += 2
        elif args[i] in ('--target',) and i + 1 < len(args):
            target = args[i + 1]; i += 2
        elif args[i] in ('--jobs', '-j') and i + 1 < len(args):
            try: jobs = int(args[i + 1])
            except ValueError: pass
            i += 2
        else:
            i += 1

    os.makedirs(build_dir, exist_ok=True)

    # Fresh log file  ----------------------------------------------------
    with open(build_log, 'w') as f:
        f.write('')

    t0 = time.time()

    cmd = [make_exe, '--no-print-directory', f'-j{jobs}', target]
    # On Windows the shell invocation differs; rely on subprocess defaults.
    proc = subprocess.run(cmd, capture_output=True, text=True)

    t1 = time.time()

    # Write captured output to the log  ---------------------------------
    with open(build_log, 'a') as f:
        f.write(proc.stdout)
        if proc.stderr:
            f.write(proc.stderr)

    elapsed = int(t1 - t0 + 0.5)

    # Count warnings / errors (same regex logic as build_summary.ps1)  --
    combined = proc.stdout + '\n' + proc.stderr
    warnings = len(re.findall(r'(?im)(^|[^A-Za-z0-9_])warning:', combined))
    errors   = len(re.findall(r'(?im)(^|[^A-Za-z0-9_])error:', combined))

    # Also print the captured output so the user sees the build  --------
    sys.stdout.write(proc.stdout)
    if proc.stderr:
        sys.stderr.write(proc.stderr)

    mins = elapsed // 60
    secs = elapsed % 60
    print()
    print('=== Build Summary ===')
    print(f'Compile time: {mins:02d}:{secs:02d} ({elapsed}s)')
    print(f'Warnings: {warnings}')
    print(f'Errors  : {errors}')

    sys.exit(proc.returncode)


if __name__ == '__main__':
    main()
