#!/usr/bin/env python3
"""LVGL asset packer.

Builds the binary blob that gets programmed into the W25Q64 LVGL sub-region
via JFlash + the custom .FLM (which remaps JLink address 0x90000000 to W25Q64
byte offset 0x300000 = LVGL partition start).

Output layout follows ``00_Config/inc/cfg_storage.h``:

    offset 0x000000  CFG_LVGL_ASSET_MAGIC          (4 B, little-endian)
    offset 0x001000  41 UI images                  (sector-aligned entries)
    offset 0x096000  custom font bitmap payloads   (sector-aligned entries)
    ...              0xFF padding to next sector

Image bytes are extracted from the LVGL-converter ``.c`` files in
``lvgl_ui/images``, picking the ``#if LV_COLOR_DEPTH == ...`` branch that
matches lv_conf.h.

The packed image/font bytes are extracted from generated LVGL ``.c`` files.
The firmware-side font files may compile a tiny placeholder bitmap only; the
full ``glyph_bitmap[]`` remains in source text so this packer can still build
the W25Q64 image.
"""

import argparse
import re
import sys
from pathlib import Path

# (cfg_macro_stem, c_filename, array_var_name) — all parsed from --image-dir
ASSETS = [
    ("FEN",         "_fen_alpha_80x8.c",          "_fen_alpha_80x8_map"),
    ("TIME",        "_time_alpha_50x8.c",         "_time_alpha_50x8_map"),
    ("MDLBG",       "_MDLBG_alpha_240x280.c",     "_MDLBG_alpha_240x280_map"),
    ("BIAOPAN1",    "_biaopan1_200x200.c",        "_biaopan1_200x200_map"),
    ("WATCHDIGHT1", "_watchdight1_alpha_60x60.c", "_watchdight1_alpha_60x60_map"),
    ("WATCHDIGHT2", "_watchdight2_alpha_60x60.c", "_watchdight2_alpha_60x60_map"),
    ("WATCHDIGHT3", "_watchdight3_alpha_60x60.c", "_watchdight3_alpha_60x60_map"),
    ("SHESHIDU",    "_sheshidu_alpha_10x10.c",    "_sheshidu_alpha_10x10_map"),
    ("WATHER16X16", "_wather16x16_alpha_16x16.c", "_wather16x16_alpha_16x16_map"),
    ("HEART16X16",  "_heart16x16_alpha_16x16.c",  "_heart16x16_alpha_16x16_map"),
    ("KLL16X16",    "_KLL16x16_alpha_16x16.c",    "_KLL16x16_alpha_16x16_map"),
    ("FOOT16X16",   "_foot16x16_alpha_16x16.c",   "_foot16x16_alpha_16x16_map"),
    ("BT32",        "_BT32_alpha_32x32.c",        "_BT32_alpha_32x32_map"),
    ("MIANTI_0",    "_mianti_0_alpha_32x32.c",    "_mianti_0_alpha_32x32_map"),
    ("ZHENGDONG_0", "_zhengdong_0_alpha_32x32.c", "_zhengdong_0_alpha_32x32_map"),
    ("COPESSS",     "_copesss_alpha_32x32.c",     "_copesss_alpha_32x32_map"),
    ("WEATER32X32", "_weater32x32_alpha_32x32.c", "_weater32x32_alpha_32x32_map"),
    ("ELLIPSE",     "_Ellipse_alpha_40x40.c",     "_Ellipse_alpha_40x40_map"),
    ("STIME",       "_Stime_alpha_16x8.c",        "_Stime_alpha_16x8_map"),
    ("SFEN",        "_Sfen_alpha_21x6.c",         "_Sfen_alpha_21x6_map"),
    ("POWER_HIGHT", "_power_hight_alpha_32x32.c", "_power_hight_alpha_32x32_map"),
    ("LOCATION",    "_location_alpha_32x32.c",    "_location_alpha_32x32_map"),
    ("TAIWAN",      "_taiwan_alpha_32x32.c",      "_taiwan_alpha_32x32_map"),
    ("NFC",         "_nfc_alpha_32x32.c",         "_nfc_alpha_32x32_map"),
    ("LIANGDU",     "_liangdu_47x47.c",           "_liangdu_47x47_map"),
    ("ZNZBG",       "_ZNZBG_alpha_100x100.c",     "_ZNZBG_alpha_100x100_map"),
    ("ARW",         "_arw_alpha_50x40.c",         "_arw_alpha_50x40_map"),
    ("ZNZ",         "_ZNZ_alpha_50x50.c",         "_ZNZ_alpha_50x50_map"),
    ("HEART32X32",  "_heart32x32_alpha_32x32.c",  "_heart32x32_alpha_32x32_map"),
    ("TIWEN",       "_tiwen_alpha_32x32.c",       "_tiwen_alpha_32x32_map"),
    ("PA",          "_pa_alpha_32x32.c",          "_pa_alpha_32x32_map"),
    ("LOCATION32X32","_location32x32_alpha_32x32.c","_location32x32_alpha_32x32_map"),
    ("BIGHEART",    "_BIGHeart_alpha_93x85.c",    "_BIGHeart_alpha_93x85_map"),
    ("NFC32X32",    "_NFC32x32_alpha_32x32.c",    "_NFC32x32_alpha_32x32_map"),
    ("ERROR48",     "_error_alpha_48x48.c",       "_error_alpha_48x48_map"),
    ("HEART37X32",  "_heart32x32_alpha_37x32.c",  "_heart32x32_alpha_37x32_map"),
    ("LOCATION20X20","_location20x20_alpha_20x20.c","_location20x20_alpha_20x20_map"),
    ("LOGO100",     "_logo_100x100.c",            "_logo_100x100_map"),
    ("QRCODE32",    "_qrcode32x32_alpha_32x32.c", "_qrcode32x32_alpha_32x32_map"),
    ("SET32",       "_set32x32_alpha_32x32.c",    "_set32x32_alpha_32x32_map"),
    ("SYTEAM32",    "_syteam32x32_alpha_32x32.c", "_syteam32x32_alpha_32x32_map"),
]

