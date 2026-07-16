#!/usr/bin/env python3

"""Validate, compress, render, and compare London 1940 v1 map artifacts."""

from __future__ import annotations

import argparse
import gzip
import hashlib
import importlib.util
import json
import struct
import sys
import zlib
from pathlib import Path
from typing import Any

try:
    from jsonschema import Draft202012Validator
except ImportError as error:  # pragma: no cover - exercised by CLI environments
    raise SystemExit("jsonschema is required; install requirements-test.txt") from error

ROOT = Path(__file__).resolve().parents[1]
MIGRATE_SPEC = importlib.util.spec_from_file_location("map_migrate", ROOT / "tools" / "map_migrate.py")
MAP_MIGRATE = importlib.util.module_from_spec(MIGRATE_SPEC)
assert MIGRATE_SPEC.loader is not None
MIGRATE_SPEC.loader.exec_module(MAP_MIGRATE)


class HarnessError(ValueError):
    pass


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def deterministic_gzip(data: bytes) -> bytes:
    # gzip.compress with mtime=0 is deterministic and emits no source filename.
    return gzip.compress(data, compresslevel=9, mtime=0)


def png_chunk(kind: bytes, data: bytes) -> bytes:
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)


def encode_gray_png(width: int, height: int, pixels: bytearray) -> bytes:
    if len(pixels) != width * height:
        raise HarnessError("grayscale pixel buffer has the wrong size")
    rows = b"".join(b"\x00" + bytes(pixels[y * width:(y + 1) * width]) for y in range(height))
    signature = b"\x89PNG\r\n\x1a\n"
    header = struct.pack(">IIBBBBB", width, height, 8, 0, 0, 0, 0)
    return signature + png_chunk(b"IHDR", header) + png_chunk(b"IDAT", zlib.compress(rows, 9)) + png_chunk(b"IEND", b"")


def parse_png(data: bytes, label: str) -> tuple[int, int, int, list[bytes]]:
    if len(data) < 33 or data[:8] != b"\x89PNG\r\n\x1a\n":
        raise HarnessError(f"{label} is not a complete PNG")
    position = 8
    width = height = color_type = 0
    idat: list[bytes] = []
    saw_header = saw_end = False
    while position < len(data):
        if position + 12 > len(data):
            raise HarnessError(f"{label} has a truncated chunk")
        length = struct.unpack(">I", data[position:position + 4])[0]
        kind = data[position + 4:position + 8]
        end = position + 12 + length
        if end > len(data):
            raise HarnessError(f"{label} has a truncated {kind.decode('ascii', 'replace')} chunk")
        payload = data[position + 8:position + 8 + length]
        expected_crc = struct.unpack(">I", data[position + 8 + length:end])[0]
        if zlib.crc32(kind + payload) & 0xFFFFFFFF != expected_crc:
            raise HarnessError(f"{label} has a corrupt {kind.decode('ascii', 'replace')} chunk")
        if kind == b"IHDR":
            if saw_header or length != 13 or position != 8:
                raise HarnessError(f"{label} has an invalid IHDR")
            width, height, depth, color_type, compression, filtering, interlace = struct.unpack(">IIBBBBB", payload)
            if width < 1 or height < 1 or depth != 8 or compression != 0 or filtering != 0 or interlace not in (0, 1):
                raise HarnessError(f"{label} uses unsupported or invalid PNG metadata")
            saw_header = True
        elif kind == b"IDAT":
            idat.append(payload)
        elif kind == b"IEND":
            if length != 0:
                raise HarnessError(f"{label} has an invalid IEND")
            saw_end = True
            if end != len(data):
                raise HarnessError(f"{label} has bytes after IEND")
        position = end
    if not saw_header or not saw_end or not idat:
        raise HarnessError(f"{label} is truncated")
    return width, height, color_type, idat


def validate_png(path: Path) -> tuple[int, int]:
    width, height, _, _ = parse_png(path.read_bytes(), str(path))
    return width, height


def decode_reference_mask(path: Path) -> tuple[int, int, bytearray]:
    width, height, color_type, chunks = parse_png(path.read_bytes(), str(path))
    if color_type != 0:
        raise HarnessError(f"{path} is not an 8-bit grayscale reference mask")
    raw = zlib.decompress(b"".join(chunks))
    stride = width + 1
    if len(raw) != stride * height:
        raise HarnessError(f"{path} has an unexpected decoded size")
    pixels = bytearray()
    for y in range(height):
        row = raw[y * stride:(y + 1) * stride]
        if row[0] != 0:
            raise HarnessError(f"{path} uses a non-zero PNG row filter")
        pixels.extend(row[1:])
    return width, height, pixels


