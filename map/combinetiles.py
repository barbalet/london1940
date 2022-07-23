#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import sys
import json
import argparse


def polygon_bounding_box(poly: {}) -> (int, int, int, int):
    """Returns a bounding box for the polygon
    """
    if poly.get("ext"):
        exterior = poly["ext"]
    else:
        exterior = poly["perimeter"]
    box_tx = 99999999999999999
    box_ty = 99999999999999999
    box_bx = -99999999999999999
    box_by = -99999999999999999
    for vertex in exterior:
        x_coord = vertex[0]
        y_coord = vertex[1]
        if x_coord < box_tx:
            box_tx = x_coord
        if x_coord > box_bx:
            box_bx = x_coord
        if y_coord < box_ty:
            box_ty = y_coord
        if y_coord > box_by:
            box_by = y_coord
    return box_tx, box_ty, box_bx, box_by


def point_inside_bounding_box(point_x: int, point_y: int,
                              bb_tx: int, bb_ty: int,
                              bb_bx: int, bb_by):
    """Is the given point inside the bounding box?
    """
    if point_x >= bb_tx and point_y >= bb_ty and \
       point_x <= bb_bx and point_y <= bb_by:
        return True
    return False


def point_in_polygon(poly: [], x_coord: float, y_coord: float) -> bool:
    """Returns true if the given point is inside the given polygon
    """
    num = len(poly)
    inside = False
    p2x = 0.0
    p2y = 0.0
    xints = 0.0
    p1x, p1y = poly[0]
    for i in range(num + 1):
        p2x, p2y = poly[i % num]
        if y_coord > min(p1y, p2y):
            if y_coord <= max(p1y, p2y):
                if x_coord <= max(p1x, p2x):
                    if p1y != p2y:
                        xints = \
                            (y_coord - p1y) * (p2x - p1x) / (p2y - p1y) + p1x
                    if p1x == p2x or x_coord <= xints:
                        inside = not inside
        p1x, p1y = p2x, p2y

    return inside


def dedup_polygons(polygons: {}, search_radius: int) -> {}:
    """Deduplicate polygons dict.
    This is a simplistic method which assumes that if the bounding box for
    two polygons is very similar - within a few pixels - then it is likely
    to be a duplicate detection
    """
    no_of_duplicates = 0
    result = {}
    index = 0
    poly_box = []
    poly_list = []
    for _, poly in polygons.items():
        poly_list.append(poly)
        box_tx, box_ty, box_bx, box_by = polygon_bounding_box(poly)
        poly_box.append([box_tx, box_ty, box_bx, box_by])

    for i in range(len(poly_box)):
        if i % 100 == 0:
            print('.', end='', flush=True)
        tx1 = poly_box[i][0]
        ty1 = poly_box[i][1]
        bx1 = poly_box[i][2]
        by1 = poly_box[i][3]

        center_x = tx1 + int((bx1 - tx1)/2)
        center_y = ty1 + int((by1 - ty1)/2)

        found = False
        for j in range(i+1, len(poly_box)):
            tx2 = poly_box[j][0]
            ty2 = poly_box[j][1]
            bx2 = poly_box[j][2]
            by2 = poly_box[j][3]

            if point_inside_bounding_box(center_x, center_y,
                                         tx2, ty2, bx2, by2):
                no_of_duplicates += 1
                print('x', end='', flush=True)
                found = True
                break

        if not found:
            result[str(index)] = poly_list[i]
            index += 1
    print('\n' + str(no_of_duplicates) + ' duplicates removed')
    return result


def remove_duplicate_points(new_points: [], current_points: [],
                            search_radius: int) -> []:
    """Removes duplicate entries for [x,y] coords within the new list
    """
    deduped = []
    for coord1 in new_points:
        coord_new = coord1.copy()
        if isinstance(coord_new[0], list):
            coord_new = coord_new[0]
        x1_coord = coord_new[0]
        y1_coord = coord_new[1]
        found = False
        for coord2 in current_points:
            coord_curr = coord2.copy()
            if isinstance(coord_curr[0], list):
                coord_curr = coord_curr[0]
            x2_coord = coord_curr[0]
            if abs(x2_coord - x1_coord) < search_radius:
                y2_coord = coord_curr[1]
                if abs(y2_coord - y1_coord) < search_radius:
                    found = True
                    break
        if not found:
            deduped.append(coord1)
    return deduped


def get_json_files(path: str) -> []:
    """Gets json files
    """
    result = []
    for subdir, _, files in os.walk(path):
        for filename in files:
            if not filename.endswith('.json'):
                continue
            if filename.endswith('railway.json'):
                continue
            if filename.endswith('heightmap.json'):
                continue
            filename = os.path.join(subdir, filename)
            with open(filename, 'r') as json_file:
                data = json_file.read()
                json_object = json.loads(data)
                if json_object.get("tile"):
                    result.append(json_object)
    return result