# (cfg_macro_stem, c_filename)
FONTS = [
    ("INTERTTF_24", "lv_font_interttf_24.c"),
    ("INTERTTF_10", "lv_font_interttf_10.c"),
    ("INTERTTF_82", "lv_font_interttf_82.c"),
    ("ALIMAMA_16", "lv_font_alimama_16.c"),
    ("ALIMAMA_36", "lv_font_alimama_36.c"),
    ("DIGITALDREAMFATNARROW_36", "lv_font_digitaldreamfatnarrow_36.c"),
    ("ALIMAMA_12", "lv_font_alimama_12.c"),
    ("ALIMAMA_10", "lv_font_alimama_10.c"),
    ("INTERTTF_16", "lv_font_interttf_16.c"),
]

_CFG_RE = re.compile(
    r"#define\s+(CFG_(?:LVGL_ASSET|LVGL_FONT|W25Q64)_[A-Z_0-9]+)\s+\(?\s*"
    r"(0[xX][0-9a-fA-F]+|\d+)U?L?\s*\)?"
)
_HEX_BYTE_RE = re.compile(r"\b0x([0-9a-fA-F]+)\b")
_C_NUM_RE = re.compile(
    r"\b0x[0-9a-fA-F]+\b|(?<![A-Za-z0-9_])\d+(?![A-Za-z0-9_])"
)


def parse_macros(header: Path) -> dict[str, int]:
    """Extract `#define CFG_..._X  (literal)` pairs from a header."""
    text = header.read_text(encoding="utf-8")
    return {m.group(1): int(m.group(2), 0) for m in _CFG_RE.finditer(text)}


def read_lv_color(conf: Path) -> tuple[int, int]:
    text = conf.read_text(encoding="utf-8")
    depth_m = re.search(r"^\s*#define\s+LV_COLOR_DEPTH\s+(\d+)", text, re.MULTILINE)
    swap_m = re.search(r"^\s*#define\s+LV_COLOR_16_SWAP\s+(\d+)", text, re.MULTILINE)
    if depth_m is None:
        sys.exit(f"LV_COLOR_DEPTH not found in {conf}")
    return int(depth_m.group(1)), (int(swap_m.group(1)) if swap_m else 0)


def _block_active(condition: str, depth: int, swap: int) -> bool:
    """Evaluate `LV_COLOR_DEPTH == X [&& ...]` style C-preprocessor guards."""
    expr = (condition.replace("LV_COLOR_DEPTH", str(depth))
                     .replace("LV_COLOR_16_SWAP", str(swap))
                     .replace("&&", " and ")
                     .replace("||", " or "))
    try:
        return bool(eval(expr, {"__builtins__": {}}))
    except Exception:
        return False


def extract_array_bytes(c_file: Path, var: str, depth: int, swap: int) -> bytes:
    src = c_file.read_text(encoding="utf-8")
    decl = re.search(rf"\b{re.escape(var)}\s*\[\s*\]\s*=\s*\{{", src)
    if decl is None:
        sys.exit(f"array `{var}` not found in {c_file}")
    body_end = src.find("};", decl.end())
    if body_end < 0:
        sys.exit(f"unterminated array `{var}` in {c_file}")
    body = src[decl.end():body_end]

    out: list[int] = []
    active = True  # outside any #if/#endif: include
    for line in body.splitlines():
        s = line.strip()
        if s.startswith("#if "):
            active = _block_active(s[4:], depth, swap)
        elif s.startswith("#endif"):
            active = True
        elif active:
            for m in _HEX_BYTE_RE.finditer(s):
                v = int(m.group(1), 16)
                if v > 0xFF:
                    sys.exit(f"non-byte literal 0x{v:x} in {c_file}")
                out.append(v)
    return bytes(out)


