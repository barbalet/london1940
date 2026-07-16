#!/usr/bin/env python3

"""Checked public driver for the London 1940 map extractor."""

from __future__ import annotations

import argparse
import importlib.util
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

VERSION = "2.0.0"
SCRIPT_PATH = Path(__file__).resolve()
SOURCE_ROOT = SCRIPT_PATH.parents[1]
if (SOURCE_ROOT / "tools" / "map_migrate.py").is_file():
    ROOT = SOURCE_ROOT
    MIGRATE_PATH = ROOT / "tools" / "map_migrate.py"
    DEFAULT_ENGINE = ROOT / "map" / "map2json-core"
else:
    PREFIX = SCRIPT_PATH.parents[1]
    ROOT = PREFIX
    MIGRATE_PATH = PREFIX / "libexec" / "london1940" / "map_migrate.py"
    DEFAULT_ENGINE = PREFIX / "libexec" / "london1940" / "map2json-core"

MIGRATE_SPEC = importlib.util.spec_from_file_location("map_migrate", MIGRATE_PATH)
MAP_MIGRATE = importlib.util.module_from_spec(MIGRATE_SPEC)
assert MIGRATE_SPEC.loader is not None
MIGRATE_SPEC.loader.exec_module(MAP_MIGRATE)


class PipelineError(RuntimeError):
    pass


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(
        prog="map2json",
        description="Extract a source PNG into validated london1940-map v1 JSON.",
    )
    result.add_argument("input", nargs="?", type=Path, help="source map PNG")
    result.add_argument("-f", "--filename", dest="filename", type=Path, help=argparse.SUPPRESS)
    result.add_argument("-o", "--output", type=Path, required=False, help="v1 JSON destination")
    result.add_argument("--work-dir", type=Path, help="parent directory for isolated scratch data")
    result.add_argument("--diagnostics", action="store_true", help="retain engine logs and diagnostic images")
    result.add_argument("--profile", choices=("legacy-1",), default="legacy-1", help="versioned extraction profile")
    result.add_argument("--format-version", type=int, choices=(1,), default=1)
    result.add_argument("--metadata", type=Path, help="explicit JSON georeferencing metadata")
    result.add_argument("--engine", type=Path, default=DEFAULT_ENGINE, help=argparse.SUPPRESS)
    result.add_argument("--version", action="version", version=f"%(prog)s {VERSION}")
    return result


def checked_input(args: argparse.Namespace) -> Path:
    if args.input and args.filename:
        raise PipelineError("specify the input once, either positionally or with --filename")
    source = args.input or args.filename
    if source is None:
        raise PipelineError("a source PNG is required")
    source = source.resolve()
    if not source.is_file():
        raise PipelineError(f"source does not exist: {source}")
    if source.suffix.lower() != ".png":
        raise PipelineError("Phase 3 accepts PNG extraction inputs only")
    return source


def extraction_limits(width: int, height: int) -> list[str]:
    pixels = width * height
    # These bounds scale with the actual image while retaining ample headroom
    # over every Phase 0 fixture. The C engine rejects overflow at its existing
    # safety checks instead of reserving its historical multi-million entries.
    polygon_points = max(100_000, pixels // 8)
    road_points = max(25_000, pixels // 64)
    junctions = max(2_000, pixels // 512)
    return [
        "--maxpolypts", str(polygon_points),
        "--maxroadpts", str(road_points),
        "--maxjunctions", str(junctions),
        "--maxstations", "1000",
        "--maxbridges", "1000",
    ]


def png_dimensions(path: Path) -> tuple[int, int]:
    data = path.read_bytes()[:24]
    if len(data) != 24 or data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        raise PipelineError(f"not a complete PNG with an IHDR header: {path}")
    width = int.from_bytes(data[16:20], "big")
    height = int.from_bytes(data[20:24], "big")
    if width < 1 or height < 1:
        raise PipelineError("PNG dimensions must be positive")
    return width, height


def apply_metadata(document: dict, path: Path | None) -> None:
    if path is None:
        return
    try:
        metadata = json.loads(path.read_text())
    except json.JSONDecodeError as error:
        raise PipelineError(f"invalid metadata JSON: {error}") from error
    if not isinstance(metadata, dict) or not metadata or set(metadata) - {"tile", "geographicBounds"}:
        raise PipelineError("metadata must contain only tile and/or geographicBounds")
    if "tile" in metadata:
        document["tile"] = metadata["tile"]
    if "geographicBounds" in metadata:
        bounds = metadata["geographicBounds"]
        if not isinstance(bounds, dict):
            raise PipelineError("metadata geographicBounds must be an object")
        document["geographicBounds"] = bounds
        document["coordinates"]["crs"] = bounds.get("crs")


def run_pipeline(args: argparse.Namespace) -> Path:
    source = checked_input(args)
    output = (args.output or source.with_suffix(".v1.json")).resolve()
    engine = args.engine.resolve()
    if not engine.is_file():
        raise PipelineError(f"extractor engine is missing; run make -C map ({engine})")
    width, height = png_dimensions(source)
    work_parent = args.work_dir.resolve() if args.work_dir else None
    if work_parent:
        work_parent.mkdir(parents=True, exist_ok=True)
    scratch = Path(tempfile.mkdtemp(prefix="map2json-", dir=work_parent))
    legacy_path = scratch / "map.json"
    log_path = scratch / "extractor.log"
    command = [str(engine), "-f", str(source), *extraction_limits(width, height), "--apesdk"]
    try:
        environment = os.environ.copy()
        environment["MAP2JSON_EXTRACT_ONLY"] = "1"
        if not args.diagnostics:
            environment["MAP2JSON_SUPPRESS_PNG"] = "1"
        completed = subprocess.run(command, cwd=scratch, env=environment, text=True, stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT, check=False)
        log_path.write_text(completed.stdout)
        if completed.returncode != 0:
            raise PipelineError(f"extractor failed with status {completed.returncode}; see {log_path}")
        if not legacy_path.is_file():
            raise PipelineError(f"extractor reported success without producing map.json; see {log_path}")
        try:
            legacy = json.loads(legacy_path.read_text())
            document = MAP_MIGRATE.migrate_legacy(legacy, source.parent)
            MAP_MIGRATE.validate_v1(document)
        except (json.JSONDecodeError, MAP_MIGRATE.MapValidationError, KeyError, TypeError, ValueError) as error:
            raise PipelineError(f"extractor output is invalid: {error}") from error
        document["generator"] = {"name": "map2json", "version": VERSION, "extractionProfile": args.profile}
        apply_metadata(document, args.metadata)
        MAP_MIGRATE.validate_v1(document)
        output.parent.mkdir(parents=True, exist_ok=True)
        temporary_output = output.with_name(f".{output.name}.tmp")
        try:
            temporary_output.write_bytes(MAP_MIGRATE.canonical_bytes(document))
            reloaded = json.loads(temporary_output.read_text())
            MAP_MIGRATE.validate_v1(reloaded)
            if MAP_MIGRATE.canonical_bytes(reloaded) != temporary_output.read_bytes():
                raise PipelineError("requested output failed canonical self-validation")
            temporary_output.replace(output)
        finally:
            temporary_output.unlink(missing_ok=True)
        if args.diagnostics:
            print(f"diagnostics: {scratch}", file=sys.stderr)
        return output
    finally:
        if not args.diagnostics:
            shutil.rmtree(scratch, ignore_errors=True)


def main() -> int:
    args = parser().parse_args()
    try:
        output = run_pipeline(args)
    except (OSError, PipelineError) as error:
        print(f"map2json: error: {error}", file=sys.stderr)
        return 1
    print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
