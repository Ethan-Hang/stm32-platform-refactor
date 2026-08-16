#!/usr/bin/env python3
"""Normalize an Intel HEX file to LF line endings without changing records."""

from pathlib import Path
import sys


def main() -> int:
    if len(sys.argv) != 2:
        raise SystemExit("usage: normalize_hex.py <firmware.hex>")
    path = Path(sys.argv[1])
    payload = path.read_bytes().replace(b"\r\n", b"\n")
    path.write_bytes(payload)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