def set_pixel(mask: bytearray, width: int, height: int, x: int, y: int, value: int = 255) -> None:
    if 0 <= x < width and 0 <= y < height:
        mask[y * width + x] = value


def draw_disk(mask: bytearray, width: int, height: int, x: int, y: int, radius: int) -> None:
    radius_squared = radius * radius
    for yy in range(y - radius, y + radius + 1):
        for xx in range(x - radius, x + radius + 1):
            if (xx - x) ** 2 + (yy - y) ** 2 <= radius_squared:
                set_pixel(mask, width, height, xx, yy)


def draw_line(mask: bytearray, width: int, height: int, start: list[int], end: list[int], radius: int) -> None:
    x0, y0 = start
    x1, y1 = end
    dx = abs(x1 - x0)
    sx = 1 if x0 < x1 else -1
    dy = -abs(y1 - y0)
    sy = 1 if y0 < y1 else -1
    error = dx + dy
    while True:
        draw_disk(mask, width, height, x0, y0, radius)
        if x0 == x1 and y0 == y1:
            break
        doubled = 2 * error
        if doubled >= dy:
            error += dy
            x0 += sx
        if doubled <= dx:
            error += dx
            y0 += sy


def fill_ring(mask: bytearray, width: int, height: int, ring: list[list[int]], value: int) -> None:
    minimum_y = max(0, min(point[1] for point in ring))
    maximum_y = min(height - 1, max(point[1] for point in ring))
    for y in range(minimum_y, maximum_y + 1):
        scan_y = y + 0.5
        intersections: list[float] = []
        previous = ring[-1]
        for current in ring:
            x1, y1 = previous
            x2, y2 = current
            if (y1 <= scan_y < y2) or (y2 <= scan_y < y1):
                intersections.append(x1 + (scan_y - y1) * (x2 - x1) / (y2 - y1))
            previous = current
        intersections.sort()
        for index in range(0, len(intersections) - 1, 2):
            start = max(0, int(intersections[index] + 0.999999))
            end = min(width - 1, int(intersections[index + 1]))
            for x in range(start, end + 1):
                set_pixel(mask, width, height, x, y, value)


def local_pair(pair: list[int], offset: list[int]) -> list[int]:
    return [pair[0] - offset[0], pair[1] - offset[1]]


def normalized_ring(ring: list[list[int]]) -> list[list[int]]:
    result: list[list[int]] = []
    for point in ring:
        if not result or point != result[-1]:
            result.append(point)
    if len(result) > 1 and result[0] == result[-1]:
        result.pop()
    return result


def ring_issue(ring: list[list[int]]) -> str | None:
    points = normalized_ring(ring)
    if len({tuple(point) for point in points}) < 3:
        return "fewer-than-three-distinct-points"
    doubled_area = sum(
        points[index][0] * points[(index + 1) % len(points)][1]
        - points[(index + 1) % len(points)][0] * points[index][1]
        for index in range(len(points))
    )
    if doubled_area == 0:
        return "zero-area"
    return None


def point_in_ring(point: list[int], ring: list[list[int]]) -> bool:
    x, y = point
    inside = False
    previous = ring[-1]
    for current in ring:
        x1, y1 = previous
        x2, y2 = current
        if (y1 > y) != (y2 > y):
            intersection = (x2 - x1) * (y - y1) / (y2 - y1) + x1
            if x < intersection:
                inside = not inside
        previous = current
    return inside