def tiles_bounding_box(tiles: []) -> ([], []):
    """Returns the bounding box for all tiles, in pixels and geocoords
    """
    map_tx = 99999999999999999
    map_ty = 99999999999999999
    map_bx = -99999999999999999
    map_by = -99999999999999999

    longitude_tx = 99999999999999999
    latitude_ty = 99999999999999999
    longitude_bx = -99999999999999999
    latitude_by = -99999999999999999
    for tile_json in tiles:
        if not tile_json.get("offset"):
            continue
        if not tile_json.get("resolution"):
            continue
        if not tile_json.get("geocoords"):
            continue

        tile_tx = tile_json["offset"][0]
        tile_ty = tile_json["offset"][1]
        tile_bx = tile_json["offset"][0] + tile_json["resolution"][0]
        tile_by = tile_json["offset"][1] + tile_json["resolution"][1]
        if tile_tx < map_tx:
            map_tx = tile_tx
            longitude_tx = tile_json["geocoords"]["topleft"]["longitude"]
        if tile_bx > map_bx:
            map_bx = tile_bx
            longitude_bx = tile_json["geocoords"]["bottomright"]["longitude"]
        if tile_ty < map_ty:
            map_ty = tile_ty
            latitude_ty = tile_json["geocoords"]["topleft"]["latitude"]
        if tile_by > map_by:
            map_by = tile_by
            latitude_by = tile_json["geocoords"]["bottomright"]["latitude"]

    return [map_tx, map_ty, map_bx, map_by], \
        [longitude_tx, latitude_ty, longitude_bx, latitude_by]


