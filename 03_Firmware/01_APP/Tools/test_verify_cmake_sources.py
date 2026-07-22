#!/usr/bin/env python3
"""Focused regression fixture for Make filter-out source exclusions."""
from __future__ import annotations

import importlib.util
import tempfile
from pathlib import Path

SCRIPT_PATH = Path(__file__).with_name("verify_cmake_sources.py")
SPEC = importlib.util.spec_from_file_location("verify_cmake_sources", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
VERIFY_CMAKE_SOURCES = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(VERIFY_CMAKE_SOURCES)


def test_filter_out_removes_literal_and_wildcard_draw_sources() -> None:
    with tempfile.TemporaryDirectory() as temporary_directory:
        source_root = Path(temporary_directory)
        for relative_path in (
            "lvgl/src/draw/lv_draw.c",
            "lvgl/src/draw/nxp/lv_draw_nxp.c",
            "lvgl/src/draw/sdl/lv_draw_sdl.c",
            "lvgl/src/draw/sw/lv_draw_sw.c",
        ):
            source_path = source_root / relative_path
            source_path.parent.mkdir(parents=True, exist_ok=True)
            source_path.write_text("/* fixture */\n", encoding="utf-8")

        makefile = source_root / "Makefile"
        makefile.write_text(
            "LVGL_DIR = lvgl\n"
            "C_SOURCES += $(filter-out "
            "$(LVGL_DIR)/src/draw/nxp/lv_draw_nxp.c "
            "$(LVGL_DIR)/src/draw/sdl/*.c, "
            "$(wildcard $(LVGL_DIR)/src/draw/*.c) "
            "$(wildcard $(LVGL_DIR)/src/draw/nxp/*.c) "
            "$(wildcard $(LVGL_DIR)/src/draw/sdl/*.c))\n"
            "C_SOURCES += $(wildcard $(LVGL_DIR)/src/draw/sw/*.c)\n",
            encoding="utf-8",
        )

        sources = VERIFY_CMAKE_SOURCES.parse_make_sources(makefile, source_root)

        assert Path("lvgl/src/draw/lv_draw.c") in sources
        assert Path("lvgl/src/draw/sw/lv_draw_sw.c") in sources
        assert Path("lvgl/src/draw/nxp/lv_draw_nxp.c") not in sources
        assert Path("lvgl/src/draw/sdl/lv_draw_sdl.c") not in sources


def test_filter_out_percent_requires_non_empty_stem() -> None:
    with tempfile.TemporaryDirectory() as temporary_directory:
        source_root = Path(temporary_directory)
        for relative_path in ("src/foo.c", "src/foobar.c"):
            source_path = source_root / relative_path
            source_path.parent.mkdir(parents=True, exist_ok=True)
            source_path.write_text("/* fixture */\n", encoding="utf-8")

        makefile = source_root / "Makefile"
        makefile.write_text(
            "C_SOURCES += $(filter-out src/foo%.c, $(wildcard src/*.c))\n",
            encoding="utf-8",
        )

        sources = VERIFY_CMAKE_SOURCES.parse_make_sources(makefile, source_root)

        assert Path("src/foo.c") in sources
        assert Path("src/foobar.c") not in sources


def main() -> None:
    test_filter_out_removes_literal_and_wildcard_draw_sources()
    test_filter_out_percent_requires_non_empty_stem()
    print("PASS: filter-out fixture removes excluded draw sources and retains included wildcards")


if __name__ == "__main__":
    main()
