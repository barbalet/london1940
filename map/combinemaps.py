#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import sys
import json
import argparse

# points which match between the large maps
matching_points = {
    "160": {
        "161": [
            [[10480,1406], [486,1464]],
            [[10484,2004], [488,2060]],
            [[10480,3416], [484,3484]],
            [[10480,3756], [484,3808]],
            [[10484,4404], [484,4464]],
            [[10488,7080], [488,7124]],
            [[10500,11632], [492,11668]],
            [[10500,11784], [488,11824]]
        ],
        "170": [
            [[1408,12116], [1496,958]],
            [[1508,12116], [1592,958]],
            [[2774,12108], [2858,954]],
            [[3698,12108], [3774,952]],
            [[6462,12104], [6536,948]],
            [[9672,12096], [9736,946]]
        ]
    },
    "161": {
        "162": [
            [[10404,1462], [1744,1418]],
            [[10404,1776], [1744,1726]],
            [[10410,4746], [1744,4696]],
            [[10414,7052], [1748,7016]],
            [[10414,10088], [1744,10040]]
        ],
        "171": [
            [[614,12138], [666,946]],
            [[1008,12138], [1060,944]],
            [[5434,12134], [5490,950]],
            [[7980,12134], [8034,956]]
        ]
    },
    "162": {
        "172": [
            [[574,12086], [538,950]],
            [[10440,12092], [10398,982]]
        ]
    },
    "170": {
        "171": [
            [[10560,1046], [544,1050]],
            [[10564,1576], [540,1580]],
            [[10568,10832], [528,10818]],
            [[10570,11332], [522,11312]],
            [[10572,11880], [526,11866]]
        ],
        "182": [
            [[658,10298], [612,1708]],
            [[2462,10364], [2424,1784]],
            [[5982,10356], [5950,1798]],
            [[8374,9614], [8348,1068]],
            [[8940,9616], [8916,1068]],
            [[9832,10356], [9808,1814]]
        ]
    },
    "171": {
        "172": [
            [[10478,3122], [1700,3120]],
            [[10474,3980], [1696,3972]],
            [[10480,5814], [1692,5802]],
            [[10474,7362], [1686,7362]],
            [[10468,9742], [1682,9740]],
            [[10464,11484], [1678,11480]]
        ],
        "183": [
            [[1144,11338], [1164,1092]],
            [[3764,11344], [3780,1096]],
            [[5080,11346], [5112,1098]],
            [[8450,11356], [8472,1104]],
            [[8818,11356], [8840,1104]],
            [[9218,11360], [9236,1106]],
            [[9980,11364], [9998,1108]]
        ]
    },
    "172": {
        "173": [
            [[8748,3318], [980,1568]],
            [[8872,5946], [1128,4198]],
            [[8676,7856], [952,6112]],
            [[8812,10428], [1110,8682]]
        ],
        "184": [
            [[572,8382], [648,954]],
            [[1196,8380], [1268,956]],
            [[1806,8384], [1878,954]],
            [[4112,8390], [4182,954]],
            [[6340,8398], [6404,956]],
            [[9396,8412], [9456,950]]
        ]
    }
}


def get_json_files(path: str) -> []:
    """Gets json files
    """
    result = []
    for subdir, _, files in os.walk(path):
        for filename in files:
            if not filename.endswith('.json'):
                continue
            filename = os.path.join(subdir, filename)
            with open(filename, 'r') as json_file:
                data = json_file.read()
                json_object = json.loads(data)
                json_object['filename'] = filename
                if not json_object.get("tile"):
                    result.append(json_object)
    return result