def combine_tiles(tiles: []) -> {}:
    """Combines the given json tiles
    """
    pixels_box, geo_box = tiles_bounding_box(tiles)

    combined = {
        "offset": [pixels_box[0], pixels_box[1]],
        "resolution": [pixels_box[2] - pixels_box[0],
                       pixels_box[3] - pixels_box[1]],
        "geocoords": {
            "topleft": {
                "latitude": geo_box[1],
                "longitude": geo_box[0]
            },
            "bottomright": {
                "latitude": geo_box[3],
                "longitude": geo_box[2]
            }
        },
        "terrain": [],
        "urban": []
    }
    combined_buildings = {}
    combined_woods = {}
    combined_lakes = {}
    combined_sea = {}
    combined_sands = {}
    combined_rivers = {
        "type": "rivers",
        "points": [],
        "widths": [],
        "links": []
    }
    combined_harbours = {
        "type": "harbours",
        "points": [],
        "links": []
    }
    combined_railway_lines = {
        "type": "railway lines",
        "points": [],
        "links": []
    }
    combined_railway_tunnels = {
        "type": "railway tunnels",
        "points": [],
        "links": []
    }
    combined_main_roads = {
        "type": "main roads",
        "points": [],
        "links": []
    }
    combined_minor_roads = {
        "type": "minor roads",
        "points": [],
        "links": []
    }
    combined_bridges = {
        "type": "bridges",
        "pointdirections": []
    }
    combined_junctions = {
        "type": "junctions",
        "points": []
    }
    combined_railway_stations = {
        "type": "railway stations",
        "points": []
    }
    combined_orchards = {
        "type": "orchards",
        "points": []
    }

    building_index = 0
    woods_index = 0
    sands_index = 0
    lake_index = 0
    sea_index = 0
    for json_tile in tiles:
        if json_tile.get('terrain'):
            for terrain_item in json_tile['terrain']:
                if terrain_item['type'] == 'sands':
                    for _, sands_points in terrain_item['polygons'].items():
                        combined_sands[str(sands_index)] = {
                            "perimeter": sands_points["perimeter"]
                        }
                        sands_index += 1
                elif terrain_item['type'] == 'woodland':
                    for _, woods_points in terrain_item['polygons'].items():
                        combined_woods[str(woods_index)] = woods_points
                        woods_index += 1
                elif terrain_item['type'] == 'lakes':
                    for _, lake_item in terrain_item['polygons'].items():
                        combined_lakes[str(lake_index)] = {
                            "perimeter": lake_item["perimeter"]
                        }
                        lake_index += 1
                elif terrain_item['type'] == 'sea':
                    for _, sea_item in terrain_item['polygons'].items():
                        combined_sea[str(sea_index)] = {
                            "perimeter": sea_item["perimeter"]
                        }
                        sea_index += 1
                elif terrain_item['type'] == 'rivers':
                    curr_river_points = len(combined_rivers["points"])
                    combined_rivers["points"] += terrain_item["points"]
                    combined_rivers["widths"] += terrain_item["widths"]
                    for river_link in terrain_item["links"]:
                        next_index = river_link[0]
                        prev_index = river_link[1]
                        new_link = []
                        if next_index > -1:
                            new_link.append(next_index + curr_river_points)
                        else:
                            new_link.append(-1)
                        if prev_index > -1:
                            new_link.append(prev_index + curr_river_points)
                        else:
                            new_link.append(-1)
                        combined_rivers["links"].append(new_link)
        if json_tile.get('urban'):
            for urban_item in json_tile['urban']:
                if urban_item['type'] == 'harbours':
                    curr_harbour_points = len(combined_harbours["points"])
                    combined_harbours["points"] += urban_item["points"]
                    for harbour_link in urban_item["links"]:
                        next_index = harbour_link[0]
                        prev_index = harbour_link[1]
                        new_link = []
                        if next_index > -1:
                            new_link.append(next_index + curr_harbour_points)
                        else:
                            new_link.append(-1)
                        if prev_index > -1:
                            new_link.append(prev_index + curr_harbour_points)
                        else:
                            new_link.append(-1)
                        combined_harbours["links"].append(new_link)
                elif urban_item['type'] == 'railway lines':
                    curr_railway_line_points = len(combined_railway_lines["points"])
                    combined_railway_lines["points"] += urban_item["points"]
                    for railway_line_link in urban_item["links"]:
                        next_index = railway_line_link[0]
                        prev_index = railway_line_link[1]
                        new_link = []
                        if next_index > -1:
                            new_link.append(next_index + curr_railway_line_points)
                        else:
                            new_link.append(-1)
                        if prev_index > -1:
                            new_link.append(prev_index + curr_railway_line_points)
                        else:
                            new_link.append(-1)
                        combined_railway_lines["links"].append(new_link)
                elif urban_item['type'] == 'railway tunnels':
                    curr_railway_tunnel_points = len(combined_railway_tunnels["points"])
                    combined_railway_tunnels["points"] += urban_item["points"]
                    for railway_tunnel_link in urban_item["links"]:
                        next_index = railway_tunnel_link[0]
                        prev_index = railway_tunnel_link[1]
                        new_link = []
                        if next_index > -1:
                            new_link.append(next_index + curr_railway_tunnel_points)
                        else:
                            new_link.append(-1)
                        if prev_index > -1:
                            new_link.append(prev_index + curr_railway_tunnel_points)
                        else:
                            new_link.append(-1)
                        combined_railway_tunnels["links"].append(new_link)
                elif urban_item['type'] == 'main roads':
                    curr_main_road_points = len(combined_main_roads["points"])
                    combined_main_roads["points"] += urban_item["points"]
                    for main_road_link in urban_item["links"]:
                        next_index = main_road_link[0]
                        prev_index = main_road_link[1]
                        new_link = []
                        if next_index > -1:
                            new_link.append(next_index + curr_main_road_points)
                        else:
                            new_link.append(-1)
                        if prev_index > -1:
                            new_link.append(prev_index + curr_main_road_points)
                        else:
                            new_link.append(-1)
                        combined_main_roads["links"].append(new_link)
                elif urban_item['type'] == 'minor roads':
                    curr_minor_road_points = \
                        len(combined_minor_roads["points"])
                    combined_minor_roads["points"] += urban_item["points"]
                    for minor_road_link in urban_item["links"]:
                        next_index = minor_road_link[0]
                        prev_index = minor_road_link[1]
                        new_link = []
                        if next_index > -1:
                            new_link.append(next_index +
                                            curr_minor_road_points)
                        else:
                            new_link.append(-1)
                        if prev_index > -1:
                            new_link.append(prev_index +
                                            curr_minor_road_points)
                        else:
                            new_link.append(-1)
                        combined_minor_roads["links"].append(new_link)
                elif urban_item['type'] == 'buildings':
                    for _, building_item in urban_item['polygons'].items():
                        combined_buildings[str(building_index)] = building_item
                        building_index += 1
                elif urban_item['type'] == 'bridges':
                    if not urban_item.get("pointdirections"):
                        print(str(urban_item))
                    else:
                        new_points = \
                            remove_duplicate_points(urban_item["pointdirections"],
                                                    combined_bridges["pointdirections"],
                                                    20)
                        combined_bridges["pointdirections"] += new_points
                elif urban_item['type'] == 'junctions':
                    new_points = \
                        remove_duplicate_points(urban_item["points"],
                                                combined_junctions["points"],
                                                20)
                    combined_junctions["points"] += new_points
                elif urban_item['type'] == 'railway stations':
                    station_points = combined_railway_stations["points"]
                    new_points = \
                        remove_duplicate_points(urban_item["points"],
                                                station_points, 20)
                    combined_railway_stations["points"] += new_points
                elif urban_item['type'] == 'orchards':
                    orchard_points = combined_orchards["points"]
                    new_points = \
                        remove_duplicate_points(urban_item["points"],
                                                orchard_points, 10)
                    combined_orchards["points"] += new_points

    print('Deduplicating woodland areas')
    combined_woods_dedup = dedup_polygons(combined_woods, 3)
    combined["terrain"].append({
        "type": "woodland",
        "polygons": combined_woods_dedup
    })

    print('Deduplicating lakes')
