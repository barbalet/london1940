#!/usr/bin/env python3

"""Extract a sheet-sized elevation grid from a signed 16-bit big-endian DEM."""

from __future__ import annotations

import argparse
import array
import json
import sys
from pathlib import Path

from pyproj import Transformer

TILES_OSGB36 = {
    160: (495, 225, 535, 180), 161: (535, 225, 575, 180),
    162: (570, 225, 610, 180), 170: (495, 180, 535, 135),
    171: (535, 180, 575, 135), 172: (570, 180, 610, 135),
    173: (602, 173, 642, 128), 182: (495, 145, 535, 100),
    183: (535, 138, 575, 93), 184: (570, 150, 610, 105),
}
NO_DATA = -9999


class HeightmapError(ValueError):
    pass


def coords_from_tile(tile_number: int) -> tuple[float, float, float, float]:
    if tile_number not in TILES_OSGB36:
        raise HeightmapError(f"unsupported tile: {tile_number}")
    left, top, right, bottom = (value * 1000 for value in TILES_OSGB36[tile_number])
    transformer = Transformer.from_crs("EPSG:27700", "EPSG:4326", always_xy=True)
    top_left_longitude, top_left_latitude = transformer.transform(left, top)
    bottom_right_longitude, bottom_right_latitude = transformer.transform(right, bottom)
    return top_left_longitude, top_left_latitude, bottom_right_longitude, bottom_right_latitude


def load_dem(path: Path, width: int, height: int) -> array.array:
    if width < 1 or height < 1:
        raise HeightmapError("DEM dimensions must be positive")
    expected_bytes = width * height * 2
    actual_bytes = path.stat().st_size
    if actual_bytes != expected_bytes:
        raise HeightmapError(f"DEM size is {actual_bytes} bytes; expected {expected_bytes}")
    values = array.array("h")
    with path.open("rb") as stream:
        values.fromfile(stream, width * height)
    if sys.byteorder == "little":
        values.byteswap()
    # Validate values directly. The legacy code incorrectly treated each
    # elevation as an array index and could crash or accept an all-NODATA file.
    if not any(value != NO_DATA for value in values):
        raise HeightmapError(f"DEM contains only the NODATA value {NO_DATA}")
    return values


def extract(values: array.array, width: int, height: int, bounds: tuple[int, int, int, int], tile: int) -> dict:
    left, top, right, bottom = bounds
    if not (0 <= left < right <= width and 0 <= top < bottom <= height):
        raise HeightmapError(f"pixel bounds {bounds} are outside {width}x{height}")
    rows = []
    for y in range(top, bottom):
        row = []
        for x in range(left, right):
            value = values[y * width + x]
            row.append(None if value == NO_DATA else value)
        rows.append(row)
    return {"tile_number": tile, "width": right - left, "height": bottom - top, "elevation": rows}


def pixel_bounds(tile_bounds: tuple[float, float, float, float], width: int, height: int,
                 west: float, east: float, north: float, south: float) -> tuple[int, int, int, int]:
    left_longitude, top_latitude, right_longitude, bottom_latitude = tile_bounds
    if not (-180 <= west < east <= 180 and -90 <= south < north <= 90):
        raise HeightmapError("invalid DEM geographic bounds")
    if left_longitude < west or right_longitude > east or top_latitude > north or bottom_latitude < south:
        raise HeightmapError("tile geographic bounds lie outside the DEM")
    left = int((left_longitude - west) * width / (east - west))
    right = int((right_longitude - west) * width / (east - west))
    top = int((north - top_latitude) * height / (north - south))
    bottom = int((north - bottom_latitude) * height / (north - south))
    return left, top, right, bottom


def argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dir", "-d", type=Path, required=True, dest="maps_dir")
    parser.add_argument("--tile", type=int, default=160, choices=sorted(TILES_OSGB36))
    parser.add_argument("--image-width", type=int, default=4800)
    parser.add_argument("--image-height", type=int, default=6000)
    parser.add_argument("--west", type=float, default=-20)
    parser.add_argument("--east", type=float, default=20)
    parser.add_argument("--north", type=float, default=90)
    parser.add_argument("--south", type=float, default=40)
    parser.add_argument("--dem-filename", default="gt30w020n90.dem")
    parser.add_argument("--pixel-bounds", nargs=4, type=int, metavar=("LEFT", "TOP", "RIGHT", "BOTTOM"),
                        help="checked fixture/debug override for geographic pixel selection")
    parser.add_argument("--output", "-o", type=Path)
    return parser


def main() -> int:
    args = argument_parser().parse_args()
    try:
        if not args.maps_dir.is_dir():
            raise HeightmapError(f"directory not found: {args.maps_dir}")
        path = args.maps_dir / args.dem_filename
        if not path.is_file():
            raise HeightmapError(f"file not found: {path}")
        values = load_dem(path, args.image_width, args.image_height)
        bounds = tuple(args.pixel_bounds) if args.pixel_bounds else pixel_bounds(
            coords_from_tile(args.tile), args.image_width, args.image_height,
            args.west, args.east, args.north, args.south,
        )
        result = extract(values, args.image_width, args.image_height, bounds, args.tile)
        encoded = json.dumps(result, separators=(",", ":"), sort_keys=True) + "\n"
        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(encoded)
        else:
            sys.stdout.write(encoded)
    except (OSError, HeightmapError) as error:
        print(f"heightmap: error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
