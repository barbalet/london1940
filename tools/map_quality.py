#!/usr/bin/env python3

"""Phase 5 quality policy, profile comparison, and overlap helpers."""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
VALIDATE_SPEC = importlib.util.spec_from_file_location("map_validate", ROOT / "tools" / "map_validate.py")
MAP_VALIDATE = importlib.util.module_from_spec(VALIDATE_SPEC)
assert VALIDATE_SPEC.loader is not None
VALIDATE_SPEC.loader.exec_module(MAP_VALIDATE)


class QualityError(ValueError):
    pass


def assess_reports(report_paths: list[Path], policy_path: Path) -> dict[str, Any]:
    policy = json.loads(policy_path.read_text())
    thresholds = policy["realMapClasses"]
    observed: dict[str, int] = {identifier: 0 for identifier in thresholds}
    for path in report_paths:
        report = json.loads(path.read_text())
        for identifier, layer in report["layers"].items():
            if identifier not in thresholds:
                continue
            metrics = layer.get("metrics")
            if not metrics:
                raise QualityError(f"{path}: {identifier} has no reviewed metrics")
            expected = thresholds[identifier]
            for metric, threshold_name in (("precision", "minimumPrecision"), ("recall", "minimumRecall"),
                                           ("intersectionOverUnion", "minimumIoU")):
                if metrics[metric] < expected[threshold_name]:
                    raise QualityError(f"{path}: {identifier} {metric} is below policy")
            observed[identifier] += 1
    missing = sorted(identifier for identifier, count in observed.items() if count == 0)
    if missing:
        raise QualityError(f"reviewed real-map classes missing from reports: {missing}")
    return {"valid": True, "reports": len(report_paths), "observations": observed}


def profile_difference(first: dict[str, Any], second: dict[str, Any]) -> dict[str, Any]:
    if first["resolution"] != second["resolution"] or first["offset"] != second["offset"]:
        raise QualityError("profile outputs use different coordinate frames")
    width, height = first["resolution"]
    first_layers = {layer["id"]: layer for layer in first["layers"]}
    second_layers = {layer["id"]: layer for layer in second["layers"]}
    differences = {}
    for identifier in sorted(set(first_layers) | set(second_layers)):
        left = first_layers.get(identifier)
        right = second_layers.get(identifier)
        if left is None or right is None:
            differences[identifier] = {"presenceChanged": True}
            continue
        left_mask = MAP_VALIDATE.render_layer(left, width, height, first["offset"])
        right_mask = MAP_VALIDATE.render_layer(right, width, height, second["offset"])
        metrics = MAP_VALIDATE.mask_metrics(left_mask, right_mask)
        if not metrics["exact"]:
            differences[identifier] = {
                "metrics": metrics,
                "firstSha256": hashlib.sha256(left_mask).hexdigest(),
                "secondSha256": hashlib.sha256(right_mask).hexdigest(),
            }
    return {
        "format": "london1940-profile-comparison", "version": 1,
        "firstProfile": first["generator"]["extractionProfile"],
        "secondProfile": second["generator"]["extractionProfile"],
        "differences": differences,
    }


def merge_networks(layers: list[dict[str, Any]]) -> dict[str, Any]:
    if not layers:
        raise QualityError("at least one network layer is required")
    template = {key: value for key, value in layers[0].items() if key not in {"points", "links", "widths"}}
    points: list[list[int]] = []
    index_by_point: dict[tuple[int, int], int] = {}
    edges: set[tuple[int, int]] = set()
    widths: dict[int, int] = {}
    for layer in layers:
        local_to_merged = {}
        for index, point in enumerate(layer["points"]):
            key = tuple(point)
            merged = index_by_point.setdefault(key, len(index_by_point))
            if merged == len(points):
                points.append(list(point))
            local_to_merged[index] = merged
            if "widths" in layer:
                widths[merged] = max(widths.get(merged, 0), layer["widths"][index])
        for start, links in enumerate(layer["links"]):
            for end in links:
                if end >= 0 and start != end:
                    edges.add(tuple(sorted((local_to_merged[start], local_to_merged[end]))))
    adjacency = [set() for _ in points]
    for start, end in edges:
        adjacency[start].add(end)
        adjacency[end].add(start)
    result = {**template, "points": points, "links": []}
    for neighbours in adjacency:
        ordered = sorted(neighbours)[:2]
        result["links"].append(ordered + [-1] * (2 - len(ordered)))
    if widths:
        result["widths"] = [widths[index] for index in range(len(points))]
    return result


def simplify_ring(ring: list[list[int]]) -> list[list[int]]:
    points = MAP_VALIDATE.normalized_ring(ring)
    changed = True
    while changed and len(points) > 3:
        changed = False
        for index in range(len(points)):
            previous = points[index - 1]
            current = points[index]
            following = points[(index + 1) % len(points)]
            cross = ((current[0] - previous[0]) * (following[1] - current[1])
                     - (current[1] - previous[1]) * (following[0] - current[0]))
            if cross == 0:
                del points[index]
                changed = True
                break
    if MAP_VALIDATE.ring_issue(points):
        raise QualityError("simplification would create an invalid ring")
    return points


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    assess = subparsers.add_parser("assess")
    assess.add_argument("reports", nargs="+", type=Path)
    assess.add_argument("--policy", type=Path, default=ROOT / "tests/fixtures/quality-thresholds.json")
    compare = subparsers.add_parser("compare-profiles")
    compare.add_argument("first", type=Path)
    compare.add_argument("second", type=Path)
    compare.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        if args.command == "assess":
            result = assess_reports(args.reports, args.policy)
            print(json.dumps(result, sort_keys=True, indent=2))
        else:
            first = json.loads(args.first.read_text())
            second = json.loads(args.second.read_text())
            result = profile_difference(first, second)
            if not result["differences"]:
                raise QualityError("profiles produced no reviewable mask differences")
            args.output.write_text(json.dumps(result, sort_keys=True, indent=2) + "\n")
            print(args.output)
    except (OSError, json.JSONDecodeError, QualityError) as error:
        print(f"map_quality: error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