#    combined_lakes_dedup = dedup_polygons(combined_lakes, 3)
    combined["terrain"].append({
        "type": "lakes",
        "polygons": combined_lakes
    })

    print('Deduplicating sea areas')
    combined_sea_dedup = dedup_polygons(combined_sea, 3)
    combined["terrain"].append({
        "type": "sea",
        "polygons": combined_sea_dedup
    })
#    combined_sands_dedup = dedup_polygons(combined_sands, 3)
    combined["terrain"].append({
        "type": "sands",
        "polygons": combined_sands
    })
    combined["terrain"].append(combined_rivers)
    combined["urban"].append(combined_harbours)
    combined["urban"].append(combined_railway_lines)
    combined["urban"].append(combined_railway_tunnels)
    combined["urban"].append(combined_main_roads)
    combined["urban"].append(combined_minor_roads)
    combined["urban"].append(combined_bridges)
    combined["urban"].append(combined_junctions)
    combined["urban"].append(combined_railway_stations)
    combined["urban"].append(combined_orchards)

    print('Deduplicating buildings')
    combined_buildings_dedup = dedup_polygons(combined_buildings, 3)
    combined["urban"].append({
        "type": "buildings",
        "polygons": combined_buildings_dedup
    })

    print(str(sands_index) + ' sands areas')
    print(str(woods_index) + ' woodland areas')
    print(str(lake_index) + ' lake areas')
    print(str(sea_index) + ' sea areas')
    print(str(len(combined_rivers["points"])) + ' river points')
    print(str(len(combined_harbours["points"])) + ' harbour points')
    print(str(len(combined_railway_lines["points"])) + ' railway line points')
    print(str(len(combined_railway_tunnels["points"])) + ' railway tunnel points')
    print(str(len(combined_main_roads["points"])) + ' main road points')
    print(str(len(combined_minor_roads["points"])) + ' minor road points')
    print(str(len(combined_buildings_dedup.items())) + ' buildings')
    print(str(len(combined_bridges["pointdirections"])) + ' bridges')
    print(str(len(combined_junctions["points"])) + ' junctions')
    print(str(len(combined_railway_stations["points"])) + ' railway stations')
    print(str(len(combined_orchards["points"])) + ' orchards')
    return combined


parser = argparse.ArgumentParser(description='combinetiles')
parser.add_argument('--dir', '-d', type=str, dest='tiles_dir',
                    default='tiles',
                    help='Tiles directory')
parser.add_argument('--output', '-o', type=str, dest='output_filename',
                    default='largemap.json',
                    help='Output filename for the large map json')
args = parser.parse_args()

json_tiles = get_json_files(args.tiles_dir)
print(str(len(json_tiles)) + ' tiles loaded')
if len(json_tiles) == 0:
    sys.exit()

json_map = combine_tiles(json_tiles)

# append height map
heightmap_filename = os.path.join(args.tiles_dir, 'heightmap.json')
if os.path.isfile(heightmap_filename):
    print('Loading elevations: ' + heightmap_filename)
    with open(heightmap_filename, 'r') as json_file:
        data = json_file.read()
        if data:
            heightmap_json = json.loads(data)
            if heightmap_json:
                json_map["terrain"]["elevation"] = heightmap_json

# append railways and harbours to the result
railway_filename = os.path.join(args.tiles_dir, 'railway.json')
if os.path.isfile(railway_filename):
    with open(railway_filename, 'r') as json_file:
        data = json_file.read()
        railway_objects_list = json.loads(data)
        if railway_objects_list:
            # create a new urban list with the railways removed
            new_urban_list = []
            for item in json_map['urban']:
                if not item.get('type'):
                    continue
                if item['type'] != 'harbours' and \
                   item['type'] != 'railway lines' and \
                   item['type'] != 'railway tunnels':
                    new_urban_list.append(item)
            # add the railways to the list
            for item in railway_objects_list:
                if not item.get('type'):
                    continue
                if item['type'] == 'harbours' or \
                   item['type'] == 'railway lines' or \
                   item['type'] == 'railway tunnels':
                    new_urban_list.append(item)
            json_map['urban'] = new_urban_list

# pprint(json_map)
map_str = json.dumps(json_map)

with open(args.output_filename, 'w+') as fp:
    fp.write(map_str)