def maps_bounding_box_geo(maps: []) -> []:
    """Returns the bounding box for all maps, in geocoords
    """
    # ranges are in ordinance survey grid squares
    min_longitude = 495
    max_longitude = 642
    min_latitude = 93
    max_latitude = 225
    longitude_tx = 99999999999999999
    latitude_ty = -99999999999999999
    longitude_bx = -99999999999999999
    latitude_by = 99999999999999999
    for map_json in maps:
        if not map_json.get("geocoords"):
            continue

        if map_json["geocoords"]["topleft"]["longitude"] < longitude_tx:
            longitude_tx = map_json["geocoords"]["topleft"]["longitude"]
            if (longitude_tx < min_longitude) or \
               (longitude_tx > max_longitude):
                print(map_json['filename'])
                print("longitude tx out of range: " + str(longitude_tx))
        if map_json["geocoords"]["bottomright"]["longitude"] > longitude_bx:
            longitude_bx = map_json["geocoords"]["bottomright"]["longitude"]
            if (longitude_bx < min_longitude) or \
               (longitude_bx > max_longitude):
                print(map_json['filename'])
                print("longitude bx out of range: " + str(longitude_bx))

        if map_json["geocoords"]["topleft"]["latitude"] > latitude_ty:
            latitude_ty = map_json["geocoords"]["topleft"]["latitude"]
            if (latitude_ty < min_latitude) or \
               (latitude_ty > max_latitude):
                print(map_json['filename'])
                print("latitude ty out of range: " + str(latitude_ty))
        if map_json["geocoords"]["bottomright"]["latitude"] < latitude_by:
            latitude_by = map_json["geocoords"]["bottomright"]["latitude"]
            if (latitude_by < min_latitude) or \
               (latitude_by > max_latitude):
                print(map_json['filename'])
                print("latitude by out of range: " + str(latitude_by))

    return [longitude_tx, latitude_ty, longitude_bx, latitude_by]


def maps_bounding_box_pixels(maps: []) -> []:
    """Returns the bounding box for all maps, in pixels
    """
    map_offset_diff = {}
    map_offset_total = {}

    map_resolution = {}
    map_offset = {}
    map_offset_pixels = {}
    map_updated = {}

    for map_json in maps:
        if not map_json.get("resolution"):
            continue
        if not map_json.get("filename"):
            continue
        map_name = get_map_name_from_filename(map_json['filename'])
        map_resolution[map_name] = map_json["resolution"]
        map_offset[map_name] = map_json["offset"]
        map_offset_pixels[map_name] = [0,0]
        map_updated[map_name] = False

    for map_json in maps:
        if not map_json.get("resolution"):
            continue
        if not map_json.get("filename"):
            continue
        map_name = get_map_name_from_filename(map_json['filename'])
        if not matching_points.get(map_name):
            continue

        for other_map_name, points_list in matching_points[map_name].items():
            dx = 0
            dy = 0
            adjust_dx = 0
            adjust_dy = 0
            print("adjust " + map_name + ' -> ' + other_map_name)
            for match_pts in points_list:
                map_x = match_pts[0][0] - map_offset[map_name][0]
                map_y = match_pts[0][1] - map_offset[map_name][1]
                other_map_x = match_pts[1][0] - map_offset[other_map_name][0]
                other_map_y = match_pts[1][1] - map_offset[other_map_name][1]
                dx += map_x - other_map_x
                dy += map_y - other_map_y

                if other_map_name == "173":
                    ddx = other_map_x + (map_resolution[map_name][0] - map_x)
                    ddy = other_map_y + (map_resolution[map_name][1] - map_y)
                elif abs(dx) > abs(dy):
                    ddx = other_map_x + (map_resolution[map_name][0] - map_x)
                    ddy = dy
                else:
                    ddx = dx
                    ddy = other_map_y + (map_resolution[map_name][1] - map_y)
                print(str(int(ddx)) + ' ' + str(int(ddy)))
                adjust_dx += ddx                
                adjust_dy += ddy

            dx = int(dx / len(points_list))
            dy = int(dy / len(points_list))
            adjust_dx = int(adjust_dx / len(points_list))
            adjust_dy = int(adjust_dy / len(points_list))

            if not map_offset_diff.get(map_name):
                map_offset_diff[map_name] = {
                    other_map_name: [dx, dy, adjust_dx, adjust_dy]
                }
            else:
                map_offset_diff[map_name][other_map_name] = \
                    [dx, dy, adjust_dx, adjust_dy]

    map_updated["160"] = True
    new_updates = True
    while new_updates:
        new_updates = False
        updated_list = []
        for map_json in maps:
            if not map_json.get("filename"):
                continue
            map_name = get_map_name_from_filename(map_json['filename'])
            if not map_offset_diff.get(map_name):
                continue
            if not map_offset_diff[map_name]:
                continue
            # map must have had offsets assigned
            if not map_updated[map_name]:
                continue

            for other_map_name, offset in map_offset_diff[map_name].items():
                # map should not have had offsets assigned
                if map_updated[other_map_name]:
                    continue
                new_updates = True
                # add offset, then mark as updated
                adjust_x = offset[2]
                adjust_y = offset[3]
                curr_offset_x = map_offset_pixels[map_name][0]
                curr_offset_y = map_offset_pixels[map_name][1]
                prev_map_resolution_x = map_resolution[map_name][0]
                prev_map_resolution_y = map_resolution[map_name][1]
                if other_map_name == "173":
                    map_offset_pixels[other_map_name] = [
                        curr_offset_x + prev_map_resolution_x - adjust_x,
                        curr_offset_y + prev_map_resolution_y - adjust_y
                    ]
                elif abs(offset[0]) > abs(offset[1]):
                    map_offset_pixels[other_map_name] = [
                        curr_offset_x + prev_map_resolution_x - adjust_x,
                        curr_offset_y - adjust_y
                    ]
                else:
                    map_offset_pixels[other_map_name] = [
                        curr_offset_x - adjust_x,
                        curr_offset_y + prev_map_resolution_y - adjust_y
                    ]
                # update the list of maps which have had theor offset calculated
                if other_map_name not in updated_list:
                    updated_list.append(other_map_name)
        for map_name in updated_list:
            map_updated[map_name] = True

    resolution_x = 0
    resolution_y = 0
    for map_json in maps:
        if not map_json.get("filename"):
            continue
        if not map_json.get("resolution"):
            continue
        map_name = get_map_name_from_filename(map_json['filename'])
        if map_offset_pixels.get(map_name):
            map_json["full_offset"] = map_offset_pixels[map_name]
            full_bx = int(map_json["full_offset"][0] + map_json['resolution'][0])
            full_by = int(map_json["full_offset"][1] + map_json['resolution'][1])
            if full_bx > resolution_x:
                resolution_x = full_bx
            if full_by > resolution_y:
                resolution_y = full_by

    return [0, 0, resolution_x, resolution_y]


