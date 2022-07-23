
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "map2json.h"
#include "toolkit.h"

n_c_int apesdk_format_used = 0;

int main(int argc, char* argv[])
{
    n_map_data mapdata;
    n_c_int i;
    char * filename = NULL;
    char output_filename[255];
    n_byte * image_data = NULL;
    n_byte4 image_width=0;
    n_byte4 image_height=0;
    n_byte4 image_bitsperpixel=0;
    n_byte * thresholded = NULL;
    n_byte * thresholded_ref = NULL;
    n_byte * dark_image = NULL;
    n_byte * possible_roads = NULL;
    n_byte * original_data = NULL;
    n_c_int proximal_color[3];
    n_c_int proximal_radius=70;
    n_c_int proximal_coverage=23;
    n_c_int adjusted_coverage;
    n_c_int line_search_radius = 10;
    n_c_int max_building_size = 15;
    n_c_int max_building_polygon_width = 1700*max_building_size/100;
    n_c_int max_building_polygon_height = 1000*max_building_size/100;
    n_c_int current_coverage = 0;

    n_c_int woods_coverage = 25;
    n_c_int woods_averaging_radius = 1;
    n_c_int woods_threshold = 30;

    n_c_int sea_threshold_red_low = 160;
    n_c_int sea_threshold_red_high = 245;
    n_c_int sea_threshold_green_low = 180;
    n_c_int sea_threshold_green_high = 240;
    n_c_int sea_threshold_blue_low = 155;
    n_c_int sea_threshold_blue_high = 210;
    n_c_int min_sea_polygon_width = 1700/30;
    n_c_int min_sea_polygon_height = 1000/30;
    n_c_int sea_coverage = 25;
    n_c_int sea_averaging_radius = 2;
    n_c_int sea_area_percent;
    char water_area_name[16];
    char water_filename[32];
    /* percentage of the image after which a water area is considered to be the sea */
    n_c_int sea_threshold = 10;

    n_c_int line_point_radius = 3;

    n_c_int sands_patch_size = 24;
    n_c_int sands_texture_threshold = 15;
    n_c_int sands_coverage = 30;
    n_c_int min_sands_polygon_width = 1700/30;
    n_c_int min_sands_polygon_height = 1000/30;
    n_c_int sands_area_percent;

    n_c_int no_of_widths;
    n_c_int water_coverage = 5;
    n_c_int water_point_spacing = 6;
    n_c_int water_line_link_radius = water_point_spacing*4;
    n_c_int max_river_width = 50;
    n_c_int max_road_width = 50;
    n_c_int water_averaging_radius = 1;
    /* this is the dark blue color typical for rivers */
    n_c_int water_min_blue = 100;
    n_c_int water_max_blue = 200;
    n_c_int water_threshold_red = 5;
    n_c_int water_threshold_green = 5;
    /* this is the blue-green color in the middle of wider rivers,
       which is not the same color as the sea */
    n_c_int water_min_blue2 = 135;
    n_c_int water_max_blue2 = 252;
    n_c_int water_threshold_red2 = 10;
    n_c_int water_threshold_blue_green_diff = 30;
    n_c_int water_segment_connections;

    n_c_int max_clump_points = 6;
    n_c_int min_possible_road_width = 1;
    n_c_int max_possible_road_width = 20;
    n_c_int no_of_links;
    n_c_int road_segment_connections;
    n_c_int road_point_spacing = 4;
    n_c_int road_line_link_radius = road_point_spacing*3;
    n_c_int main_road_coverage = 12;
    n_c_int road_main_threshold_red = 70;
    n_c_int road_main_threshold_green = 20;
    n_c_int road_main_averaging_radius = 1;
    n_c_int road_main_min_red = 130;
    n_c_int road_main_max_red = 252;
    n_c_int road_main_max_green = 150;

    n_c_int minor_road_coverage = 12;
    n_c_int road_minor_threshold_red = 50;
    n_c_int road_minor_threshold_green = 20;
    n_c_int road_minor_min_red = 150;
    n_c_int road_minor_max_red = 241;
    n_c_int road_minor_averaging_radius = 0;
    n_c_int minor_road_join_ends_radius = 20;
    n_c_int potential_roads_radius = 4;
    n_c_int minor_road_background_threshold = 70;

    n_c_int min_station_size = 5;
    n_c_int max_station_size = 20;

    n_c_int tile_overlap_percent = 10;
    n_byte output_specified = 0;

    n_c_int max_total_polygon_points = MAX_TOTAL_POLYGON_POINTS;
    n_c_int max_road_points = MAX_ROAD_POINTS;
    n_c_int max_junctions = MAX_JUNCTIONS;
    n_c_int max_station_points = MAX_STATION_POINTS;
    n_c_int max_orchard_points = MAX_ORCHARD_POINTS;
    n_c_int max_bridges = MAX_BRIDGES;

    n_c_int orchard_tree_diameter = 10;
    n_c_int orchard_tree_spacing = 15;

    n_int railway_line_pixels;
    n_byte railway_line_color_red = 255;
    n_byte railway_line_color_green = 255;
    n_byte railway_line_color_blue = 0;
    n_byte railway_tunnel_color_red = 178;
    n_byte railway_tunnel_color_green = 57;
    n_byte railway_tunnel_color_blue = 255;
    n_c_int railway_line_width = 5;
    n_c_int railway_line_point_spacing = 2;
    n_c_int railway_line_link_radius = railway_line_point_spacing*3;
    n_c_int railway_line_segment_connections;

    n_int harbour_pixels;
    n_c_int harbour_width = 5;
    n_c_int harbour_point_spacing = 2;
    n_c_int harbour_link_radius = harbour_point_spacing*3;
    n_c_int harbour_segment_connections;
    n_byte harbour_color_red = 93;
    n_byte harbour_color_green = 215;
    n_byte harbour_color_blue = 255;

    memset(proximal_color, 0, 3 * sizeof(int));
    sprintf((char*)output_filename,"%s","map.json");

    for (i=1; i<argc; i+=2) {
        if ((strcmp(argv[i],"-f")==0) ||
                (strcmp(argv[i],"--filename")==0)) {
            filename = argv[i+1];
        }

        if ((strcmp(argv[i],"-as")==0) ||
                (strcmp(argv[i],"--apesdk")==0)) {
            apesdk_format_used = 1;
        }

        if ((strcmp(argv[i],"-o")==0) ||
                (strcmp(argv[i],"--output")==0)) {
            sprintf((char*)output_filename,"%s",argv[i+1]);
            output_specified = 1;
        }
        if (strcmp(argv[i],"--treediam")==0) {
            orchard_tree_diameter = atoi(argv[i+1]);
        }
        if (strcmp(argv[i],"--treespacing")==0) {
            orchard_tree_spacing = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"-pc")==0) ||
                (strcmp(argv[i],"--coverage")==0)) {
            proximal_coverage = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"-pr")==0) ||
                (strcmp(argv[i],"--proximalred")==0)) {
            proximal_color[0] = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"-pg")==0) ||
                (strcmp(argv[i],"--proximalgreen")==0)) {
            proximal_color[1] = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"-pb")==0) ||
                (strcmp(argv[i],"--proximalblue")==0)) {
            proximal_color[2] = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--proxradius")==0) ||
                (strcmp(argv[i],"--proximalradius")==0)) {
            proximal_radius = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--roadspacing")==0) ||
                (strcmp(argv[i],"--roadpointspacing")==0)) {
            road_point_spacing = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--mainroadcov")==0) ||
                (strcmp(argv[i],"--mainroadcoverage")==0)) {
            main_road_coverage = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--minorroadbackground")==0) ||
                (strcmp(argv[i],"--minorroadback")==0)) {
            minor_road_background_threshold = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--minorroadcov")==0) ||
                (strcmp(argv[i],"--minorroadcoverage")==0)) {
            minor_road_coverage = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--roadminorredthreshold")==0) ||
                (strcmp(argv[i],"--roadminorredthresh")==0)) {
            road_minor_threshold_red = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--roadminorgreenthreshold")==0) ||
                (strcmp(argv[i],"--roadminorgreenthresh")==0)) {
            road_minor_threshold_green = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--roadminorminred")==0) ||
                (strcmp(argv[i],"--roadminorminr")==0)) {
            road_minor_min_red = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--roadminormaxred")==0) ||
                (strcmp(argv[i],"--roadminormaxr")==0)) {
            road_minor_max_red = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--riverspacing")==0) ||
                (strcmp(argv[i],"--riverpointspacing")==0)) {
            water_point_spacing = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--riverminblue")==0) ||
                (strcmp(argv[i],"--riverminb")==0)) {
            water_min_blue = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--rivermaxblue")==0) ||
                (strcmp(argv[i],"--rivermaxb")==0)) {
            water_max_blue = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--riverminblue2")==0) ||
                (strcmp(argv[i],"--riverminb2")==0)) {
            water_min_blue2 = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--rivermaxblue2")==0) ||
                (strcmp(argv[i],"--rivermaxb2")==0)) {
            water_max_blue2 = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--rivergreen")==0) ||
                (strcmp(argv[i],"--rivergreen")==0)) {
            water_threshold_green = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--riverred")==0) ||
                (strcmp(argv[i],"--riverred")==0)) {
            water_threshold_red = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--riverred2")==0) ||
                (strcmp(argv[i],"--riverred2")==0)) {
            water_threshold_red2 = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--riverbluegreen")==0) ||
                (strcmp(argv[i],"--riverbluegreen")==0)) {
            water_threshold_blue_green_diff = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--riverlinkradius")==0) ||
                (strcmp(argv[i],"-rivlr")==0)) {
            water_line_link_radius = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--linesearch")==0) ||
                (strcmp(argv[i],"--linesrch")==0)) {
            line_search_radius = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--linesearch")==0) ||
                (strcmp(argv[i],"--linesrch")==0)) {
            line_search_radius = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--woodsthreshold")==0) ||
                (strcmp(argv[i],"--woodsthresh")==0)) {
            woods_threshold = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--woodscoverage")==0) ||
                (strcmp(argv[i],"--woodscov")==0)) {
            woods_coverage = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--watercoverage")==0) ||
                (strcmp(argv[i],"--watercov")==0)) {
            water_coverage = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--seacoverage")==0) ||
                (strcmp(argv[i],"--seacov")==0)) {
            sea_coverage = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--searedlow")==0) ||
                (strcmp(argv[i],"-srl")==0)) {
            sea_threshold_red_low = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--searedhigh")==0) ||
                (strcmp(argv[i],"-srh")==0)) {
            sea_threshold_red_high = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--seagreenlow")==0) ||
                (strcmp(argv[i],"-sgl")==0)) {
            sea_threshold_green_low = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--seagreenhigh")==0) ||
                (strcmp(argv[i],"-sgh")==0)) {
            sea_threshold_green_high = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--seabluelow")==0) ||
                (strcmp(argv[i],"-sbl")==0)) {
            sea_threshold_blue_low = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--seabluehigh")==0) ||
                (strcmp(argv[i],"-sbh")==0)) {
            sea_threshold_blue_high = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--maxsize")==0) ||
                (strcmp(argv[i],"-maxs")==0)) {
            max_building_size = atoi(argv[i+1]);
            max_building_polygon_width = 1700*max_building_size/100;
            max_building_polygon_height = 1000*max_building_size/100;
        }
        if ((strcmp(argv[i],"--minstationsize")==0) ||
                (strcmp(argv[i],"-minstatsize")==0)) {
            min_station_size = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--maxstationsize")==0) ||
                (strcmp(argv[i],"-maxstatsize")==0)) {
            max_station_size = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--seathreshold")==0) ||
                (strcmp(argv[i],"-st")==0)) {
            sea_threshold = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--roadlinkradius")==0) ||
                (strcmp(argv[i],"-rlr")==0)) {
            road_line_link_radius = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--overlap")==0) ||
                (strcmp(argv[i],"-olap")==0)) {
            tile_overlap_percent = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--maxclumppoints")==0) ||
                (strcmp(argv[i],"-maxclump")==0)) {
            max_clump_points = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--maxtotalpolygonpoints")==0) ||
                (strcmp(argv[i],"--maxpolypts")==0)) {
            max_total_polygon_points = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--maxroadpoints")==0) ||
                (strcmp(argv[i],"--maxroadpts")==0)) {
            max_road_points = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--maxjunctions")==0) ||
                (strcmp(argv[i],"--maxj")==0)) {
            max_junctions = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--maxstationpoints")==0) ||
                (strcmp(argv[i],"--maxstations")==0)) {
            max_station_points = atoi(argv[i+1]);
        }
        if ((strcmp(argv[i],"--maxbridges")==0) ||
                (strcmp(argv[i],"--maxbr")==0)) {
            max_bridges = atoi(argv[i+1]);
        }
    }

    /* was a file specified */
    if (filename == NULL) {
        printf("No image or json file specified\n");
        return 0;
    }

    /* amount of memory used to store polygons */
    n_c_int polygons_kb = (5*(4*max_total_polygon_points + 1))*(int)sizeof(n_c_int)/1024;
    n_c_int water_kb = (5*max_road_points + 1)*(int)sizeof(n_c_int)/1024;
    n_c_int roads_kb = (3*(4*max_road_points + 1))*(int)sizeof(n_c_int)/1024;
    n_c_int railway_lines_kb = (2*(4*max_road_points + 1))*(int)sizeof(n_c_int)/1024;
    n_c_int harbours_kb = (2*(4*max_road_points + 1))*(int)sizeof(n_c_int)/1024;
    n_c_int junctions_kb = (2*max_junctions + 1)*(int)sizeof(n_c_int)/1024;
    n_c_int stations_kb = (2*max_station_points + 1)*(int)sizeof(n_c_int)/1024;
    n_c_int orchards_kb = (2*max_orchard_points + 1)*(int)sizeof(n_c_int)/1024;
    n_c_int bridges_kb = (2*max_bridges + 1)*(int)sizeof(n_c_int)/1024;
    if (junctions_kb < 1) junctions_kb = 1;
    if (stations_kb < 1) stations_kb = 1;
    if (orchards_kb < 1) orchards_kb = 1;
    if (bridges_kb < 1) bridges_kb = 1;
    n_c_int total_kb =
        polygons_kb + water_kb + roads_kb + junctions_kb +
        stations_kb + orchards_kb + bridges_kb + railway_lines_kb +
        harbours_kb;

    printf("Maximum polygon points:         %d Kb, size %d\n",
           polygons_kb, max_total_polygon_points);
    printf("Maximum river points:           %d Kb, size %d\n",
           water_kb, max_road_points);
    printf("Maximum road points:            %d Kb, size %d\n",
           roads_kb, max_road_points);
    printf("Maximum junction points:        %d Kb, size %d\n",
           junctions_kb, max_junctions);
    printf("Maximum harbour points:         %d Kb, size %d\n",
           harbours_kb, max_road_points);
    printf("Maximum railway line points:    %d Kb, size %d\n",
           railway_lines_kb, max_road_points);
    printf("Maximum railway station points: %d Kb, size %d\n",
           stations_kb, max_station_points);
    printf("Maximum orchard points: %d Kb, size %d\n",
           orchards_kb, max_orchard_points);
    printf("Maximum bridge points:          %d Kb, size %d\n",
           bridges_kb, max_bridges);
    printf("Total map memory:               %d Kb\n\n", total_kb);

    if (create_map_data(max_total_polygon_points,
                        max_road_points,
                        max_junctions,
                        max_station_points,
                        max_orchard_points,
                        max_bridges,
                        &mapdata) != 0) {
        return 1;
    }

    if (strstr(filename, ".json") != NULL) {
        /* load json and convert it into a map image */
        if (output_specified == 0) {
            /* default map image filename */
            sprintf((char*)output_filename,"%s","map.png");
        }
        if (load_map_json(filename,
                          mapdata.buildings.vertices,
                          mapdata.buildings.id,
                          mapdata.buildings.coords,
                          &mapdata.buildings.count,
                          mapdata.woods.vertices,
                          mapdata.woods.id,
                          mapdata.woods.coords,
                          &mapdata.woods.count,
                          mapdata.sands.vertices,
                          mapdata.sands.id,
                          mapdata.sands.coords,
                          &mapdata.sands.count,
                          mapdata.sea.vertices,
                          mapdata.sea.id,
                          mapdata.sea.coords,
                          &mapdata.sea.count,
                          mapdata.water.points,
                          mapdata.water.width,
                          &mapdata.water.count,
                          mapdata.water.links,
                          mapdata.harbour.points,
                          &mapdata.harbour.count,
                          mapdata.harbour.links,
                          mapdata.railway_line.points,
                          &mapdata.railway_line.count,
                          mapdata.railway_line.links,
                          mapdata.railway_tunnel.points,
                          &mapdata.railway_tunnel.count,
                          mapdata.railway_tunnel.links,
                          mapdata.main_road.points,
                          &mapdata.main_road.count,
                          mapdata.main_road.links,
                          mapdata.minor_road.points,
                          &mapdata.minor_road.count,
                          mapdata.minor_road.links,
                          mapdata.junction_points,
                          &mapdata.no_of_junctions,
                          mapdata.station_points,
                          &mapdata.no_of_stations,
                          mapdata.orchard_points,
                          &mapdata.no_of_orchards,
                          mapdata.bridge_points,
                          &mapdata.no_of_bridges,
                          mapdata.offset, mapdata.resolution,
                          mapdata.geocoords) != 0) {
            printf("Unable to load %s\n", filename);
            return 1;
        }

        /* create an image of the correct resolution */
        image_width = mapdata.resolution[0];
        image_height = mapdata.resolution[1];
        image_bitsperpixel = 24;
        image_data = (n_byte*)malloc(image_width*image_height*(image_bitsperpixel/8)*sizeof(n_byte));
        if (image_data == NULL) {
            printf("Unable to allocate image data buffer when loading from json\n");
            free(image_data);
            return 0;
        }

        show_six_lines(image_data, (n_c_int)image_width, (n_c_int)image_height,
                       image_bitsperpixel,
                       mapdata.water.points, mapdata.water.width, mapdata.water.count,
                       mapdata.water.links, 97, 46, 30,
                       mapdata.minor_road.points, mapdata.minor_road.count,
                       mapdata.minor_road.links, 4, 100, 150, 255,
                       mapdata.main_road.points, mapdata.main_road.count,
                       mapdata.main_road.links, 8, 100, 100, 255,
                       mapdata.railway_line.points, mapdata.railway_line.count,
                       mapdata.railway_line.links, 8, 10, 10, 10,
                       mapdata.railway_tunnel.points, mapdata.railway_tunnel.count,
                       mapdata.railway_tunnel.links, 8, 40, 40, 40,
                       mapdata.harbour.points, mapdata.harbour.count,
                       mapdata.harbour.links, 8, 210, 255, 210,
                       mapdata.junction_points, mapdata.no_of_junctions,
                       103, 89, 63);
        show_dots(image_data, image_width, image_height,
                  mapdata.no_of_orchards, mapdata.orchard_points,
                  0, 255, 0, 4);
        show_polygons_filled(image_data, (n_c_int)image_width, (n_c_int)image_height,
                             mapdata.buildings.count,
                             mapdata.buildings.vertices,
                             mapdata.buildings.coords,
                             50, 50, 50);
        show_polygons_empty(image_data, (n_c_int)image_width, (n_c_int)image_height,
                            mapdata.buildings.count,
                            mapdata.buildings.vertices,
                            mapdata.buildings.id,
                            mapdata.buildings.coords, 103, 89, 63);
        show_polygons_filled(image_data, (n_c_int)image_width, (n_c_int)image_height,
                             mapdata.woods.count,
                             mapdata.woods.vertices,
                             mapdata.woods.coords,
                             75, 113, 107);
        show_crossings(image_data, (n_c_int)image_width, (n_c_int)image_height, image_bitsperpixel,
                       mapdata.bridge_points, mapdata.no_of_bridges,
                       50, 50, 50, 0, 0, 255,
                       20, 10);
        show_polygons_filled(image_data, (n_c_int)image_width, (n_c_int)image_height,
                             mapdata.sea.count,
                             mapdata.sea.vertices,
                             mapdata.sea.coords,
                             77, 26, 10);
        show_polygons_filled(image_data, (n_c_int)image_width, (n_c_int)image_height,
                             mapdata.sands.count,
                             mapdata.sands.vertices,
                             mapdata.sands.coords,
                             100, 100, 100);
        write_png_file(output_filename,
                       image_width, image_height, 24, image_data);
        free(image_data);
        return 0;
    }

    image_data = read_png_file(filename, &image_width, &image_height, &image_bitsperpixel);
    if (image_data == NULL) {
        printf("Couldn't load image %s\n", filename);
        return 0;
    }
    if ((image_width == 0) || (image_height==0)) {
        printf("Couldn't load image size %dx%d\n", image_width, image_height);
        return 0;
    }
    if (image_bitsperpixel == 0) {
        printf("Couldn't load image depth\n");
        return 0;
    }

    printf("Image: %s\n", filename);
    printf("Resolution: %dx%d\n", image_width, image_height);
    printf("Depth: %d\n", image_bitsperpixel);

    if (apesdk_format_used == 0)
    {

        if ((image_width > MAX_TILE_WIDTH) || (image_height > MAX_TILE_HEIGHT)) {
            if ((image_width < MIN_SOURCE_MAP_IMAGE_WIDTH) ||
                    (image_height < MIN_SOURCE_MAP_IMAGE_HEIGHT)) {
                printf("Large images are expected to be at least %dx%d resolution.\n",
                       MIN_SOURCE_MAP_IMAGE_WIDTH, MIN_SOURCE_MAP_IMAGE_HEIGHT);
                free(image_data);
                return 1;
            }
            printf("A large map image was loaded.\n");
            harbour_pixels =
                color_pixel_count(image_data,
                                  (n_c_int)image_width, (n_c_int)image_height,
                                  harbour_color_red,
                                  harbour_color_green,
                                  harbour_color_blue);
            railway_line_pixels =
                color_pixel_count(image_data,
                                  (n_c_int)image_width, (n_c_int)image_height,
                                  railway_line_color_red,
                                  railway_line_color_green,
                                  railway_line_color_blue);
            if (railway_line_pixels + harbour_pixels > 0) {
                printf("Railway lines / harbours image\n");
                printf("Allocating buffers\n");
                original_data =
                    (n_byte*)malloc(image_width*image_height*
                                    (image_bitsperpixel/8));
                if (original_data == NULL) {
                    printf("Unable to allocate original image data buffer\n");
                    return 0;
                }
                memcpy(original_data, image_data,
                       image_width*image_height*(image_bitsperpixel/8));
                thresholded =
                    (n_byte*)malloc(image_width*image_height*
                                    sizeof(n_byte));
                if (thresholded == NULL) {
                    printf("Unable to allocate memory for thresholded image");
                }
                thresholded_ref =
                    (n_byte*)malloc(image_width*image_height*
                                    sizeof(n_byte));
                if (thresholded_ref == NULL) {
                    printf("Unable to allocate memory for thresholded reference image");
                }

                printf("Lines converting to binary image\n");
                color_pixel_to_binary(image_data,
                                      (n_c_int)image_width, (n_c_int)image_height,
                                      harbour_color_red,
                                      harbour_color_green,
                                      harbour_color_blue,
                                      thresholded, 1);
                memcpy(thresholded_ref, thresholded, image_width*image_height*sizeof(n_byte));
                mono_to_color(thresholded, image_width, image_height,
                              image_bitsperpixel, image_data);
                write_png_file("harbours_stage1.png",
                               image_width, image_height, 24, image_data);
                printf("Skeletonizing\n");
                skeletonize_simple(thresholded,
                                   (n_c_int)image_width, (n_c_int)image_height,
                                   image_data, image_bitsperpixel,
                                   harbour_width);
                write_png_file("harbours_stage2.png",
                               image_width, image_height, 24, image_data);
                mapdata.harbour.count =
                    skeleton_to_points(image_data, image_width, image_height,
                                       image_bitsperpixel,
                                       harbour_point_spacing,
                                       mapdata.harbour.points,
                                       MAX_ROAD_POINTS,
                                       harbour_point_spacing);
                mapdata.harbour.count =
                    remove_close_points(mapdata.harbour.points,
                                        mapdata.harbour.count,
                                        harbour_point_spacing);
                printf("%d harbour points\n", mapdata.harbour.count);

                show_line_points(image_data, image_width, image_height,
                                 mapdata.harbour.points,
                                 mapdata.harbour.count,
                                 2);
                write_png_file("harbours_stage3.png",
                               image_width, image_height, 24, image_data);

                mapdata.harbour.count =
                    thin_line_point_clumps(mapdata.harbour.points,
                                           mapdata.harbour.count,
                                           harbour_point_spacing,
                                           max_clump_points);

                show_line_points(image_data, image_width, image_height,
                                 mapdata.harbour.points,
                                 mapdata.harbour.count,
                                 2);
                write_png_file("harbours_stage4.png",
                               image_width, image_height, 24, image_data);
                no_of_links =
                    link_line_points(mapdata.harbour.points,
                                     mapdata.harbour.count,
                                     harbour_link_radius*4,
                                     mapdata.harbour.links,
                                     thresholded_ref,
                                     image_width, image_height, 0);
                harbour_segment_connections =
                    connect_line_sections(mapdata.harbour.points,
                                          mapdata.harbour.count,
                                          harbour_link_radius,
                                          mapdata.harbour.links);
                printf("%d harbour point links\n", no_of_links);
                printf("%d harbour segment connections\n",
                       harbour_segment_connections);
                show_lines(image_data, image_width, image_height, image_bitsperpixel,
                           mapdata.harbour.points, mapdata.harbour.count,
                           mapdata.harbour.links, 4, 100, 100, 100, 0, 1);
                write_png_file("harbours_stage5.png",
                               image_width, image_height, 24, image_data);

                printf("Lines converting to binary image\n");
                memcpy(image_data, original_data,
                       image_width*image_height*(image_bitsperpixel/8));
                color_pixel_to_binary(image_data,
                                      (n_c_int)image_width, (n_c_int)image_height,
                                      railway_line_color_red,
                                      railway_line_color_green,
                                      railway_line_color_blue,
                                      thresholded, 1);
                memcpy(thresholded_ref, thresholded, image_width*image_height*sizeof(n_byte));
                mono_to_color(thresholded, image_width, image_height,
                              image_bitsperpixel, image_data);
                write_png_file("railway_lines_stage1.png",
                               image_width, image_height, 24, image_data);
                printf("Skeletonizing\n");
                skeletonize_simple(thresholded,
                                   (n_c_int)image_width, (n_c_int)image_height,
                                   image_data, image_bitsperpixel,
                                   railway_line_width);
                write_png_file("railway_lines_stage2.png",
                               image_width, image_height, 24, image_data);
                mapdata.railway_line.count =
                    skeleton_to_points(image_data, image_width, image_height,
                                       image_bitsperpixel,
                                       railway_line_point_spacing,
                                       mapdata.railway_line.points,
                                       MAX_ROAD_POINTS,
                                       railway_line_point_spacing);
                mapdata.railway_line.count =
                    remove_close_points(mapdata.railway_line.points,
                                        mapdata.railway_line.count,
                                        railway_line_point_spacing);
                printf("%d railway line points\n", mapdata.railway_line.count);

                show_line_points(image_data, image_width, image_height,
                                 mapdata.railway_line.points,
                                 mapdata.railway_line.count,
                                 2);
                write_png_file("railway_lines_stage3.png",
                               image_width, image_height, 24, image_data);

                mapdata.railway_line.count =
                    thin_line_point_clumps(mapdata.railway_line.points,
                                           mapdata.railway_line.count,
                                           railway_line_point_spacing,
                                           max_clump_points);

                show_line_points(image_data, image_width, image_height,
                                 mapdata.railway_line.points,
                                 mapdata.railway_line.count,
                                 2);
                write_png_file("railway_lines_stage4.png",
                               image_width, image_height, 24, image_data);
                no_of_links =
                    link_line_points(mapdata.railway_line.points,
                                     mapdata.railway_line.count,
                                     railway_line_link_radius*4,
                                     mapdata.railway_line.links,
                                     thresholded_ref,
                                     image_width, image_height, 0);
                railway_line_segment_connections =
                    connect_line_sections(mapdata.railway_line.points,
                                          mapdata.railway_line.count,
                                          railway_line_link_radius,
                                          mapdata.railway_line.links);
                printf("%d railway line point links\n", no_of_links);
                printf("%d railway line segment connections\n",
                       railway_line_segment_connections);
                show_lines(image_data, image_width, image_height, image_bitsperpixel,
                           mapdata.railway_line.points, mapdata.railway_line.count,
                           mapdata.railway_line.links, 4, 100, 100, 100, 0, 1);
                write_png_file("railway_lines_stage5.png",
                               image_width, image_height, 24, image_data);

                memcpy(image_data, original_data,
                       image_width*image_height*(image_bitsperpixel/8));
                printf("Tunnels converting to binary image\n");
                color_pixel_to_binary(image_data,
                                      (n_c_int)image_width, (n_c_int)image_height,
                                      railway_tunnel_color_red,
                                      railway_tunnel_color_green,
                                      railway_tunnel_color_blue,
                                      thresholded, 1);
                memcpy(thresholded_ref, thresholded, image_width*image_height*sizeof(n_byte));
                mono_to_color(thresholded, image_width, image_height,
                              image_bitsperpixel, image_data);
                write_png_file("railway_tunnels_stage1.png",
                               image_width, image_height, 24, image_data);
                printf("Skeletonizing\n");
                skeletonize_simple(thresholded,
                                   (n_c_int)image_width, (n_c_int)image_height,
                                   image_data, image_bitsperpixel,
                                   railway_line_width);
                write_png_file("railway_tunnels_stage2.png",
                               image_width, image_height, 24, image_data);
                mapdata.railway_tunnel.count =
                    skeleton_to_points(image_data,
                                       (n_c_int)image_width, (n_c_int)image_height,
                                       image_bitsperpixel,
                                       railway_line_point_spacing,
                                       mapdata.railway_tunnel.points,
                                       MAX_ROAD_POINTS,
                                       railway_line_point_spacing);
                mapdata.railway_tunnel.count =
                    remove_close_points(mapdata.railway_tunnel.points,
                                        mapdata.railway_tunnel.count,
                                        railway_line_point_spacing);
                printf("%d railway tunnel points\n", mapdata.railway_tunnel.count);

                show_line_points(image_data, image_width, image_height,
                                 mapdata.railway_tunnel.points,
                                 mapdata.railway_tunnel.count,
                                 2);
                write_png_file("railway_tunnels_stage3.png",
                               image_width, image_height, 24, image_data);

                mapdata.railway_tunnel.count =
                    thin_line_point_clumps(mapdata.railway_tunnel.points,
                                           mapdata.railway_tunnel.count,
                                           railway_line_point_spacing,
                                           max_clump_points);

                show_line_points(image_data, image_width, image_height,
                                 mapdata.railway_tunnel.points,
                                 mapdata.railway_tunnel.count,
                                 2);
                write_png_file("railway_tunnels_stage4.png",
                               image_width, image_height, 24, image_data);
                no_of_links =
                    link_line_points(mapdata.railway_tunnel.points,
                                     mapdata.railway_tunnel.count,
                                     railway_line_link_radius*4,
                                     mapdata.railway_tunnel.links,
                                     thresholded_ref,
                                     image_width, image_height, 0);
                railway_line_segment_connections =
                    connect_line_sections(mapdata.railway_tunnel.points,
                                          mapdata.railway_tunnel.count,
                                          railway_line_link_radius,
                                          mapdata.railway_tunnel.links);
                show_lines(image_data, image_width, image_height, image_bitsperpixel,
                           mapdata.railway_line.points, mapdata.railway_line.count,
                           mapdata.railway_line.links, 4, 100, 100, 100, 0, 1);
                show_lines(image_data, image_width, image_height, image_bitsperpixel,
                           mapdata.railway_tunnel.points, mapdata.railway_tunnel.count,
                           mapdata.railway_tunnel.links, 4, 100, 0, 100, 0, 0);
                write_png_file("railway.png",
                               image_width, image_height, 24, image_data);

                if (strcmp(output_filename, "map.json") == 0) {
                    sprintf((char*)output_filename,"%s","railway.json");
                }
                save_railway_json(output_filename,
                                  mapdata.railway_line.count,
                                  mapdata.railway_line.points,
                                  mapdata.railway_line.links,
                                  mapdata.railway_tunnel.count,
                                  mapdata.railway_tunnel.points,
                                  mapdata.railway_tunnel.links,
                                  mapdata.harbour.count,
                                  mapdata.harbour.points,
                                  mapdata.harbour.links);

                free(thresholded);
                free(thresholded_ref);
                free(image_data);
                free(original_data);
                printf("Ended Successfully\n");
                return 0;
            }

            printf("Creating tiles.\n");
            /* break up the map into tiles */
            if (breakup_large_map(filename,
                                  image_data, image_width, image_height,
                                  image_bitsperpixel, "tiles",
                                  tile_overlap_percent) == 0) {
                printf("Tiles created\n");
            }
            free(image_data);
            printf("Ended Successfully\n");
            return 0;
        }

    }

    original_data = (n_byte*)malloc(image_width*image_height*(image_bitsperpixel/8)*sizeof(n_byte));
    if (original_data == NULL) {
        printf("Unable to allocate original image data buffer\n");
        return 0;
    }

    /* make a copy of the original image data */
    memcpy(original_data, image_data, image_width*image_height*(image_bitsperpixel/8));

    if (image_bitsperpixel != 3*8) {
        printf("Expected 3 bytes per pixel\n");
        return 0;
    }

    thresholded = (n_byte*)malloc(image_width*image_height*sizeof(n_byte));
    if (thresholded == NULL) {
        printf("Unable to allocate memory for thresholded image");
    }

    thresholded_ref = (n_byte*)malloc(image_width*image_height*sizeof(n_byte));
    if (thresholded_ref == NULL) {
        printf("Unable to allocate memory for thresholded reference image");
    }

    dark_image = (n_byte*)malloc(image_width*image_height*sizeof(n_byte));
    if (dark_image == NULL) {
        printf("Unable to allocate memory for dark image");
    }

    possible_roads = (n_byte*)malloc(image_width*image_height*sizeof(n_byte));
    if (possible_roads == NULL) {
        printf("Unable to allocate memory for potential roads image");
    }

    /* detect sea/lakes */
    memcpy(image_data, original_data, image_width*image_height*(image_bitsperpixel/8));
    detect_color(image_data, image_width, image_height, image_bitsperpixel, 1,
                 sea_threshold_red_low, sea_threshold_red_high,
                 sea_threshold_green_low, sea_threshold_green_high,
                 sea_threshold_blue_low, sea_threshold_blue_high,
                 sea_averaging_radius);
    write_png_file("sea_stage1.png", image_width, image_height, 24, image_data);
    /* extract polygons for sea */
    color_to_binary(image_data, image_width, image_height,
                    image_bitsperpixel, 250, thresholded);
    proximal_erase(thresholded, image_width, image_height,
                   10, sea_coverage);
    mapdata.sea.count = \
                        proximal_fill(thresholded, image_width, image_height,
                                      image_data,
                                      min_sea_polygon_width, min_sea_polygon_height,
                                      image_width, image_height,
                                      4,
                                      mapdata.sea.id,
                                      mapdata.sea.vertices,
                                      mapdata.sea.coords,
                                      MAX_TOTAL_POLYGON_POINTS, 0);
    /* detection of sea or lakes just depends upon the percentage of the image
       covered by water color */
    sea_area_percent =
        get_polygons_total_area(image_width, image_height,
                                mapdata.sea.count, mapdata.sea.vertices, mapdata.sea.coords);
    printf("Water area: %d%%\n", sea_area_percent);
    if (sea_area_percent > sea_threshold) {
        sprintf(water_area_name,"%s","sea");
    }
    else {
        sprintf(water_area_name,"%s","lake");
    }
    sprintf(water_filename,"%s_stage2.png", water_area_name);
    write_png_file(water_filename,
                   image_width, image_height, 24, image_data);
    printf("%d %s areas\n", mapdata.sea.count, water_area_name);
    /* save polygon image for sea */
    show_polygons(image_data, image_width, image_height,
                  mapdata.sea.count,
                  mapdata.sea.id,
                  mapdata.sea.vertices,
                  mapdata.sea.coords);
    sprintf(water_filename,"%s.png", water_area_name);
    write_png_file(water_filename, image_width, image_height, 24, image_data);

    /* detect sands */
    memcpy(image_data, original_data, image_width*image_height*(image_bitsperpixel/8));

    detect_color(image_data, image_width, image_height, image_bitsperpixel, 1,
                 sea_threshold_red_low, sea_threshold_red_high,
                 sea_threshold_green_low, sea_threshold_green_high,
                 sea_threshold_blue_low, sea_threshold_blue_high,
                 sea_averaging_radius);
    write_png_file("sands_stage1.png", image_width, image_height, 24, image_data);

    detect_texture(image_data, (n_c_int)image_width, (n_c_int)image_height,
                   sands_patch_size, sands_texture_threshold,
                   thresholded);

    mono_to_color(thresholded, image_width, image_height,
                  image_bitsperpixel, image_data);
    write_png_file("sands_stage2.png", image_width, image_height, 24, image_data);


    proximal_erase(thresholded, image_width, image_height,
                   10, sands_coverage);
    mapdata.sands.count = \
                          proximal_fill(thresholded, image_width, image_height,
                                        image_data,
                                        min_sands_polygon_width, min_sands_polygon_height,
                                        image_width, image_height,
                                        4,
                                        mapdata.sands.id,
                                        mapdata.sands.vertices,
                                        mapdata.sands.coords,
                                        MAX_TOTAL_POLYGON_POINTS, 0);
    sands_area_percent =
        get_polygons_total_area(image_width, image_height,
                                mapdata.sands.count, mapdata.sands.vertices, mapdata.sands.coords);
    printf("Sands area: %d%%\n", sands_area_percent);
    write_png_file("sands_stage3.png",
                   image_width, image_height, 24, image_data);
    /* save polygon image for sands */
    show_polygons(image_data, image_width, image_height,
                  mapdata.sands.count,
                  mapdata.sands.id,
                  mapdata.sands.vertices,
                  mapdata.sands.coords);
    write_png_file("sands_stage4.png", image_width, image_height, 24, image_data);
    if (sea_area_percent < sea_threshold) {
        /* append sands to lakes/sea */
        append_polygons(&mapdata.sea, &mapdata.sands);
    }

    /* detect orchards */
    memcpy(image_data, original_data, image_width*image_height*(image_bitsperpixel/8));
    proximal_threshold(image_data, image_width, image_height,
                       proximal_color[0], proximal_color[1], proximal_color[2],
                       proximal_radius, thresholded);
    mapdata.orchards.count = \
                             proximal_fill(thresholded, image_width, image_height,
                                           image_data,
                                           orchard_tree_diameter*3/4, orchard_tree_diameter*3/4,
                                           orchard_tree_diameter, orchard_tree_diameter,
                                           4,
                                           mapdata.orchards.id,
                                           mapdata.orchards.vertices,
                                           mapdata.orchards.coords,
                                           MAX_TOTAL_POLYGON_POINTS, 1);
    write_png_file("orchard_stage1.png", image_width, image_height, 24, image_data);
    detect_dots(&mapdata.orchards, orchard_tree_spacing,
                mapdata.orchard_points,
                image_data, image_width, image_height);
    mapdata.no_of_orchards = mapdata.orchards.count;
    printf("%d orchards detected\n", mapdata.orchards.count);
    write_png_file("orchard_stage2.png", image_width, image_height, 24, image_data);

    /* detect buildings */
    memcpy(image_data, original_data, image_width*image_height*(image_bitsperpixel/8));
    proximal_threshold(image_data, image_width, image_height,
                       proximal_color[0], proximal_color[1], proximal_color[2],
                       proximal_radius, thresholded);
    remove_nogo_areas(thresholded, image_width, image_height,
                      mapdata.sea.count, mapdata.sea.vertices, mapdata.sea.coords);
    memcpy(dark_image, thresholded, image_width*image_height*sizeof(n_byte));
    mono_to_color(dark_image, image_width, image_height,
                  image_bitsperpixel, image_data);
    write_png_file("dark_image.png", image_width, image_height, 24, image_data);
    potential_roads(thresholded, image_width, image_height,
                    possible_roads,
                    min_possible_road_width, max_possible_road_width);
    mono_to_color(possible_roads, image_width, image_height,
                  image_bitsperpixel, image_data);
    write_png_file("roads_possible.png", image_width, image_height, 24, image_data);
    mono_to_color(thresholded, image_width, image_height,
                  image_bitsperpixel, image_data);
    write_png_file("buildings_stage1.png", image_width, image_height, 24, image_data);
    remove_high_frequency(thresholded, image_width, image_height, 4);
    mono_to_color(thresholded, image_width, image_height,
                  image_bitsperpixel, image_data);
    write_png_file("buildings_stage2.png", image_width, image_height, 24, image_data);
    current_coverage = percent_coverage(thresholded,image_width, image_height);
    printf("Building coverage: %d\n", current_coverage);
    /* adjust the coverage threshold so that it's lower in denser urban areas */
    adjusted_coverage = proximal_coverage - ((current_coverage - 16)*2);
    if (adjusted_coverage < 10) adjusted_coverage = 10;
    if (adjusted_coverage > 25) adjusted_coverage = 25;
    /* remove areas without enough coverage */
    proximal_erase(thresholded, image_width, image_height,
                   20, adjusted_coverage);
    /* make a copy of the binary image which can be used to check
       links later on */
    memcpy(thresholded_ref, thresholded, image_width*image_height*sizeof(n_byte));
    mono_to_color(thresholded, image_width, image_height,
                  image_bitsperpixel, image_data);
    write_png_file("buildings_stage3.png", image_width, image_height, 24, image_data);

    /* extract polygons for buildings */
    mapdata.buildings.count = \
                              proximal_fill(thresholded_ref, image_width, image_height,
                                            image_data,
                                            0, 0,
                                            max_building_polygon_width,
                                            max_building_polygon_height,
                                            4,
                                            mapdata.buildings.id,
                                            mapdata.buildings.vertices,
                                            mapdata.buildings.coords,
                                            MAX_TOTAL_POLYGON_POINTS, 1);

    /* save building areas */
    write_png_file("buildings_stage4.png",
                   image_width, image_height, 24, image_data);

    printf("Polygons %d\n", mapdata.buildings.count);

    /* save polygon fit image */
    show_polygons_against_reference(image_data, image_width, image_height,
                                    thresholded,
                                    mapdata.buildings.count,
                                    mapdata.buildings.id,
                                    mapdata.buildings.vertices,
                                    mapdata.buildings.coords, 1);
    write_png_file("buildings_polygon_fit_inner.png", image_width, image_height, 24, image_data);
    show_polygons_against_reference(image_data, image_width, image_height,
                                    thresholded,
                                    mapdata.buildings.count,
                                    mapdata.buildings.id,
                                    mapdata.buildings.vertices,
                                    mapdata.buildings.coords, 0);
    write_png_file("buildings_polygon_fit_outer.png", image_width, image_height, 24, image_data);
    /* save polygon image */
    show_polygons(image_data, image_width, image_height,
                  mapdata.buildings.count,
                  mapdata.buildings.id,
                  mapdata.buildings.vertices,
                  mapdata.buildings.coords);

    /* save the image */
    write_png_file("buildings.png", image_width, image_height, 24, image_data);

    /* detect woodland */
    memcpy(image_data, original_data, image_width*image_height*(image_bitsperpixel/8));
    detect_green(image_data, image_width, image_height, image_bitsperpixel,
                 woods_threshold, woods_averaging_radius);
    write_png_file("woods_stage1.png", image_width, image_height, 24, image_data);
    /* extract polygons for woodland */
    color_to_binary(image_data, image_width, image_height,
                    image_bitsperpixel, 250, thresholded);
    proximal_erase(thresholded, image_width, image_height,
                   10, woods_coverage);
    mapdata.woods.count = \
                          proximal_fill(thresholded, image_width, image_height,
                                        image_data,
                                        0, 0, 1700, 1000,
                                        4,
                                        mapdata.woods.id,
                                        mapdata.woods.vertices,
                                        mapdata.woods.coords,
                                        MAX_TOTAL_POLYGON_POINTS, 0);
    write_png_file("woods_stage2.png",
                   image_width, image_height, 24, image_data);
    printf("%d woodland areas\n", mapdata.woods.count);
    /* save polygon image */
    show_polygons(image_data, image_width, image_height,
                  mapdata.woods.count,
                  mapdata.woods.id,
                  mapdata.woods.vertices,
                  mapdata.woods.coords);
    write_png_file("woods.png", image_width, image_height, 24, image_data);

    /* detect water */
    memcpy(image_data, original_data, image_width*image_height*(image_bitsperpixel/8));
    detect_blue(image_data, image_width, image_height, image_bitsperpixel,
                water_threshold_red,
                water_threshold_green,
                water_averaging_radius,
                water_min_blue,
                water_max_blue);
    write_png_file("water_stage1.png", image_width, image_height, 24, image_data);
    color_to_binary(image_data, image_width, image_height,
                    image_bitsperpixel, water_max_blue, thresholded);
    detect_blue_green(original_data,
                      image_width, image_height, image_bitsperpixel,
                      water_threshold_red2,
                      water_threshold_blue_green_diff,
                      water_averaging_radius,
                      water_min_blue2, water_max_blue2,
                      thresholded);
    mono_to_color(thresholded, image_width, image_height,
                  image_bitsperpixel, image_data);
    write_png_file("water_stage2.png", image_width, image_height, 24, image_data);
    proximal_erase(thresholded, image_width, image_height,
                   10, water_coverage);
    /* make a copy of the binary image which can be used to check
       links later on */
    memcpy(thresholded_ref, thresholded, image_width*image_height*sizeof(n_byte));
    skeletonize(thresholded, image_width, image_height,
                image_data, image_bitsperpixel, max_river_width);
    write_png_file("water_stage3.png", image_width, image_height, 24, image_data);
    mapdata.water.count =
        skeleton_to_points(image_data, image_width, image_height,
                           image_bitsperpixel, water_point_spacing,
                           mapdata.water.points, MAX_ROAD_POINTS,
                           water_point_spacing*3/2);
    mapdata.water.count =
        remove_close_points(mapdata.water.points, mapdata.water.count,
                            water_point_spacing);
    printf("%d water line points\n", mapdata.water.count);
    no_of_widths =
        detect_line_point_widths(thresholded_ref, image_width, image_height,
                                 mapdata.water.points, mapdata.water.count,
                                 mapdata.water.width, max_river_width);
    printf("%d water widths\n", no_of_widths);
    show_line_points(image_data, image_width, image_height,
                     mapdata.water.points, mapdata.water.count,
                     line_point_radius);
    write_png_file("water_stage4.png", image_width, image_height, 24, image_data);
    no_of_links =
        link_line_points(mapdata.water.points, mapdata.water.count,
                         water_line_link_radius, mapdata.water.links,
                         thresholded_ref, image_width, image_height, 2);
    printf("%d water point links\n", no_of_links);
    water_segment_connections =
        connect_line_sections(mapdata.water.points, mapdata.water.count,
                              water_line_link_radius*3, mapdata.water.links);
    printf("%d water segment connections\n", water_segment_connections);
    show_lines_with_width(image_data, image_width, image_height, image_bitsperpixel,
                          mapdata.water.points, mapdata.water.count,
                          mapdata.water.links, mapdata.water.width, 255, 0, 0);
    write_png_file("water_stage5.png",
                   image_width, image_height, 24, image_data);

    /* detect main roads */
    memcpy(image_data, original_data, image_width*image_height*(image_bitsperpixel/8));
    detect_red(image_data, image_width, image_height, image_bitsperpixel,
               road_main_threshold_red,
               road_main_threshold_green,
               road_main_averaging_radius,
               road_main_min_red,
               road_main_max_red,
               road_main_max_green);
    write_png_file("roads_main_stage1.png", image_width, image_height, 24, image_data);
    color_to_binary(image_data, image_width, image_height,
                    image_bitsperpixel, 127, thresholded);
    /* make a copy of the binary image which can be used to check
       links later on */
    memcpy(thresholded_ref, thresholded, image_width*image_height*sizeof(n_byte));
    mono_to_color(thresholded, image_width, image_height,
                  image_bitsperpixel, image_data);
    write_png_file("roads_main_stage2.png",
                   image_width, image_height, 24, image_data);
    proximal_erase(thresholded, image_width, image_height,
                   10, main_road_coverage);
    skeletonize(thresholded, image_width, image_height,
                image_data, image_bitsperpixel, max_road_width);
    write_png_file("roads_main_stage3.png", image_width, image_height, 24, image_data);
    /* detect railway stations */
    mapdata.no_of_stations =
        detect_blobs(thresholded_ref, image_width, image_height,
                     line_search_radius, MAX_STATION_POINTS, mapdata.station_points,
                     min_station_size, max_station_size, image_data);
    if (mapdata.no_of_stations > 0) {
        printf("%d station points\n", mapdata.no_of_stations);
        show_line_points(image_data, image_width, image_height,
                         mapdata.station_points, mapdata.no_of_stations,
                         line_point_radius);
        write_png_file("stations.png", image_width, image_height, 24, image_data);
    }
    /* continue with main road detection */
    skeletonize(thresholded, image_width, image_height,
                image_data, image_bitsperpixel, max_road_width);
    mapdata.main_road.count =
        skeleton_to_points(image_data, image_width, image_height,
                           image_bitsperpixel, road_point_spacing,
                           mapdata.main_road.points, MAX_ROAD_POINTS,
                           road_point_spacing*3/2);
    /* remove any road points which are railway stations */
    mapdata.main_road.count =
        remove_blobs_from_points(mapdata.station_points, mapdata.no_of_stations,
                                 mapdata.main_road.points, mapdata.main_road.count,
                                 max_station_size);
    /* remove surplus main road points */
    mapdata.main_road.count =
        remove_close_points(mapdata.main_road.points, mapdata.main_road.count,
                            road_point_spacing);
    show_line_points(image_data, image_width, image_height,
                     mapdata.main_road.points, mapdata.main_road.count,
                     line_point_radius);
    printf("%d main road line points\n", mapdata.main_road.count);
    write_png_file("roads_main_stage4.png", image_width, image_height, 24, image_data);

    mapdata.main_road.count =
        thin_line_point_clumps(mapdata.main_road.points, mapdata.main_road.count,
                               road_point_spacing*3, max_clump_points);
    show_line_points(image_data, image_width, image_height,
                     mapdata.main_road.points, mapdata.main_road.count,
                     line_point_radius);
    write_png_file("roads_main_stage5.png", image_width, image_height, 24, image_data);

    no_of_links =
        link_line_points(mapdata.main_road.points, mapdata.main_road.count,
                         road_line_link_radius, mapdata.main_road.links,
                         thresholded_ref, image_width, image_height, 2);
    road_segment_connections =
        connect_line_sections(mapdata.main_road.points, mapdata.main_road.count,
                              road_line_link_radius*4, mapdata.main_road.links);
    printf("%d main road point links\n", no_of_links);
    printf("%d main road segment connections\n", road_segment_connections);
    show_lines(image_data, image_width, image_height, image_bitsperpixel,
               mapdata.main_road.points, mapdata.main_road.count,
               mapdata.main_road.links, 4, 100, 100, 255, 1, 1);
    write_png_file("roads_main_stage6.png", image_width, image_height, 24, image_data);

    /* detect minor roads */
    memcpy(image_data, original_data, image_width*image_height*(image_bitsperpixel/8));
    detect_orange(image_data, image_width, image_height, image_bitsperpixel,
                  road_minor_threshold_red,
                  road_minor_threshold_green,
                  road_minor_averaging_radius,
                  road_minor_min_red,
                  road_minor_max_red);
    write_png_file("roads_minor_stage1.png",
                   image_width, image_height, 24, image_data);
    color_to_binary(image_data, image_width, image_height,
                    image_bitsperpixel, 252, thresholded);
    subtract_image(thresholded, image_width, image_height,
                   possible_roads, dark_image, potential_roads_radius,
                   minor_road_background_threshold);
    /* make a copy of the binary image which can be used to check
       links later on */
    memcpy(thresholded_ref, thresholded, image_width*image_height*sizeof(n_byte));
    mono_to_color(thresholded, image_width, image_height,
                  image_bitsperpixel, image_data);
    write_png_file("roads_minor_stage2.png",
                   image_width, image_height, 24, image_data);
    proximal_erase(thresholded, image_width, image_height,
                   10, minor_road_coverage);
    skeletonize(thresholded, image_width, image_height,
                image_data, image_bitsperpixel, max_road_width);
    write_png_file("roads_minor_stage3.png",
                   image_width, image_height, 24, image_data);
    mapdata.minor_road.count =
        skeleton_to_points(image_data, image_width, image_height,
                           image_bitsperpixel, road_point_spacing,
                           mapdata.minor_road.points, MAX_ROAD_POINTS,
                           road_point_spacing*3/2);
    /* remove surplus minor road points */
    mapdata.minor_road.count =
        remove_close_points(mapdata.minor_road.points, mapdata.minor_road.count,
                            road_point_spacing);
    printf("%d minor road line points\n", mapdata.minor_road.count);
    show_line_points(image_data, image_width, image_height,
                     mapdata.minor_road.points, mapdata.minor_road.count,
                     line_point_radius);
    write_png_file("roads_minor_stage4.png",
                   image_width, image_height, 24, image_data);

    mapdata.minor_road.count =
        thin_line_point_clumps(mapdata.minor_road.points, mapdata.minor_road.count,
                               road_point_spacing*3, max_clump_points);

    show_line_points(image_data, image_width, image_height,
                     mapdata.minor_road.points, mapdata.minor_road.count,
                     line_point_radius);
    write_png_file("roads_minor_stage5.png",
                   image_width, image_height, 24, image_data);
    no_of_links =
        link_line_points(mapdata.minor_road.points, mapdata.minor_road.count,
                         road_line_link_radius, mapdata.minor_road.links,
                         thresholded_ref, image_width, image_height, 2);
    road_segment_connections =
        connect_line_sections(mapdata.minor_road.points, mapdata.minor_road.count,
                              road_line_link_radius*4, mapdata.minor_road.links);
    printf("%d minor road point links\n", no_of_links);
    printf("%d minor road segment connections\n", road_segment_connections);
    show_lines(image_data, image_width, image_height, image_bitsperpixel,
               mapdata.minor_road.points, mapdata.minor_road.count,
               mapdata.minor_road.links, 4, 100, 150, 255, 1, 1);
    write_png_file("roads_minor_stage6.png",
                   image_width, image_height, 24, image_data);

    /* bridges */
    mapdata.no_of_bridges =
        detect_crossings(mapdata.main_road.points, mapdata.main_road.count,
                         mapdata.main_road.links,
                         mapdata.minor_road.points, mapdata.minor_road.count,
                         mapdata.minor_road.links,
                         mapdata.water.points, mapdata.water.count,
                         mapdata.water.links,
                         mapdata.bridge_points, MAX_BRIDGES);
    printf("%d bridges\n", mapdata.no_of_bridges);

    /* junctions */
    mapdata.no_of_junctions =
        detect_junctions(mapdata.main_road.points, mapdata.main_road.count,
                         mapdata.main_road.links,
                         mapdata.minor_road.points, mapdata.minor_road.count,
                         mapdata.minor_road.links,
                         mapdata.junction_points, MAX_JUNCTIONS,
                         minor_road_join_ends_radius);
    printf("%d junctions\n", mapdata.no_of_junctions);
    /* all roads */
    show_six_lines(image_data, image_width, image_height,
                   image_bitsperpixel,
                   mapdata.water.points, mapdata.water.width, mapdata.water.count,
                   mapdata.water.links, 97, 46, 30,
                   mapdata.minor_road.points, mapdata.minor_road.count,
                   mapdata.minor_road.links, 4, 100, 150, 255,
                   mapdata.main_road.points, mapdata.main_road.count,
                   mapdata.main_road.links, 8, 100, 100, 255,
                   mapdata.railway_line.points, mapdata.railway_line.count,
                   mapdata.railway_line.links, 8, 10, 10, 10,
                   mapdata.railway_tunnel.points, mapdata.railway_tunnel.count,
                   mapdata.railway_tunnel.links, 8, 40, 40, 40,
                   mapdata.harbour.points, mapdata.harbour.count,
                   mapdata.harbour.links, 8, 210, 255, 210,
                   mapdata.junction_points, mapdata.no_of_junctions,
                   103, 89, 63);
    show_dots(image_data, image_width, image_height,
              mapdata.no_of_orchards, mapdata.orchard_points,
              0, 255, 0, 4);
    show_polygons_filled(image_data, image_width, image_height,
                         mapdata.buildings.count,
                         mapdata.buildings.vertices,
                         mapdata.buildings.coords,
                         50, 50, 50);
    show_polygons_empty(image_data, image_width, image_height,
                        mapdata.buildings.count,
                        mapdata.buildings.vertices,
                        mapdata.buildings.id,
                        mapdata.buildings.coords, 103, 89, 63);
    show_polygons_filled(image_data, image_width, image_height,
                         mapdata.woods.count,
                         mapdata.woods.vertices,
                         mapdata.woods.coords,
                         75, 113, 107);
    show_crossings(image_data, image_width, image_height, image_bitsperpixel,
                   mapdata.bridge_points, mapdata.no_of_bridges,
                   50, 50, 50, 0, 0, 255,
                   20, 10);
    show_polygons_filled(image_data, image_width, image_height,
                         mapdata.sea.count,
                         mapdata.sea.vertices,
                         mapdata.sea.coords,
                         77, 26, 10);
    show_polygons_filled(image_data, image_width, image_height,
                         mapdata.sands.count,
                         mapdata.sands.vertices,
                         mapdata.sands.coords,
                         100, 100, 100);
    write_png_file("roads.png",
                   image_width, image_height, 24, image_data);

    /* save a json file for this map */
    mapdata.railway_line.count = 0;
    mapdata.railway_tunnel.count = 0;
    save_map_json(filename, mapdata.buildings.count,
                  mapdata.buildings.id,
                  mapdata.buildings.vertices,
                  mapdata.buildings.coords,
                  mapdata.harbour.count,
                  mapdata.harbour.points,
                  mapdata.harbour.links,
                  mapdata.railway_line.count,
                  mapdata.railway_line.points,
                  mapdata.railway_line.links,
                  mapdata.railway_tunnel.count,
                  mapdata.railway_tunnel.points,
                  mapdata.railway_tunnel.links,
                  mapdata.main_road.count,
                  mapdata.main_road.points,
                  mapdata.main_road.links,
                  mapdata.minor_road.count,
                  mapdata.minor_road.points,
                  mapdata.minor_road.links,
                  mapdata.woods.count,
                  mapdata.woods.id,
                  mapdata.woods.vertices,
                  mapdata.woods.coords,
                  mapdata.sands.count,
                  mapdata.sands.id,
                  mapdata.sands.vertices,
                  mapdata.sands.coords,
                  mapdata.sea.count,
                  mapdata.sea.id,
                  mapdata.sea.vertices,
                  mapdata.sea.coords,
                  mapdata.water.count,
                  mapdata.water.points, mapdata.water.width,
                  mapdata.water.links,
                  mapdata.no_of_stations,
                  mapdata.station_points,
                  mapdata.no_of_orchards,
                  mapdata.orchard_points,
                  sea_area_percent, sea_threshold,
                  mapdata.bridge_points, mapdata.no_of_bridges,
                  mapdata.junction_points, mapdata.no_of_junctions,
                  image_width, image_height,
                  output_filename);

    /* test */
    load_map_json("map.json",
                  mapdata.buildings.vertices,
                  mapdata.buildings.id,
                  mapdata.buildings.coords,
                  &mapdata.buildings.count,
                  mapdata.woods.vertices,
                  mapdata.woods.id,
                  mapdata.woods.coords,
                  &mapdata.woods.count,
                  mapdata.sands.vertices,
                  mapdata.sands.id,
                  mapdata.sands.coords,
                  &mapdata.sands.count,
                  mapdata.sea.vertices,
                  mapdata.sea.id,
                  mapdata.sea.coords,
                  &mapdata.sea.count,
                  mapdata.water.points,
                  mapdata.water.width,
                  &mapdata.water.count,
                  mapdata.water.links,
                  mapdata.harbour.points,
                  &mapdata.harbour.count,
                  mapdata.harbour.links,
                  mapdata.railway_line.points,
                  &mapdata.railway_line.count,
                  mapdata.railway_line.links,
                  mapdata.railway_tunnel.points,
                  &mapdata.railway_tunnel.count,
                  mapdata.railway_tunnel.links,
                  mapdata.main_road.points,
                  &mapdata.main_road.count,
                  mapdata.main_road.links,
                  mapdata.minor_road.points,
                  &mapdata.minor_road.count,
                  mapdata.minor_road.links,
                  mapdata.junction_points,
                  &mapdata.no_of_junctions,
                  mapdata.station_points,
                  &mapdata.no_of_stations,
                  mapdata.orchard_points,
                  &mapdata.no_of_orchards,
                  mapdata.bridge_points,
                  &mapdata.no_of_bridges,
                  mapdata.offset, mapdata.resolution, mapdata.geocoords);

    show_six_lines(image_data, image_width, image_height,
                   image_bitsperpixel,
                   mapdata.water.points, mapdata.water.width, mapdata.water.count,
                   mapdata.water.links, 97, 46, 30,
                   mapdata.minor_road.points, mapdata.minor_road.count,
                   mapdata.minor_road.links, 4, 100, 150, 255,
                   mapdata.main_road.points, mapdata.main_road.count,
                   mapdata.main_road.links, 8, 100, 100, 255,
                   mapdata.railway_line.points, mapdata.railway_line.count,
                   mapdata.railway_line.links, 8, 10, 10, 10,
                   mapdata.railway_tunnel.points, mapdata.railway_tunnel.count,
                   mapdata.railway_tunnel.links, 8, 40, 40, 40,
                   mapdata.harbour.points, mapdata.harbour.count,
                   mapdata.harbour.links, 8, 210, 255, 210,
                   mapdata.junction_points, mapdata.no_of_junctions,
                   103, 89, 63);
    show_polygons_filled(image_data, image_width, image_height,
                         mapdata.buildings.count,
                         mapdata.buildings.vertices,
                         mapdata.buildings.coords,
                         50, 50, 50);
    show_polygons_empty(image_data, image_width, image_height,
                        mapdata.buildings.count,
                        mapdata.buildings.vertices,
                        mapdata.buildings.id,
                        mapdata.buildings.coords, 103, 89, 63);
    show_polygons_filled(image_data, image_width, image_height,
                         mapdata.woods.count,
                         mapdata.woods.vertices,
                         mapdata.woods.coords,
                         75, 113, 107);
    show_polygons_filled(image_data, image_width, image_height,
                         mapdata.orchards.count,
                         mapdata.orchards.vertices,
                         mapdata.orchards.coords,
                         75, 113, 75);
    show_crossings(image_data, image_width, image_height, image_bitsperpixel,
                   mapdata.bridge_points, mapdata.no_of_bridges,
                   50, 50, 50, 0, 0, 255,
                   20, 10);
    show_polygons_filled(image_data, image_width, image_height,
                         mapdata.sea.count,
                         mapdata.sea.vertices,
                         mapdata.sea.coords,
                         77, 26, 10);
    show_polygons_filled(image_data, image_width, image_height,
                         mapdata.sands.count,
                         mapdata.sands.vertices,
                         mapdata.sands.coords,
                         100, 100, 100);
    write_png_file("load_test.png",
                   image_width, image_height, 24, image_data);


    sprintf((char*)output_filename,"%s","map2.json");
    mapdata.railway_line.count = 0;
    mapdata.railway_tunnel.count = 0;
    save_map_json(output_filename, mapdata.buildings.count,
                  mapdata.buildings.id,
                  mapdata.buildings.vertices,
                  mapdata.buildings.coords,
                  mapdata.harbour.count,
                  mapdata.harbour.points,
                  mapdata.harbour.links,
                  mapdata.railway_line.count,
                  mapdata.railway_line.points,
                  mapdata.railway_line.links,
                  mapdata.railway_tunnel.count,
                  mapdata.railway_tunnel.points,
                  mapdata.railway_tunnel.links,
                  mapdata.main_road.count,
                  mapdata.main_road.points,
                  mapdata.main_road.links,
                  mapdata.minor_road.count,
                  mapdata.minor_road.points,
                  mapdata.minor_road.links,
                  mapdata.woods.count,
                  mapdata.woods.id,
                  mapdata.woods.vertices,
                  mapdata.woods.coords,
                  mapdata.sands.count,
                  mapdata.sands.id,
                  mapdata.sands.vertices,
                  mapdata.sands.coords,
                  mapdata.sea.count,
                  mapdata.sea.id,
                  mapdata.sea.vertices,
                  mapdata.sea.coords,
                  mapdata.water.count,
                  mapdata.water.points, mapdata.water.width,
                  mapdata.water.links,
                  mapdata.no_of_stations,
                  mapdata.station_points,
                  mapdata.no_of_orchards,
                  mapdata.orchard_points,
                  sea_area_percent, sea_threshold,
                  mapdata.bridge_points, mapdata.no_of_bridges,
                  mapdata.junction_points, mapdata.no_of_junctions,
                  image_width, image_height,
                  output_filename);

    /* free memory */
    free(possible_roads);
    free(dark_image);
    free(thresholded);
    free(thresholded_ref);
    free(image_data);
    free(original_data);
    free_map_data(&mapdata);

    printf("Ended Successfully\n");
    return 0;
}
