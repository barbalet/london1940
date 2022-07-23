#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import sys
import argparse
import math
import pyproj
import array
from pyproj import Transformer

def coords_from_tile(tile_number: int) -> (float,float,float,float):
    """Returns the bounding box geocoordinates for a tile
    """
    tiles_osgb_36 = {
        "160": {
            "tx_long": 495,
            "ty_lat": 225,
            "bx_long": 535,
            "by_lat": 180
        },
        "161": {
            "tx_long": 535,
            "ty_lat": 225,
            "bx_long": 575,
            "by_lat": 180
        },
        "162": {
            "tx_long": 570,
            "ty_lat": 225,
            "bx_long": 610,
            "by_lat": 180
        },
        "170": {
            "tx_long": 495,
            "ty_lat": 180,
            "bx_long": 535,
            "by_lat": 135
        },
        "171": {
            "tx_long": 535,
            "ty_lat": 180,
            "bx_long": 575,
            "by_lat": 135
        },
        "172": {
            "tx_long": 570,
            "ty_lat": 180,
            "bx_long": 610,
            "by_lat": 135
        },
        "173": {
            "tx_long": 602,
            "ty_lat": 173,
            "bx_long": 642,
            "by_lat": 128
        },
        "182": {
            "tx_long": 495,
            "ty_lat": 145,
            "bx_long": 535,
            "by_lat": 100
        },
        "183": {
            "tx_long": 535,
            "ty_lat": 138,
            "bx_long": 575,
            "by_lat": 93
        },
        "184": {
            "tx_long": 570,
            "ty_lat": 150,
            "bx_long": 610,
            "by_lat": 105
        }
    }
    if str(tile_number) not in tiles_osgb_36:
        return None,None,None,None
    tx_long = tiles_osgb_36[str(tile_number)]["tx_long"]
    ty_lat = tiles_osgb_36[str(tile_number)]["ty_lat"]
    bx_long = tiles_osgb_36[str(tile_number)]["bx_long"]
    by_lat = tiles_osgb_36[str(tile_number)]["by_lat"]

    tx_long *= 1000
    ty_lat *= 1000
    bx_long *= 1000
    by_lat *= 1000

    transformer = Transformer.from_crs("epsg:27700", "epsg:4326")
    top_left = transformer.transform(tx_long, ty_lat)
    bottom_right = transformer.transform(bx_long, by_lat)

    return [
        top_left[1],
        top_left[0],
        bottom_right[1],
        bottom_right[0]
    ]


parser = argparse.ArgumentParser(description='heightmap')
parser.add_argument('--dir', '-d', type=str, dest='maps_dir',
                    default=None,
                    help='Maps directory')
parser.add_argument('--tile', type=int, dest='tile_number',
                    default=160,
                    help='Tile number')
parser.add_argument('--image-width', type=int, dest='image_width',
                    default=4800,
                    help='Width of the height map')
parser.add_argument('--image-height', type=int, dest='image_height',
                    default=6000,
                    help='Height of the height map')
parser.add_argument('--west', type=int, dest='west',
                    default=20,
                    help='Degrees west for the left side of the height image')
parser.add_argument('--east', type=int, dest='east',
                    default=20,
                    help='Degrees east for the right side of the height image')
parser.add_argument('--north', type=int, dest='north',
                    default=90,
                    help='Degrees north for the top side of the height image')
parser.add_argument('--south', type=int, dest='south',
                    default=40,
                    help='Degrees south for the bottom side of the height image')
parser.add_argument('--dem-filename', type=str, dest='dem_filename',
                    default="gt30w020n90.dem",
                    help='Filename of the height map')
args = parser.parse_args()

if not args.maps_dir:
    print('Please specify a directory for the elevation maps')
    sys.exit()

if not os.path.isdir(args.maps_dir):
    print('Directory not found: ' + args.maps_dir)
    sys.exit()

filename = args.maps_dir + '/' + args.dem_filename
if not os.path.isfile(filename):
    print('File not found: ' + filename)
    sys.exit()

# load the height map
b = array.array("h")
with open(filename, "rb") as fp_heightmap:
    b.fromfile(fp_heightmap, args.image_width*args.image_height)
if sys.byteorder == "little":
    b.byteswap()

valid = False
for index in b:
    if b[index] != -9999:
        valid = True
        break
if not valid:
    print('Invalid data in ' + filename)
    sys.exit()

tile_geocoords = coords_from_tile(args.tile_number)

tile_tx_long = tile_geocoords[0]
tile_ty_lat = tile_geocoords[1]
tile_bx_long = tile_geocoords[2]
tile_by_lat = tile_geocoords[3]

if args.east < 0:
    args.east = -args.east

if abs(tile_tx_long) > args.west:
    print('Tile left side is out of bounds ' + str(tile_tx_long) + ' ' + str(args.west))
    sys.exit()
if abs(tile_bx_long) > args.east:
    print('Tile right side is out of bounds ' + str(tile_bx_long) + ' -' + str(abs(args.east)))
    sys.exit()
if abs(tile_ty_lat) > args.north:
    print('Tile top side is out of bounds ' + str(tile_ty_lat) + ' ' + str(args.north))
    sys.exit()
if abs(tile_by_lat) < args.south:
    print('Tile bottom side is out of bounds ' + str(tile_by_lat) + ' ' + str(args.south))
    sys.exit()

# height map width and height in degrees
image_width_degrees = abs(args.east) + abs(args.west)
image_height_degrees = args.north - args.south

# get the bounding box within the height map in pixels
bx = abs(int((tile_tx_long - args.west) * args.image_width / image_width_degrees))
ty = int((image_height_degrees - (tile_ty_lat - args.south)) * args.image_height / image_height_degrees)
tx = abs(int((tile_bx_long - args.west) * args.image_width / image_width_degrees))
by = int((image_height_degrees - (tile_by_lat - args.south)) * args.image_height / image_height_degrees)

if tx >= bx:
    print('Bounding box horizontal error ' + str(tx) + ' >= ' + str(bx))
    sys.exit()
if ty >= by:
    print('Bounding box vertical error ' + str(ty) + ' >= ' + str(by))
    sys.exit()
if bx >= args.image_width:
    print('Bounding box horizontal out of range ' + str(bx))
    sys.exit()
if by >= args.image_height:
    print('Bounding box vertical out of range ' + str(by))
    sys.exit()

height_map = {
    "tile_number": args.tile_number,
    "width": bx-tx,
    "height": by-ty,
    "elevation": []
}
for y in range(by-ty):
    height_row = []
    for x in range(bx-tx):
        index = (ty+y)*args.image_width + (tx+x)
        height_value = b[index]
        if height_value == -9999:
            height_value = None
        height_row.append(height_value)
    height_map['elevation'].append(height_row)

print(str(height_map))
