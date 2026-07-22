#!/usr/bin/env python3
"""Check that the CMake compile database matches the legacy Makefile sources."""
from __future__ import annotations
import argparse
import fnmatch
import json
import posixpath
import re
from pathlib import Path

SOURCE_SUFFIXES = {".c", ".s", ".S"}
EXCLUDED_PATH_SUBSTRINGS = (
    "01_App/User_Sensor/audio/src/wt588_test_task.c",
    "01_App/User_Sensor/display/src/st7789_mock_test_task.c",
    "01_App/User_Sensor/display/src/st7789_hal_test_task.c",
    "01_App/User_Sensor/touch/src/cst816t_mock_test_task.c",
    "01_App/User_Sensor/touch/src/cst816t_hal_test_task.c",
    "01_App/User_Sensor/storage/src/w25q64_mock_test_task.c",
    "01_App/User_Sensor/storage/src/w25q64_handler_mock_test_task.c",
    "01_App/User_Sensor/storage/src/w25q64_hal_test_task.c",
    "01_App/User_Sensor/em7028/src/em7028_mock_test_task.c",
    "01_App/User_Sensor/em7028/src/em7028_handler_mock_test_task.c",
    "01_App/User_Sensor/em7028/src/em7028_iic_hal_test_task.c",
    "01_App/User_Sensor/em7028/src/em7028_jscope_capture_task.c",
    "04_Impl/impl_middleware/lvgl/lvgl_ui/images/",
)

def _relative_project_path(path: Path, source_root: Path) -> Path | None:
    """Normalize *path* to a source-root-relative path, or ignore it."""
    try:
        return path.resolve().relative_to(source_root.resolve())
    except ValueError:
        return None

def _logical_lines(text: str) -> list[str]:
    return re.sub(r"\\\n", " ", text).splitlines()

def _expand_variables(value: str, variables: dict[str, str]) -> str:
    for name, replacement in variables.items():
        value = value.replace(f"$({name})", replacement)
    return value

def _wildcard_patterns(expression: str) -> list[str]:
    """Extract Make wildcard arguments after project variables have expanded."""
    return re.findall(r"\$\(wildcard\s+([^)]*)\)", expression)

def _glob_sources(source_root: Path, pattern: str) -> set[Path]:
    return {
        relative for match in source_root.glob(pattern)
        if match.suffix in SOURCE_SUFFIXES
        if (relative := _relative_project_path(match, source_root)) is not None
    }


def _normalize_posix_project_path(value: str) -> str:
    """Return a normalized POSIX path for Make pattern comparisons."""
    normalized = posixpath.normpath(value.replace("\\", "/"))
    return normalized.removeprefix("./")


def _filter_out_patterns(expression: str) -> set[str]:
    """Return normalized literal and wildcard patterns from filter-out's first argument."""
    if "$(filter-out" not in expression:
        return set()
    first_argument = expression.split("$(filter-out", 1)[1].split(",", 1)[0]
    wildcard_patterns = _wildcard_patterns(first_argument)
    literal_patterns = re.sub(r"\$\(wildcard\s+[^)]*\)", " ", first_argument)
    return {
        _normalize_posix_project_path(pattern)
        for pattern in [*wildcard_patterns, *re.findall(r"[^\s,]+", literal_patterns)]
    }


def _matches_filter_out(path: Path, patterns: set[str]) -> bool:
    candidate = _normalize_posix_project_path(path.as_posix())
    return any(
        fnmatch.fnmatchcase(candidate, pattern.replace("%", "?*"))
        for pattern in patterns
    )

def parse_make_sources(makefile: Path, source_root: Path) -> set[Path]:
    """Return normalized .c/.s/.S paths selected by explicit lists and LVGL wildcards."""
    lines = _logical_lines(makefile.read_text(encoding="utf-8"))
    variables = {
        match.group(1): match.group(2)
        for line in lines
        if (match := re.match(r"\s*(\w+)\s*=\s*(\S+)\s*$", line))
    }
    sources: set[Path] = set()
    for line in lines:
        if not re.match(r"\s*(?:C_SOURCES|ASM_SOURCES(?:_S)?)\s*(?:\+)?=", line):
            continue
        expression = _expand_variables(line, variables)
        excluded_patterns = _filter_out_patterns(expression)
        candidates: set[Path] = set()
        for pattern in _wildcard_patterns(expression):
            candidates.update(_glob_sources(source_root, pattern))
        explicit = re.sub(r"\$\([^)]*\)", " ", expression)
        for token in re.findall(r"[^\s,]+\.(?:c|s|S)\b", explicit):
            relative = _relative_project_path(source_root / token, source_root)
            if relative is not None:
                candidates.add(relative)
        sources.update(
            candidate for candidate in candidates
            if not _matches_filter_out(candidate, excluded_patterns)
        )
    return sources

def parse_compile_commands(path: Path, source_root: Path) -> set[Path]:
    """Return normalized project-local source paths from compile_commands.json."""
    commands = json.loads(path.read_text(encoding="utf-8"))
    sources: set[Path] = set()
    for command in commands:
        source_file = Path(command["file"])
        if source_file.suffix not in SOURCE_SUFFIXES:
            continue
        if not source_file.is_absolute():
            source_file = Path(command.get("directory", source_root)) / source_file
        relative = _relative_project_path(source_file, source_root)
        if relative is not None:
            sources.add(relative)
    return sources

def compare_sources(expected: set[Path], actual: set[Path]) -> tuple[set[Path], set[Path]]:
    return expected - actual, actual - expected

def _is_excluded(path: Path) -> bool:
    return any(substring in path.as_posix() for substring in EXCLUDED_PATH_SUBSTRINGS)

def _print_category(title: str, paths: set[Path]) -> None:
    print(f"{title}:")
    for path in sorted(paths, key=lambda item: item.as_posix()):
        print(f"  {path.as_posix()}")

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    expected_group = parser.add_mutually_exclusive_group(required=True)
    expected_group.add_argument("--makefile", type=Path)
    expected_group.add_argument("--expected-json", type=Path)
    parser.add_argument("--compile-commands", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    if args.makefile is not None and not args.makefile.is_file():
        parser.error(f"Makefile not found: {args.makefile}")
    if args.expected_json is not None and not args.expected_json.is_file():
        parser.error(f"expected source JSON not found: {args.expected_json}")
    if not args.compile_commands.is_file():
        parser.error(f"compile_commands.json not found: {args.compile_commands}")
    if not args.source_root.is_dir():
        parser.error(f"source root not found: {args.source_root}")
    if args.makefile is not None:
        expected = parse_make_sources(args.makefile, args.source_root)
    else:
        payload = json.loads(args.expected_json.read_text(encoding="utf-8"))
        if payload.get("schema") != 1 or not isinstance(payload.get("sources"), list):
            parser.error(f"unsupported expected source JSON: {args.expected_json}")
        expected = {Path(item) for item in payload["sources"]}
    actual = parse_compile_commands(args.compile_commands, args.source_root)
    excluded = {path for path in actual if _is_excluded(path)}
    missing, unexpected = compare_sources(expected, actual - excluded)
    _print_category("Missing from CMake", missing)
    _print_category("Unexpected in CMake", unexpected)
    _print_category("Excluded source compiled", excluded)
    return int(bool(missing or unexpected or excluded))

if __name__ == "__main__":
    raise SystemExit(main())