def render_layer(layer: dict[str, Any], width: int, height: int, offset: list[int]) -> bytearray:
    mask = bytearray(width * height)
    geometry = layer["geometry"]
    if geometry == "polygons":
        for polygon in layer["polygons"]:
            rings = polygon["rings"]
            if ring_issue(rings[0]) is not None:
                continue
            fill_ring(mask, width, height, [local_pair(point, offset) for point in rings[0]], 255)
            for hole in rings[1:]:
                if ring_issue(hole) is not None or not point_in_ring(normalized_ring(hole)[0], normalized_ring(rings[0])):
                    continue
                fill_ring(mask, width, height, [local_pair(point, offset) for point in hole], 0)
    elif geometry == "points":
        for point in layer["points"]:
            x, y = local_pair(point, offset)
            draw_disk(mask, width, height, x, y, 2)
    elif geometry == "point-directions":
        for item in layer["items"]:
            x, y = local_pair(item["point"], offset)
            draw_disk(mask, width, height, x, y, 2)
    elif geometry == "network":
        points = [local_pair(point, offset) for point in layer["points"]]
        drawn: set[tuple[int, int]] = set()
        for point_index, endpoints in enumerate(layer["links"]):
            for endpoint in endpoints:
                if endpoint < 0:
                    continue
                edge = tuple(sorted((point_index, endpoint)))
                if edge in drawn:
                    continue
                drawn.add(edge)
                radius = 1
                if "widths" in layer:
                    radius = max(1, min(20, (layer["widths"][point_index] + layer["widths"][endpoint]) // 4))
                draw_line(mask, width, height, points[point_index], points[endpoint], radius)
        for point in points:
            draw_disk(mask, width, height, point[0], point[1], 1)
    return mask


def mask_metrics(actual: bytes | bytearray, expected: bytes | bytearray) -> dict[str, Any]:
    if len(actual) != len(expected):
        raise HarnessError("mask sizes differ")
    true_positive = false_positive = false_negative = 0
    for actual_value, expected_value in zip(actual, expected):
        actual_set = actual_value != 0
        expected_set = expected_value != 0
        true_positive += int(actual_set and expected_set)
        false_positive += int(actual_set and not expected_set)
        false_negative += int(not actual_set and expected_set)
    precision = true_positive / (true_positive + false_positive) if true_positive + false_positive else 1.0
    recall = true_positive / (true_positive + false_negative) if true_positive + false_negative else 1.0
    union = true_positive + false_positive + false_negative
    iou = true_positive / union if union else 1.0
    return {
        "truePositive": true_positive,
        "falsePositive": false_positive,
        "falseNegative": false_negative,
        "precision": precision,
        "recall": recall,
        "intersectionOverUnion": iou,
        "exact": false_positive == 0 and false_negative == 0,
    }


def semantic_invariants(document: dict[str, Any]) -> dict[str, dict[str, int]]:
    MAP_MIGRATE.validate_v1(document)
    offset_x, offset_y = document["offset"]
    width, height = document["resolution"]
    minimum_x, maximum_x = offset_x, offset_x + width - 1
    minimum_y, maximum_y = offset_y, offset_y + height - 1

    def check_point(point: list[int], path: str) -> None:
        if not (minimum_x <= point[0] <= maximum_x and minimum_y <= point[1] <= maximum_y):
            raise HarnessError(f"{path} lies outside declared offset/resolution")

    topology: dict[str, dict[str, int]] = {}
    for layer in document["layers"]:
        topology[layer["id"]] = {"validRings": 0, "quarantinedRings": 0, "interiorRings": 0}
        if layer["geometry"] == "polygons":
            for polygon_index, polygon in enumerate(layer["polygons"]):
                for ring_index, ring in enumerate(polygon["rings"]):
                    for point_index, point in enumerate(ring):
                        check_point(point, f"{layer['id']}.polygons[{polygon_index}].rings[{ring_index}][{point_index}]")
                    issue = ring_issue(ring)
                    if ring_index > 0 and issue is None and not point_in_ring(normalized_ring(ring)[0], normalized_ring(polygon["rings"][0])):
                        issue = "outside-exterior"
                        topology[layer["id"]]["misplacedInteriorRings"] = topology[layer["id"]].get("misplacedInteriorRings", 0) + 1
                    if issue is None:
                        topology[layer["id"]]["validRings"] += 1
                    else:
                        topology[layer["id"]]["quarantinedRings"] += 1
                    if ring_index > 0:
                        topology[layer["id"]]["interiorRings"] += 1
        elif layer["geometry"] in {"points", "network"}:
            for point_index, point in enumerate(layer["points"]):
                check_point(point, f"{layer['id']}.points[{point_index}]")
        else:
            for item_index, item in enumerate(layer["items"]):
                check_point(item["point"], f"{layer['id']}.items[{item_index}].point")
    return topology


def load_expectations(path: Path | None, fixture_name: str) -> dict[str, Any] | None:
    if path is None:
        return None
    document = json.loads(path.read_text())
    if fixture_name not in document.get("fixtures", {}):
        raise HarnessError(f"expectations do not contain fixture {fixture_name}")
    return document["fixtures"][fixture_name]


def validate_map(input_path: Path, schema_path: Path, output_dir: Path, expectations_path: Path | None) -> dict[str, Any]:
    if expectations_path is not None:
        expectations_path = expectations_path.resolve()
    try:
        document = json.loads(input_path.read_text())
    except json.JSONDecodeError as error:
        raise HarnessError(f"invalid JSON: {error}") from error
    schema = json.loads(schema_path.read_text())
    Draft202012Validator.check_schema(schema)
    errors = sorted(Draft202012Validator(schema).iter_errors(document), key=lambda error: list(error.path))
    if errors:
        raise HarnessError("schema validation failed: " + "; ".join(error.message for error in errors[:5]))
    topology = semantic_invariants(document)

    canonical = MAP_MIGRATE.canonical_bytes(document)
    reloaded = json.loads(canonical)
    if reloaded != document or MAP_MIGRATE.canonical_bytes(reloaded) != canonical:
        raise HarnessError("canonical JSON data-model or byte round-trip failed")
    compressed = deterministic_gzip(canonical)
    if gzip.decompress(compressed) != canonical or deterministic_gzip(canonical) != compressed:
        raise HarnessError("deterministic gzip round-trip failed")

    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "map.canonical.json").write_bytes(canonical)
    (output_dir / "map.canonical.json.gz").write_bytes(compressed)

    fixture_name = input_path.stem.removesuffix(".v1")
    expectations = load_expectations(expectations_path, fixture_name)
    expected_layers = expectations.get("layers", {}) if expectations else {}
    report_layers: dict[str, Any] = {}
    width, height = document["resolution"]
    mask_dir = output_dir / "masks"
    mask_dir.mkdir(exist_ok=True)
    counts = MAP_MIGRATE.feature_counts(document)
    for layer in document["layers"]:
        mask = render_layer(layer, width, height, document["offset"])
        encoded = encode_gray_png(width, height, mask)
        mask_path = mask_dir / f"{layer['id']}.png"
        mask_path.write_bytes(encoded)
        mask_hash = sha256_bytes(bytes(mask))
        nonzero = sum(value != 0 for value in mask)
        layer_report: dict[str, Any] = {
            "counts": counts[layer["id"]],
            "maskSha256": mask_hash,
            "nonzeroPixels": nonzero,
            "topology": topology[layer["id"]],
        }
        if layer["id"] in expected_layers:
            expected = expected_layers[layer["id"]]
            minimum = expected.get("minimumFeatures", {})
            for count_name, minimum_value in minimum.items():
                if counts[layer["id"]].get(count_name, 0) < minimum_value:
                    raise HarnessError(f"{layer['id']} {count_name} fell below expected minimum")
            expected_hash = expected.get("maskSha256")
            if expected_hash and mask_hash != expected_hash:
                raise HarnessError(f"{layer['id']} mask hash differs from reviewed fixture")
            if "topology" in expected and topology[layer["id"]] != expected["topology"]:
                raise HarnessError(f"{layer['id']} topology counts differ from reviewed fixture")
            reference_path = expectations_path.parent / "reference-masks" / fixture_name / f"{layer['id']}.png"
            reference_width, reference_height, reference_pixels = decode_reference_mask(reference_path)
            if (reference_width, reference_height) != (width, height):
                raise HarnessError(f"{layer['id']} reference-mask dimensions differ")
            metrics = mask_metrics(mask, reference_pixels)
            if not metrics["exact"]:
                raise HarnessError(f"{layer['id']} differs from its independent reference mask")
            layer_report["expected"] = True
            layer_report["referenceMask"] = str(reference_path.relative_to(ROOT))
            layer_report["metrics"] = metrics
        report_layers[layer["id"]] = layer_report

    if expectations:
        missing = set(expected_layers) - set(report_layers)
        if missing:
            raise HarnessError(f"expected layers are missing: {sorted(missing)}")

    report = {
        "format": "london1940-validation-report",
        "version": 1,
        "input": input_path.name,
        "canonicalSha256": sha256_bytes(canonical),
        "gzipSha256": sha256_bytes(compressed),
        "canonicalBytes": len(canonical),
        "gzipBytes": len(compressed),
        "schema": "draft-2020-12",
        "layers": report_layers,
        "valid": True,
    }
    (output_dir / "report.json").write_bytes(MAP_MIGRATE.canonical_bytes(report))
    return report


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    validate_parser = subparsers.add_parser("validate-map")
    validate_parser.add_argument("input", type=Path)
    validate_parser.add_argument("--schema", type=Path, default=ROOT / "schema" / "map-v1.schema.json")
    validate_parser.add_argument("--output-dir", type=Path, required=True)
    validate_parser.add_argument("--expectations", type=Path)
    png_parser = subparsers.add_parser("validate-png")
    png_parser.add_argument("input", type=Path)
    compare_parser = subparsers.add_parser("compare-raw-masks")
    compare_parser.add_argument("actual", type=Path)
    compare_parser.add_argument("expected", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        if args.command == "validate-map":
            report = validate_map(args.input, args.schema, args.output_dir, args.expectations)
            print(json.dumps(report, sort_keys=True, indent=2))
        elif args.command == "validate-png":
            width, height = validate_png(args.input)
            print(f"valid PNG {width}x{height} {args.input}")
        else:
            metrics = mask_metrics(args.actual.read_bytes(), args.expected.read_bytes())
            print(json.dumps(metrics, sort_keys=True, indent=2))
    except (OSError, HarnessError, MAP_MIGRATE.MapValidationError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