def points_to_geo(full_offset: [], offset: [], resolution: [],
                  map_geo_box: [], full_geo_box: [], full_pixels_box: [],
                  points: [], x_adjust: int, y_adjust: int,
                  x_scale: float, y_scale: float,
                  downscale: int) -> []:
    """Returns the list of points converted to to the full image resolution
    """
    result = []
    max_x = int(full_pixels_box[2] / downscale)
    max_y = int(full_pixels_box[3] / downscale)
    for point in points:
        x_coord = point[0] - offset[0] + full_offset[0]
        y_coord = point[1] - offset[1] + full_offset[1]
        x_coord = int((int(x_coord*x_scale) + x_adjust) / downscale)
        y_coord = int((int(y_coord*y_scale) + y_adjust) / downscale)
        if x_coord < 0:
            x_coord = 0
        if x_coord >= max_x:
            x_coord = max_x-1;
        if y_coord < 0:
            y_coord = 0
        if y_coord >= max_y:
            y_coord = max_y-1;
        result.append([x_coord, y_coord])
    return result


def bridges_to_geo(full_offset: [], offset: [], resolution: [],
                   map_geo_box: [], full_geo_box: [], full_pixels_box: [],
                   points: [], x_adjust: int, y_adjust: int,
                   x_scale: float, y_scale: float,
                   downscale: int) -> []:
    """Returns the list of bridge points converted to the full image resolution
    """
    result = []
    max_x = int(full_pixels_box[2] / downscale)
    max_y = int(full_pixels_box[3] / downscale)
    for point in points:
        x_coord = point[0][0] - offset[0] + full_offset[0]
        y_coord = point[0][1] - offset[1] + full_offset[1]
        x_coord = int((int(x_coord*x_scale) + x_adjust) / downscale)
        y_coord = int((int(y_coord*y_scale) + y_adjust) / downscale)
        if x_coord < 0:
            x_coord = 0
        if x_coord >= max_x:
            x_coord = max_x-1;
        if y_coord < 0:
            y_coord = 0
        if y_coord >= max_y:
            y_coord = max_y-1;
        result.append([[x_coord, y_coord], point[1]])
    return result


