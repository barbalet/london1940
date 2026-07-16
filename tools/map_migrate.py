#!/usr/bin/env python3

"""Migrate legacy London 1940 map JSON into canonical version 1 JSON."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any

FORMAT = "london1940-map"
VERSION = 1
NETWORK_TYPES = {"rivers", "harbours", "railway lines", "railway tunnels", "main roads", "minor roads"}
POLYGON_TYPES = {"woodland", "lakes", "sands", "sea", "buildings"}
POINT_DIRECTION_TYPES = {"bridges"}
SLUG_RE = re.compile(r"[^a-z0-9]+")


class MapValidationError(ValueError):
    pass


def canonical_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n").encode("utf-8")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise MapValidationError(message)


def is_integer(value: Any) -> bool:
    return isinstance(value, int) and not isinstance(value, bool)


def validate_pair(value: Any, path: str, minimum: int | None = None) -> None:
    require(isinstance(value, list) and len(value) == 2, f"{path} must be a two-item array")
    for index, item in enumerate(value):
        require(is_integer(item), f"{path}[{index}] must be an integer")
        if minimum is not None:
            require(item >= minimum, f"{path}[{index}] must be at least {minimum}")


def validate_v1(document: Any) -> None:
    require(isinstance(document, dict), "document must be an object")
    required = {"format", "version", "source", "generator", "attribution", "encoding", "coordinates", "tile", "offset", "resolution", "layers"}
    allowed = required | {"geographicBounds"}
    require(set(document) >= required, f"missing top-level keys: {sorted(required - set(document))}")
    require(set(document) <= allowed, f"unknown top-level keys: {sorted(set(document) - allowed)}")
    require(document["format"] == FORMAT, f"format must be {FORMAT}")
    require(document["version"] == VERSION, f"version must be {VERSION}")

    source = document["source"]
    require(isinstance(source, dict) and set(source) == {"filename", "sha256", "legacyFormat"}, "source has invalid keys")
    require(isinstance(source["filename"], str) and source["filename"], "source.filename must be non-empty")
    require(source["legacyFormat"] in {"apesdk-array", "keyed-object", "v1"}, "invalid source.legacyFormat")
    require(source["sha256"] is None or (isinstance(source["sha256"], str) and re.fullmatch(r"[0-9a-f]{64}", source["sha256"])), "invalid source.sha256")

    generator = document["generator"]
    require(isinstance(generator, dict) and set(generator) == {"name", "version", "extractionProfile"}, "generator has invalid keys")
    require(isinstance(generator["name"], str) and generator["name"], "generator.name must be non-empty")
    require(generator["version"] is None or isinstance(generator["version"], str), "generator.version must be a string or null")
    require(isinstance(generator["extractionProfile"], str) and generator["extractionProfile"], "generator.extractionProfile must be non-empty")
    attribution = document["attribution"]
    require(isinstance(attribution, dict) and set(attribution) == {"license", "text"}, "attribution has invalid keys")
    require(attribution["license"] == "CC-BY-4.0", "attribution.license must be CC-BY-4.0")
    require(isinstance(attribution["text"], str) and attribution["text"], "attribution.text must be non-empty")

    require(document["encoding"] == {"coordinates": "absolute-integer", "sharedTables": False}, "unsupported encoding")
    require(document["coordinates"] == {"space": "sheet-pixel", "units": "pixel", "origin": "top-left", "yAxis": "down", "crs": None} or (
        isinstance(document["coordinates"], dict)
        and set(document["coordinates"]) == {"space", "units", "origin", "yAxis", "crs"}
        and document["coordinates"]["space"] == "sheet-pixel"
        and document["coordinates"]["units"] == "pixel"
        and document["coordinates"]["origin"] == "top-left"
        and document["coordinates"]["yAxis"] == "down"
        and (document["coordinates"]["crs"] is None or isinstance(document["coordinates"]["crs"], str))
    ), "invalid coordinates metadata")
    validate_pair(document["tile"], "tile")
    validate_pair(document["offset"], "offset")
    validate_pair(document["resolution"], "resolution", 1)
    if "geographicBounds" in document:
        bounds = document["geographicBounds"]
        require(isinstance(bounds, dict) and set(bounds) == {"crs", "topLeft", "bottomRight"}, "geographicBounds has invalid keys")
        require(bounds["crs"] == "EPSG:4326", "geographicBounds.crs must be EPSG:4326")
        for corner_name in ("topLeft", "bottomRight"):
            corner = bounds[corner_name]
            require(isinstance(corner, dict) and set(corner) == {"longitude", "latitude"}, f"geographicBounds.{corner_name} has invalid keys")
            longitude = corner["longitude"]
            latitude = corner["latitude"]
            require(isinstance(longitude, (int, float)) and not isinstance(longitude, bool) and -180 <= longitude <= 180, f"geographicBounds.{corner_name}.longitude is invalid")
            require(isinstance(latitude, (int, float)) and not isinstance(latitude, bool) and -90 <= latitude <= 90, f"geographicBounds.{corner_name}.latitude is invalid")

    layers = document["layers"]
    require(isinstance(layers, list), "layers must be an array")
    layer_ids: set[str] = set()
    for layer_index, layer in enumerate(layers):
        path = f"layers[{layer_index}]"
        require(isinstance(layer, dict), f"{path} must be an object")
        for key in ("id", "category", "type", "geometry"):
            require(key in layer, f"{path}.{key} is required")
        require(layer["category"] in {"terrain", "urban"}, f"{path}.category is invalid")
        require(layer["id"] == layer_id(layer["category"], layer["type"]), f"{path}.id is not canonical")
        require(layer["id"] not in layer_ids, f"duplicate layer id {layer['id']}")
        layer_ids.add(layer["id"])
        geometry = layer["geometry"]

        if geometry == "polygons":
            require(set(layer) == {"id", "category", "type", "geometry", "polygons"}, f"{path} has invalid polygon keys")
            require(isinstance(layer["polygons"], list), f"{path}.polygons must be an array")
            for polygon_index, polygon in enumerate(layer["polygons"]):
                polygon_path = f"{path}.polygons[{polygon_index}]"
                require(isinstance(polygon, dict) and set(polygon) in ({"rings"}, {"rings", "legacyId"}), f"{polygon_path} has invalid keys")
                require(isinstance(polygon["rings"], list) and polygon["rings"], f"{polygon_path}.rings must not be empty")
                for ring_index, ring in enumerate(polygon["rings"]):
                    require(isinstance(ring, list) and len(ring) >= 3, f"{polygon_path}.rings[{ring_index}] requires at least three points")
                    for point_index, point in enumerate(ring):
                        validate_pair(point, f"{polygon_path}.rings[{ring_index}][{point_index}]")
        elif geometry == "points":
            require(set(layer) == {"id", "category", "type", "geometry", "points"}, f"{path} has invalid point keys")
            require(isinstance(layer["points"], list), f"{path}.points must be an array")
            for point_index, point in enumerate(layer["points"]):
                validate_pair(point, f"{path}.points[{point_index}]")
        elif geometry == "network":
            require(set(layer) in ({"id", "category", "type", "geometry", "points", "links"}, {"id", "category", "type", "geometry", "points", "links", "widths"}), f"{path} has invalid network keys")
            points = layer["points"]
            links = layer["links"]
            require(isinstance(points, list) and isinstance(links, list), f"{path} points and links must be arrays")
            require(len(points) == len(links), f"{path} must have one link pair per point")
            for point_index, point in enumerate(points):
                validate_pair(point, f"{path}.points[{point_index}]")
            for link_index, link in enumerate(links):
                validate_pair(link, f"{path}.links[{link_index}]", -1)
                for endpoint in link:
                    require(endpoint == -1 or endpoint < len(points), f"{path}.links[{link_index}] index is out of range")
            if "widths" in layer:
                require(len(layer["widths"]) == len(points), f"{path} must have one width per point")
                require(all(is_integer(width) and width >= 0 for width in layer["widths"]), f"{path}.widths must be non-negative integers")
        elif geometry == "point-directions":
            require(set(layer) == {"id", "category", "type", "geometry", "items"}, f"{path} has invalid point-direction keys")
            require(isinstance(layer["items"], list), f"{path}.items must be an array")
            for item_index, item in enumerate(layer["items"]):
                require(isinstance(item, dict) and set(item) == {"point", "direction"}, f"{path}.items[{item_index}] has invalid keys")
                validate_pair(item["point"], f"{path}.items[{item_index}].point")
                validate_pair(item["direction"], f"{path}.items[{item_index}].direction")
        else:
            raise MapValidationError(f"{path}.geometry is invalid")


def layer_id(category: str, type_name: str) -> str:
    slug = SLUG_RE.sub("-", type_name.lower()).strip("-")
    return f"{category}.{slug}"


def copy_pair(value: Any, path: str) -> list[int]:
    validate_pair(value, path)
    return [value[0], value[1]]


def migrate_polygons(value: Any, path: str) -> tuple[list[dict[str, Any]], str]:
    if isinstance(value, list):
        polygons = []
        for index, ring in enumerate(value):
            require(isinstance(ring, list) and len(ring) >= 3, f"{path}[{index}] is not a ring")
            polygons.append({"rings": [copy_ring(ring, f"{path}[{index}]")]})
        return polygons, "apesdk-array"

    require(isinstance(value, dict), f"{path} must be an array or object")
    polygons = []
    for legacy_id in sorted(value, key=lambda item: (not str(item).isdigit(), int(item) if str(item).isdigit() else str(item))):
        polygon = value[legacy_id]
        require(isinstance(polygon, dict), f"{path}.{legacy_id} must be an object")
        if "ext" in polygon:
            rings = [copy_ring(polygon["ext"], f"{path}.{legacy_id}.ext")]
            if "int" in polygon:
                interiors = polygon["int"]
                if interiors and isinstance(interiors[0], list) and len(interiors[0]) == 2 and all(is_integer(v) for v in interiors[0]):
                    interiors = [interiors]
                require(isinstance(interiors, list), f"{path}.{legacy_id}.int must be a ring or rings")
                rings.extend(copy_ring(ring, f"{path}.{legacy_id}.int") for ring in interiors)
        else:
            require("perimeter" in polygon, f"{path}.{legacy_id} requires ext or perimeter")
            rings = [copy_ring(polygon["perimeter"], f"{path}.{legacy_id}.perimeter")]
        polygons.append({"legacyId": str(legacy_id), "rings": rings})
    return polygons, "keyed-object"


def copy_ring(ring: Any, path: str) -> list[list[int]]:
    require(isinstance(ring, list) and len(ring) >= 3, f"{path} requires at least three points")
    return [copy_pair(point, f"{path}[{index}]") for index, point in enumerate(ring)]


def resolve_source_hash(source_filename: str, source_root: Path | None) -> str | None:
    if source_root is None:
        return None
    candidate = source_root / source_filename
    return sha256_file(candidate) if candidate.is_file() else None


def migrate_legacy(document: Any, source_root: Path | None = None) -> dict[str, Any]:
    if isinstance(document, dict) and document.get("format") == FORMAT:
        validate_v1(document)
        return document
    require(isinstance(document, dict), "legacy document must be an object")
    for key in ("sourceFilename", "tile", "offset", "resolution", "terrain", "urban"):
        require(key in document, f"legacy document is missing {key}")

    detected_formats: set[str] = set()
    layers: list[dict[str, Any]] = []
    for category in ("terrain", "urban"):
        require(isinstance(document[category], list), f"legacy {category} must be an array")
        for legacy_index, legacy_layer in enumerate(document[category]):
            path = f"{category}[{legacy_index}]"
            require(isinstance(legacy_layer, dict) and isinstance(legacy_layer.get("type"), str), f"{path} requires a type")
            type_name = legacy_layer["type"]
            base = {"id": layer_id(category, type_name), "category": category, "type": type_name}
            if type_name in POLYGON_TYPES or "polygons" in legacy_layer:
                polygons, dialect = migrate_polygons(legacy_layer.get("polygons"), f"{path}.polygons")
                detected_formats.add(dialect)
                layer = {**base, "geometry": "polygons", "polygons": polygons}
            elif type_name in POINT_DIRECTION_TYPES or "pointdirections" in legacy_layer:
                values = legacy_layer.get("pointdirections", [])
                require(isinstance(values, list), f"{path}.pointdirections must be an array")
                items = []
                for item_index, item in enumerate(values):
                    require(isinstance(item, list) and len(item) == 2, f"{path}.pointdirections[{item_index}] is invalid")
                    items.append({"point": copy_pair(item[0], f"{path}.pointdirections[{item_index}][0]"), "direction": copy_pair(item[1], f"{path}.pointdirections[{item_index}][1]")})
                layer = {**base, "geometry": "point-directions", "items": items}
            elif type_name in NETWORK_TYPES or "links" in legacy_layer:
                points = [copy_pair(point, f"{path}.points") for point in legacy_layer.get("points", [])]
                links = [copy_pair(link, f"{path}.links") for link in legacy_layer.get("links", [])]
                layer = {**base, "geometry": "network", "points": points, "links": links}
                if "widths" in legacy_layer:
                    layer["widths"] = list(legacy_layer["widths"])
            else:
                points = [copy_pair(point, f"{path}.points") for point in legacy_layer.get("points", [])]
                layer = {**base, "geometry": "points", "points": points}
            layers.append(layer)

    legacy_format = "keyed-object" if "keyed-object" in detected_formats else "apesdk-array"
    result: dict[str, Any] = {
        "format": FORMAT,
        "version": VERSION,
        "source": {
            "filename": document["sourceFilename"],
            "sha256": resolve_source_hash(document["sourceFilename"], source_root),
            "legacyFormat": legacy_format,
        },
        "generator": {
            "name": "legacy-map2json",
            "version": None,
            "extractionProfile": "legacy-default-unknown",
        },
        "attribution": {
            "license": "CC-BY-4.0",
            "text": "Contains derived data from historical Ordnance Survey maps supplied by the National Library of Scotland.",
        },
        "encoding": {"coordinates": "absolute-integer", "sharedTables": False},
        "coordinates": {"space": "sheet-pixel", "units": "pixel", "origin": "top-left", "yAxis": "down", "crs": None},
        "tile": copy_pair(document["tile"], "tile"),
        "offset": copy_pair(document["offset"], "offset"),
        "resolution": copy_pair(document["resolution"], "resolution"),
        "layers": layers,
    }

    geocoords = document.get("geocoords")
    if isinstance(geocoords, dict) and {"topleft", "bottomright"} <= set(geocoords):
        top_left = geocoords["topleft"]
        bottom_right = geocoords["bottomright"]
        values = [top_left.get("longitude"), top_left.get("latitude"), bottom_right.get("longitude"), bottom_right.get("latitude")]
        if all(isinstance(value, (int, float)) for value in values) and any(value != 0 for value in values):
            result["geographicBounds"] = {
                "crs": "EPSG:4326",
                "topLeft": {"longitude": values[0], "latitude": values[1]},
                "bottomRight": {"longitude": values[2], "latitude": values[3]},
            }

    validate_v1(result)
    return result


def feature_counts(document: dict[str, Any]) -> dict[str, dict[str, int]]:
    counts: dict[str, dict[str, int]] = {}
    for layer in document["layers"]:
        if layer["geometry"] == "polygons":
            value = {"polygons": len(layer["polygons"]), "rings": sum(len(poly["rings"]) for poly in layer["polygons"])}
        elif layer["geometry"] == "points":
            value = {"points": len(layer["points"])}
        elif layer["geometry"] == "network":
            value = {"points": len(layer["points"]), "links": len(layer["links"])}
            if "widths" in layer:
                value["widths"] = len(layer["widths"])
        else:
            value = {"items": len(layer["items"])}
        counts[layer["id"]] = value
    return counts


def read_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as source:
        return json.load(source)


def write_canonical(path: Path | None, document: Any) -> None:
    data = canonical_bytes(document)
    if path is None:
        sys.stdout.buffer.write(data)
    else:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    migrate_parser = subparsers.add_parser("migrate", help="migrate legacy JSON to canonical v1")
    migrate_parser.add_argument("input", type=Path)
    migrate_parser.add_argument("--output", "-o", type=Path)
    migrate_parser.add_argument("--source-root", type=Path)
    validate_parser = subparsers.add_parser("validate", help="validate a v1 document")
    validate_parser.add_argument("input", type=Path)
    canonical_parser = subparsers.add_parser("canonicalize", help="validate and canonicalize a v1 document")
    canonical_parser.add_argument("input", type=Path)
    canonical_parser.add_argument("--output", "-o", type=Path)
    counts_parser = subparsers.add_parser("counts", help="print per-layer feature counts")
    counts_parser.add_argument("input", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        document = read_json(args.input)
        if args.command == "migrate":
            migrated = migrate_legacy(document, args.source_root)
            write_canonical(args.output, migrated)
        elif args.command == "validate":
            validate_v1(document)
            print(f"valid {args.input}")
        elif args.command == "canonicalize":
            validate_v1(document)
            write_canonical(args.output, document)
        elif args.command == "counts":
            validate_v1(document)
            print(json.dumps(feature_counts(document), sort_keys=True, indent=2))
    except (OSError, json.JSONDecodeError, MapValidationError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