def extract_font_bitmap(c_file: Path) -> bytes:
    src = c_file.read_text(encoding="utf-8")
    decl = re.search(r"\bglyph_bitmap\s*\[\s*\]\s*=\s*\{", src)
    if decl is None:
        sys.exit(f"glyph_bitmap[] not found in {c_file}")
    body_end = src.find("};", decl.end())
    if body_end < 0:
        sys.exit(f"unterminated glyph_bitmap[] in {c_file}")
    body = src[decl.end():body_end]
    body = re.sub(r"/\*.*?\*/", "", body, flags=re.DOTALL)
    body = re.sub(r"//.*", "", body)

    out: list[int] = []
    for m in _C_NUM_RE.finditer(body):
        v = int(m.group(0), 0)
        if v > 0xFF:
            sys.exit(f"non-byte literal 0x{v:x} in {c_file}")
        out.append(v)
    return bytes(out)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--config",        required=True, type=Path)
    ap.add_argument("--lv-conf",       required=True, type=Path)
    ap.add_argument("--image-dir",     required=True, type=Path,
                    help="lvgl_ui/images dir (sprite .c files compiled into firmware)")
    ap.add_argument("--font-dir",      required=True, type=Path,
                    help="lvgl_ui/guider_fonts dir (custom font .c files)")
    ap.add_argument("--output",        required=True, type=Path)
    args = ap.parse_args()

    cfg = parse_macros(args.config)
    try:
        magic       = cfg["CFG_LVGL_ASSET_MAGIC"]
        magic_off   = cfg["CFG_LVGL_ASSET_MAGIC_OFFSET"]
        magic_size  = cfg["CFG_LVGL_ASSET_MAGIC_SIZE"]
        sector_size = cfg["CFG_W25Q64_SECTOR_SIZE"]
    except KeyError as e:
        sys.exit(f"missing macro {e} in {args.config}")
    if magic_size != 4:
        sys.exit("magic must be 4 bytes")

    # cfg_storage.h defines per-asset SIZE as W*H*PX_SIZE.  The macro parser
    # only resolves literal-valued macros, so synthesise composite SIZEs here.
    for name, *_ in ASSETS:
        w = cfg.get(f"CFG_LVGL_ASSET_{name}_W")
        h = cfg.get(f"CFG_LVGL_ASSET_{name}_H")
        px = cfg.get(f"CFG_LVGL_ASSET_{name}_PX_SIZE")
        if w is not None and h is not None and px is not None:
            cfg[f"CFG_LVGL_ASSET_{name}_SIZE"] = w * h * px

    depth, swap = read_lv_color(args.lv_conf)

    # Round footprint up to sector — JFlash erases per sector, predictable size.
    image_ends = [
        cfg[f"CFG_LVGL_ASSET_{name}_OFFSET"] +
        cfg[f"CFG_LVGL_ASSET_{name}_SIZE"]
        for name, *_ in ASSETS
    ]
    font_ends = [
        cfg[f"CFG_LVGL_FONT_{name}_BITMAP_OFFSET"] +
        cfg[f"CFG_LVGL_FONT_{name}_BITMAP_SIZE"]
        for name, _ in FONTS
    ]
    footprint = max(image_ends + font_ends)
    aligned = -(-footprint // sector_size) * sector_size
    buf = bytearray(b"\xff" * aligned)
    buf[magic_off:magic_off + 4] = magic.to_bytes(4, "little")

    print(f"  LV_COLOR_DEPTH={depth}, LV_COLOR_16_SWAP={swap}", file=sys.stderr)
    print(f"  magic 0x{magic:08x} @ 0x{magic_off:06x}  4 B", file=sys.stderr)
    for name, fname, var in ASSETS:
        offset = cfg[f"CFG_LVGL_ASSET_{name}_OFFSET"]
        size   = cfg[f"CFG_LVGL_ASSET_{name}_SIZE"]
        data   = extract_array_bytes(args.image_dir / fname, var, depth, swap)
        if len(data) != size:
            sys.exit(f"{name}: extracted {len(data)} bytes, expected {size}")
        buf[offset:offset + size] = data
        print(f"  {name.lower():<8} @ 0x{offset:06x}  {size:6d} B  ({fname})",
              file=sys.stderr)

    for name, fname in FONTS:
        offset = cfg[f"CFG_LVGL_FONT_{name}_BITMAP_OFFSET"]
        size   = cfg[f"CFG_LVGL_FONT_{name}_BITMAP_SIZE"]
        data   = extract_font_bitmap(args.font_dir / fname)
        if len(data) != size:
            sys.exit(f"{name}: extracted {len(data)} bytes, expected {size}")
        buf[offset:offset + size] = data
        print(f"  font {name.lower():<24} @ 0x{offset:06x}  {size:6d} B",
              file=sys.stderr)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(buf)
    print(f"  wrote {len(buf)} B → {args.output}", file=sys.stderr)


if __name__ == "__main__":
    main()