def polygon_to_geo(full_offset: [], offset: [], resolution: [],
                   map_geo_box: [], full_geo_box: [], full_pixels_box: [],
                   poly: {}, x_adjust: int, y_adjust: int,
                   x_scale: float, y_scale: float,
                   downscale: int) -> {}:
    """Returns a polgon converted to geocoords
    """
    if poly.get("ext"):
        poly["ext"] = \
            points_to_geo(full_offset, offset,
                          resolution, map_geo_box, full_geo_box,
                          full_pixels_box, poly["ext"],
                          x_adjust, y_adjust,
                          x_scale, y_scale,
                          downscale)
    if poly.get("int"):
        poly["int"] = \
            points_to_geo(full_offset, offset,
                          resolution, map_geo_box, full_geo_box,
                          full_pixels_box, poly["int"],
                          x_adjust, y_adjust,
                          x_scale, y_scale,
                          downscale)
    if poly.get("perimeter"):
        poly["perimeter"] = \
            points_to_geo(full_offset, offset,
                          resolution, map_geo_box, full_geo_box,
                          full_pixels_box, poly["perimeter"],
                          x_adjust, y_adjust,
                          x_scale, y_scale,
                          downscale)
    return poly


def get_map_name_from_filename(filename: str) -> str:
    """
    """
    map_name = filename
    if '/' in map_name:
        map_name = map_name.split('/')[-1]
    if '.' in map_name:
        map_name = map_name.split('.')[0]
    return map_name


def point_inside_bounding_box(point_x: int, point_y: int,
                              bb_tx: int, bb_ty: int,
                              bb_bx: int, bb_by):
    """Is the given point inside the bounding box?
    """
    if point_x >= bb_tx and point_y >= bb_ty and \
       point_x <= bb_bx and point_y <= bb_by:
        return True
    return False


def remove_point_from_line(line_item: {}, remove_index: int):
    """Removes a point from a line
    """
    if remove_index < 0:
        return

    # disconnect links to the removed point
    for link in line_item["links"]:
        for idx in range(2):
            if link[idx] == remove_index:
                link[idx] = -1

    # if any indexes are higher than the removed point index then
    # decrease their index value
    for link in line_item["links"]:
        for idx in range(2):
            if link[idx] > remove_index:
                link[idx] -= 1

    # remove the point and its links
    del line_item["links"][remove_index]
    del line_item["points"][remove_index]


def remove_links(map_json: {},
                 overlap_tx: int, overlap_ty: int,
                 overlap_bx: int, overlap_by: int,
                 downscale: int, full_deduplication: bool):
    """Removes links within the given region
    """
    if not map_json.get('urban') or not map_json.get('terrain'):
        print('Map expected to contain urban and terrain sections')
        return
    print('Remove links bounding box: ' +
          str(overlap_tx) + ' ' + str(overlap_ty) + ' ' +
          str(overlap_bx) + ' ' + str(overlap_by))

    link_types = ('main roads', 'minor roads',
                  'railway lines', 'railway tunnels')
    for urban_item in map_json['urban']:
        if urban_item['type'] in link_types:
            removed_indexes = []
            for index in range(len(urban_item["points"])):
                point_x = urban_item["points"][index][0] * downscale
                point_y = urban_item["points"][index][1] * downscale
                if point_inside_bounding_box(point_x, point_y,
                                             overlap_tx, overlap_ty,
                                             overlap_bx, overlap_by):
                    urban_item["links"][index][0] = -1
                    urban_item["links"][index][1] = -1
                    removed_indexes.append(index)
            if full_deduplication:
                removed_indexes.sort(reverse=True)            
                for rm_index in removed_indexes:
                    remove_point_from_line(urban_item, rm_index)

    for urban_item in map_json['urban']:
        if urban_item['type'] != 'buildings':
            continue
        removed_indexes = []
        for index, building in urban_item['polygons'].items():
            point_x = building['ext'][0][0]
            point_y = building['ext'][0][1]
            if point_inside_bounding_box(point_x, point_y,
                                         overlap_tx, overlap_ty,
                                         overlap_bx, overlap_by):
                removed_indexes.append(index)
        print('Deduplicate ' + str(len(removed_indexes)) + ' buildings')
        for index in removed_indexes:
            del urban_item['polygons'][index]

    link_types = ('rivers')
    for terrain_item in map_json['terrain']:
        if terrain_item['type'] in link_types:
            removed_indexes = []
            for index in range(len(terrain_item["points"])):
                point_x = terrain_item["points"][index][0] * downscale
                point_y = terrain_item["points"][index][1] * downscale
                if point_inside_bounding_box(point_x, point_y,
                                             overlap_tx, overlap_ty,
                                             overlap_bx, overlap_by):
                    terrain_item["links"][index][0] = -1
                    terrain_item["links"][index][1] = -1
                    removed_indexes.append(index)
            if full_deduplication:
                removed_indexes.sort(reverse=True)            
                for rm_index in removed_indexes:
                    remove_point_from_line(terrain_item, rm_index)

#    polygon_types = ('woodland', 'lakes', 'sands')
    polygon_types = ['woodland']
    for terrain_item in map_json['terrain']:
        if terrain_item['type'] not in polygon_types:
            continue
        removed_indexes = []
        for index, item in terrain_item['polygons'].items():
            point_x = item['perimeter'][0][0]
            point_y = item['perimeter'][0][1]
            if point_inside_bounding_box(point_x, point_y,
                                         overlap_tx, overlap_ty,
                                         overlap_bx, overlap_by):
                removed_indexes.append(index)
        print('Deduplicate ' + str(len(removed_indexes)) + ' ' + terrain_item['type'])
        for index in removed_indexes:
            del terrain_item['polygons'][index]


def deduplicate_points(map_json: {}, min_separation: int, section_name: str):
    """Remove duplicate points
    """
    min_separation2 = min_separation*min_separation

    for urban_item in map_json['urban']:
        if urban_item['type'] != section_name:
            continue
        points_list = urban_item['points']
        remove_indexes = []
        for index1 in range(len(points_list)-1):
            for index2 in range(index1+1, len(points_list)):
                dx = points_list[index1][0] - points_list[index2][0]
                dy = points_list[index1][1] - points_list[index2][1]
                separation2 = dx*dx + dy*dy
                if separation2 < min_separation2:
                    remove_indexes.append(index1)
                    break
        if len(remove_indexes) == 0:
            break
        print('Removing ' + str(len(remove_indexes)) + ' duplicate ' + section_name)
        remove_indexes.sort(reverse=True)
        for index in remove_indexes:
            del urban_item['points'][index]


def deduplicate_bridges(map_json: {}, min_separation: int):
    """Remove duplicate bridges
    """
    min_separation2 = min_separation*min_separation

    for urban_item in map_json['urban']:
        if urban_item['type'] != 'bridges':
            continue
        points_list = urban_item['pointdirections']
        remove_indexes = []
        for index1 in range(len(points_list)-1):
            for index2 in range(index1+1, len(points_list)):
                dx = points_list[index1][0][0] - points_list[index2][0][0]
                dy = points_list[index1][0][1] - points_list[index2][0][1]
                separation2 = dx*dx + dy*dy
                if separation2 < min_separation2:
                    remove_indexes.append(index1)
                    break
        if len(remove_indexes) == 0:
            break
        print('Removing ' + str(len(remove_indexes)) + ' duplicate bridges')
        remove_indexes.sort(reverse=True)
        for index in remove_indexes:
            del urban_item['pointdirections'][index]


def deduplicate_maps(maps: [], downscale: int, map_adjust: {},
                     full_deduplication: bool) -> {}:
    """Removes duplicate items
    """
    for map_json in maps:
        if not map_json.get("resolution"):
            continue
        if not map_json.get("filename"):
            continue
        if "full_offset" not in map_json:
            continue
        map_name = get_map_name_from_filename(map_json['filename'])
        x_adjust = 0
        y_adjust = 0
        for map_number, adjust in map_adjust.items():
            if map_number in map_json['filename']:
                x_adjust = adjust['x_adjust']
                y_adjust = adjust['y_adjust']
                break
        
        tx1 = map_json['full_offset'][0] + x_adjust        
        ty1 = map_json['full_offset'][1] + y_adjust
        bx1 = tx1 + map_json['resolution'][0] + x_adjust
        by1 = ty1 + map_json['resolution'][1] + y_adjust
        if tx1 < 0:
            tx1 = 0
        for other_map_json in maps:
            if not other_map_json.get("resolution"):
                continue
            if not other_map_json.get("filename"):
                continue
            if "full_offset" not in other_map_json:
                continue
            other_map_name = get_map_name_from_filename(other_map_json['filename'])
            if other_map_name == map_name:
                continue

            x_adjust = 0
            y_adjust = 0
            for map_number, adjust in map_adjust.items():
                if map_number in other_map_json['filename']:
                    x_adjust = adjust['x_adjust']
                    y_adjust = adjust['y_adjust']
                    break

            tx2 = other_map_json['full_offset'][0] + x_adjust
            ty2 = other_map_json['full_offset'][1] + y_adjust
            bx2 = tx2 + other_map_json['resolution'][0] + x_adjust
            by2 = ty2 + other_map_json['resolution'][1] + y_adjust
            if tx2 < 0:
                tx2 = 0

            if tx2+30 >= tx1 and ty2 >= ty1 and \
               tx2 <= bx1 and ty2 <= by1 and \
               by2 >= by1:
                print('Overlap ' + map_name + ' -> ' + other_map_name)
                overlap_tx = tx2 - other_map_json['full_offset'][0]
                overlap_ty = ty2 - other_map_json['full_offset'][1]
                overlap_bx = bx1 - tx2 + other_map_json['offset'][0]
                overlap_by = by1 - ty2 + other_map_json['offset'][1]
                remove_links(other_map_json,
                             overlap_tx, overlap_ty,
                             overlap_bx, overlap_by, downscale,
                             full_deduplication)


def combine_maps(maps: [], downscale: int, full_deduplication: bool) -> {}:
    """Combines the given json maps. 160, 173, etc
    """
    full_geo_box = maps_bounding_box_geo(maps)
    full_pixels_box = maps_bounding_box_pixels(maps)
    map_adjust = {
        "160": {
            "x_adjust": 0,
            "y_adjust": 96
        },
        "161": {
            "x_adjust": 0,
            "y_adjust": -136 + 96
        },
        "162": {
            "x_adjust": 0,
            "y_adjust": -152 + 96
        },
        "170": {
            "x_adjust": 56,
            "y_adjust": 68
        }
    }

    combined = {
        "offset": [0, 0],
        "resolution": [
            int(full_pixels_box[2] / downscale),
            int(full_pixels_box[3] / downscale)
        ],
        "geocoords": {
            "topleft": {
                "latitude": full_geo_box[1],
                "longitude": full_geo_box[0]
            },
            "bottomright": {
                "latitude": full_geo_box[3],
                "longitude": full_geo_box[2]
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

    deduplicate_maps(maps, 1, map_adjust, full_deduplication)

    building_index = 0
    woods_index = 0
    lake_index = 0
    sea_index = 0
    sands_index = 0
    for json_map in maps:
        x_adjust = 0
        y_adjust = 0
        x_scale = 1
        y_scale = 1
        for map_number, adjust in map_adjust.items():
            if map_number in json_map['filename']:
                x_adjust = adjust['x_adjust']
                y_adjust = adjust['y_adjust']
                if adjust.get('x_scale'):
                    x_scale = \
                        (json_map['resolution'][0] + adjust['x_scale']) / \
                        float(json_map['resolution'][0])
                if adjust.get('y_scale'):
                    y_scale = \
                        (json_map['resolution'][1] + adjust['y_scale']) / \
                        float(json_map['resolution'][1])
                break

        map_geo_box = [
            json_map["geocoords"]["topleft"]["longitude"],
            json_map["geocoords"]["topleft"]["latitude"],
            json_map["geocoords"]["bottomright"]["longitude"],
            json_map["geocoords"]["bottomright"]["latitude"]
        ]
        if json_map.get('terrain'):
            for terrain_item in json_map['terrain']:
                if terrain_item['type'] == 'woodland':
                    for _, woods_points in terrain_item['polygons'].items():
                        woods_areas = \
                            points_to_geo(json_map["full_offset"],
                                          json_map["offset"],
                                          json_map["resolution"],
                                          map_geo_box, full_geo_box,
                                          full_pixels_box,
                                          woods_points['perimeter'],
                                          x_adjust, y_adjust,
                                          x_scale, y_scale,
                                          downscale)
                        combined_woods[str(woods_index)] = {
                            "perimeter": woods_areas
                        }
                        woods_index += 1
                elif terrain_item['type'] == 'lakes':
                    for _, lake_item in terrain_item['polygons'].items():
                        lake_points = points_to_geo(json_map["full_offset"],
                                                    json_map["offset"],
                                                    json_map["resolution"],
                                                    map_geo_box, full_geo_box,
                                                    full_pixels_box,
                                                    lake_item["perimeter"],
                                                    x_adjust, y_adjust,
                                                    x_scale, y_scale,
                                                    downscale)
                        combined_lakes[str(lake_index)] = {
                            "perimeter": lake_points
                        }
                        lake_index += 1
                elif terrain_item['type'] == 'sands':
                    for _, sands_item in terrain_item['polygons'].items():
                        sands_points = points_to_geo(json_map["full_offset"],
                                                     json_map["offset"],
                                                     json_map["resolution"],
                                                     map_geo_box, full_geo_box,
                                                     full_pixels_box,
                                                     sands_item["perimeter"],
                                                     x_adjust, y_adjust,
                                                     x_scale, y_scale,
                                                     downscale)
                        combined_sands[str(sands_index)] = {
                            "perimeter": sands_points
                        }
                        sands_index += 1
                elif terrain_item['type'] == 'sea':
                    for _, sea_item in terrain_item['polygons'].items():
                        sea_points = points_to_geo(json_map["full_offset"],
                                                   json_map["offset"],
                                                   json_map["resolution"],
                                                   map_geo_box, full_geo_box,
                                                   full_pixels_box,
                                                   sea_item["perimeter"],
                                                   x_adjust, y_adjust,
                                                   x_scale, y_scale,
                                                   downscale)
                        combined_sea[str(sea_index)] = {
                            "perimeter": sea_points
                        }
                        sea_index += 1
                elif terrain_item['type'] == 'rivers':
                    curr_river_points = len(combined_rivers["points"])
                    combined_rivers["points"] += \
                        points_to_geo(json_map["full_offset"],
                                      json_map["offset"],
                                      json_map["resolution"],
                                      map_geo_box, full_geo_box,
                                      full_pixels_box,
                                      terrain_item["points"],
                                      x_adjust, y_adjust,
                                      x_scale, y_scale,
                                      downscale)
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
        if json_map.get('urban'):
            for urban_item in json_map['urban']:
                if urban_item['type'] == 'harbours':
                    curr_harbour_points = len(combined_harbours["points"])
                    combined_harbours["points"] += \
                        points_to_geo(json_map["full_offset"],
                                      json_map["offset"],
                                      json_map["resolution"],
                                      map_geo_box, full_geo_box,
                                      full_pixels_box,
                                      urban_item["points"],
                                      x_adjust, y_adjust,
                                      x_scale, y_scale,
                                      downscale)
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
                    combined_railway_lines["points"] += \
                        points_to_geo(json_map["full_offset"],
                                      json_map["offset"],
                                      json_map["resolution"],
                                      map_geo_box, full_geo_box,
                                      full_pixels_box,
                                      urban_item["points"],
                                      x_adjust, y_adjust,
                                      x_scale, y_scale,
                                      downscale)
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
                    combined_railway_tunnels["points"] += \
                        points_to_geo(json_map["full_offset"],
                                      json_map["offset"],
                                      json_map["resolution"],
                                      map_geo_box, full_geo_box,
                                      full_pixels_box,
                                      urban_item["points"],
                                      x_adjust, y_adjust,
                                      x_scale, y_scale,
                                      downscale)
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
                    combined_main_roads["points"] += \
                        points_to_geo(json_map["full_offset"],
                                      json_map["offset"],
                                      json_map["resolution"],
                                      map_geo_box, full_geo_box,
                                      full_pixels_box,
                                      urban_item["points"],
                                      x_adjust, y_adjust,
                                      x_scale, y_scale,
                                      downscale)
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
                    combined_minor_roads["points"] += \
                        points_to_geo(json_map["full_offset"],
                                      json_map["offset"],
                                      json_map["resolution"],
                                      map_geo_box, full_geo_box,
                                      full_pixels_box,
                                      urban_item["points"],
                                      x_adjust, y_adjust,
                                      x_scale, y_scale,
                                      downscale)
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
                        combined_buildings[str(building_index)] = \
                            polygon_to_geo(json_map["full_offset"],
                                           json_map["offset"],
                                           json_map["resolution"],
                                           map_geo_box, full_geo_box,
                                           full_pixels_box,
                                           building_item,
                                           x_adjust, y_adjust,
                                           x_scale, y_scale,
                                           downscale)
                        building_index += 1
                elif urban_item['type'] == 'bridges':
                    new_points = \
                        bridges_to_geo(json_map["full_offset"],
                                       json_map["offset"],
                                       json_map["resolution"],
                                       map_geo_box, full_geo_box,
                                       full_pixels_box,
                                       urban_item["pointdirections"],
                                       x_adjust, y_adjust,
                                       x_scale, y_scale,
                                       downscale)
                    combined_bridges["pointdirections"] += new_points
                elif urban_item['type'] == 'junctions':
                    new_points = \
                        points_to_geo(json_map["full_offset"],
                                      json_map["offset"],
                                      json_map["resolution"],
                                      map_geo_box, full_geo_box,
                                      full_pixels_box,
                                      urban_item["points"],
                                      x_adjust, y_adjust,
                                      x_scale, y_scale,
                                      downscale)
                    combined_junctions["points"] += new_points
                elif urban_item['type'] == 'railway stations':
                    new_points = \
                        points_to_geo(json_map["full_offset"],
                                      json_map["offset"],
                                      json_map["resolution"],
                                      map_geo_box, full_geo_box,
                                      full_pixels_box,
                                      urban_item["points"],
                                      x_adjust, y_adjust,
                                      x_scale, y_scale,
                                      downscale)
                    combined_railway_stations["points"] += new_points
                elif urban_item['type'] == 'orchards':
                    new_points = \
                        points_to_geo(json_map["full_offset"],
                                      json_map["offset"],
                                      json_map["resolution"],
                                      map_geo_box, full_geo_box,
                                      full_pixels_box,
                                      urban_item["points"],
                                      x_adjust, y_adjust,
                                      x_scale, y_scale,
                                      downscale)
                    combined_orchards["points"] += new_points

    combined["terrain"].append({
        "type": "woodland",
        "polygons": combined_woods
    })

    combined["terrain"].append({
        "type": "lakes",
        "polygons": combined_lakes
    })

    combined["terrain"].append({
        "type": "sea",
        "polygons": combined_sea
    })
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

    combined["urban"].append({
        "type": "buildings",
        "polygons": combined_buildings
    })

    print('Map resolution: ' +
          str(int(full_pixels_box[2] / downscale)) + 'x' +
          str(int(full_pixels_box[3] / downscale)))
    print(str(woods_index) + ' woodland areas')
    print(str(lake_index) + ' lake areas')
    print(str(sea_index) + ' sea areas')
    print(str(sands_index) + ' sands areas')
    print(str(len(combined_rivers["points"])) + ' river points')
    print(str(len(combined_harbours["points"])) + ' harbours')
    print(str(len(combined_railway_lines["points"])) + ' railway line points')
    print(str(len(combined_railway_tunnels["points"])) + ' railway tunnel points')
    print(str(len(combined_main_roads["points"])) + ' main road points')
    print(str(len(combined_minor_roads["points"])) + ' minor road points')
    print(str(len(combined_buildings.items())) + ' buildings')
    print(str(len(combined_bridges["pointdirections"])) + ' bridges')
    print(str(len(combined_junctions["points"])) + ' junctions')
    print(str(len(combined_railway_stations["points"])) + ' railway stations')
    print(str(len(combined_orchards["points"])) + ' orchards')
    return combined


def str2bool(value_str) -> bool:
    """Returns true if the given value is a boolean
    """
    if isinstance(value_str, bool):
        return value_str
    if value_str.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    if value_str.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    raise argparse.ArgumentTypeError('Boolean value expected.')


parser = argparse.ArgumentParser(description='combinemaps')
parser.add_argument('--dir', '-d', type=str, dest='maps_dir',
                    default='maps',
                    help='Maps directory')
parser.add_argument('--output', '-o', type=str, dest='output_filename',
                    default='fullmap.json',
                    help='Output filename for the full map json')
parser.add_argument('--downscale', type=int, dest='downscale',
                    default=1,
                    help='Downscaling factor')
parser.add_argument("--full-deduplication", "--full-dedup",
                    dest='full_deduplication',
                    type=str2bool, nargs='?',
                    const=True, default=False,
                    help="Perform full deduplication. This is slow")
args = parser.parse_args()

json_maps = get_json_files(args.maps_dir)
print(str(len(json_maps)) + ' maps loaded')
if len(json_maps) == 0:
    sys.exit()

print("Full deduplication: " + str(args.full_deduplication))
    
json_map_combined = \
    combine_maps(json_maps, args.downscale, args.full_deduplication)

deduplicate_points(json_map_combined, int(20 / args.downscale), 'railway stations')
deduplicate_points(json_map_combined, int(20 / args.downscale), 'junctions')
deduplicate_bridges(json_map_combined, int(20 / args.downscale))

# pprint(json_map)
full_map_str = json.dumps(json_map_combined)

with open(args.output_filename, 'w+') as fp:
    fp.write(full_map_str)
