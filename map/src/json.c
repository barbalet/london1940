#include "map2json.h"
#include "toolkit.h"
#include <sys/stat.h>

extern n_c_int apesdk_format_used;

enum json_sections {
    JSON_SECTION_TERRAIN = 1,
    JSON_SECTION_URBAN,
    JSON_SECTION_OFFSET,
    JSON_SECTION_RESOLUTION,
    JSON_SECTION_GEOCOORDS
};

enum json_subsections {
    JSON_SUBSECTION_WOODLAND = 1,
    JSON_SUBSECTION_RIVERS,
    JSON_SUBSECTION_LAKES,
    JSON_SUBSECTION_SANDS,
    JSON_SUBSECTION_SEA,
    JSON_SUBSECTION_HARBOURS,
    JSON_SUBSECTION_RAILWAY_STATIONS,
    JSON_SUBSECTION_RAILWAY_LINES,
    JSON_SUBSECTION_RAILWAY_TUNNELS,
    JSON_SUBSECTION_ORCHARDS,
    JSON_SUBSECTION_BRIDGES,
    JSON_SUBSECTION_JUNCTIONS,
    JSON_SUBSECTION_MAIN_ROADS,
    JSON_SUBSECTION_MINOR_ROADS,
    JSON_SUBSECTION_BUILDINGS,
    JSON_SUBSECTION_TOP_LEFT,
    JSON_SUBSECTION_BOTTOM_RIGHT
};

enum json_parameters {
    JSON_PARAM_PERIMETER = 1,
    JSON_PARAM_INTERIOR,
    JSON_PARAM_EXTERIOR,
    JSON_PARAM_LINKS,
    JSON_PARAM_POINTS,
    JSON_PARAM_WIDTHS,
    JSON_PARAM_LATITUDE,
    JSON_PARAM_LONGITUDE
};

static void get_geocoords_from_filename(char filename[],
                                        n_c_int * tile_x, n_c_int * tile_y,
                                        float * long_tx, float * lat_ty,
                                        float * long_bx, float * lat_by,
                                        n_c_int * pixels_tx,
                                        n_c_int * pixels_ty)
{
    n_c_int index = strlen(filename)-1, buffer_index = 0, num_index = 0, found = 0;
    n_c_int coord_int = 0;
    char buffer[256];
    float coord = 0;

    while (index > 0) {
        if (filename[index] == '/') {
            found = 1;
            break;
        }
        index--;
    }
    if (found == 1) {
        index++;
    }
    else {
        index = 0;
    }
    while(index < (n_c_int)strlen(filename)) {
        if ((filename[index] == '_') ||
                (filename[index] == '#') ||
                (filename[index] == '.')) {
            buffer[buffer_index] = 0;
            if ((num_index >= 2) && (num_index < 6)) {
                coord = atof(buffer) / 1000.0;
            }
            else {
                coord_int = (n_c_int)atoi(buffer);
            }
            switch(num_index) {
            case 0: {
                *tile_y = coord_int;
                break;
            }
            case 1: {
                *tile_x = coord_int;
                break;
            }
            case 2: {
                *lat_ty = coord;
                break;
            }
            case 3: {
                *long_tx = coord;
                break;
            }
            case 4: {
                *lat_by = coord;
                break;
            }
            case 5: {
                *long_bx = coord;
                break;
            }
            case 6: {
                *pixels_tx = coord_int;
                break;
            }
            case 7: {
                *pixels_ty = coord_int;
                break;
            }
            }
            buffer_index = 0;
            num_index++;
            if (num_index == 8) break;
        }
        else {
            buffer[buffer_index++] = filename[index];
        }
        index++;
    }
}

void json_points(FILE * fp, n_c_int *points, n_c_int pixels_tx, n_c_int pixels_ty)
{
    fprintf(fp, "[%d,%d]",
            points[0]+pixels_tx,
            points[1]+pixels_ty);
}

void json_point_and_direction(FILE * fp, n_c_int * lines, n_c_int pixels_tx, n_c_int pixels_ty)
{
    fprintf(fp, "[[%d,%d],[%d,%d]]",
            lines[0]+pixels_tx, lines[1]+pixels_ty,
            lines[2], lines[3]);
}


void json_links(FILE * fp, n_c_int *links, n_c_int pixels_tx, n_c_int pixels_ty)
{
    fprintf(fp, "[%d,%d]", links[0]-1, links[1]-1);
}

void polygons_map_json_apesdk(FILE * fp, n_c_int no_of_sea, n_c_int * sea_id, n_c_int * sea_vertices, n_c_int * sea, n_c_int pixels_tx, n_c_int pixels_ty)
{
    n_c_int p = 0,v = 0, sea_index = 0;
    fprintf(fp, "    \"polygons\": [");
    for (p = 0; p < no_of_sea; p++) {
        fprintf(fp, "[");

        for (v = 0; v < sea_vertices[p]; v++, sea_index++) {

            if (v > 0) {
                fprintf(fp, ",");
            }

            json_points(fp, &sea[sea_index*2], pixels_tx, pixels_ty);
        }
//        fprintf(fp, "]\n");

        if (p < no_of_sea - 1) {
            fprintf(fp, "],");
        }
        else {
            fprintf(fp, "]");
        }
    }
    fprintf(fp, "]\n");
}


void polygons_map_json(FILE * fp, n_c_int no_of_sea, n_c_int * sea_id, n_c_int * sea_vertices, n_c_int * sea, n_c_int pixels_tx, n_c_int pixels_ty)
{
    if (apesdk_format_used == 1)
    {
        polygons_map_json_apesdk(fp, no_of_sea, sea_id, sea_vertices, sea, pixels_tx, pixels_ty);
        return;
    }

    n_c_int p = 0,v = 0, sea_index = 0;
    fprintf(fp, "    \"polygons\": {\n");
    for (p = 0; p < no_of_sea; p++) {
        fprintf(fp, "    \"%d\": {\n", sea_id[p]);
        fprintf(fp, "      \"perimeter\": [");
        for (v = 0; v < sea_vertices[p]; v++, sea_index++) {

            if (v > 0) {
                fprintf(fp, ",");
            }

            json_points(fp, &sea[sea_index*2], pixels_tx, pixels_ty);
        }
        fprintf(fp, "]\n");

        if (p < no_of_sea - 1) {
            fprintf(fp, "    },\n");
        }
        else {
            fprintf(fp, "    }\n");
        }
    }
    fprintf(fp, "  }\n");
}


void polygons_map_json_building(FILE * fp, n_c_int no_of_polygons, n_c_int * polygon_id, n_c_int * polygon_vertices, n_c_int * polygons, n_c_int pixels_tx, n_c_int pixels_ty)
{
    n_c_int p = 0,v = 0;
    n_c_int vertex_index = 0;
    n_c_int is_interior = 0;
    n_c_int next_interior = 0;

    fprintf(fp, "    \"polygons\": {\n");

    for (p = 0; p < no_of_polygons; p++) {
        is_interior = 0;
        if (p > 0) {
            if (polygon_id[p - 1] == polygon_id[p]) {
                is_interior = 1;
            }
        }
        if (is_interior == 0) {
            fprintf(fp, "    \"%d\": {\n", polygon_id[p]);
            fprintf(fp, "      \"ext\": [");
        }
        else {
            fprintf(fp, ",\n");
            fprintf(fp, "      \"int\": [");
        }
        for (v = 0; v < polygon_vertices[p]; v++, vertex_index++) {
            if (v > 0) {
                fprintf(fp, ",");
            }

            json_points(fp, &polygons[vertex_index*2], pixels_tx, pixels_ty);
        }
        fprintf(fp, "]");
        next_interior = 0;
        if (p < no_of_polygons - 1) {
            if (polygon_id[p + 1] == polygon_id[p]) {
                next_interior = 1;
            }
        }
        if (next_interior == 0) {
            /* has no interior */
            fprintf(fp, "\n");
            fprintf(fp, "    }");

            if (p < no_of_polygons - 1) {
                fprintf(fp, ",");
            }
            fprintf(fp, "\n");
        }
    }

    /* end of buildings */
    fprintf(fp, "  }\n");
}


n_c_int save_railway_json(char * filename,
                          n_c_int no_of_railway_line_points,
                          n_c_int railway_line_points[],
                          n_c_int railway_line_links[],
                          n_c_int no_of_railway_tunnel_points,
                          n_c_int railway_tunnel_points[],
                          n_c_int railway_tunnel_links[],
                          n_c_int no_of_harbour_points,
                          n_c_int harbour_points[],
                          n_c_int harbour_links[])
{
    n_c_int p = 0;
    FILE * fp;

    fp = fopen(filename, "w");
    if (fp == 0) return 1;


    fprintf(fp, "[\n");

    fprintf(fp, "  {\n");

    fprintf(fp, "    \"type\": \"harbours\",\n");
    fprintf(fp, "    \"points\": [");
    for (p = 0; p < no_of_harbour_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }
        json_points(fp, &harbour_points[p*2], 0, 0);
    }
    fprintf(fp, "],\n");
    fprintf(fp, "    \"links\": [");
    for (p = 0; p < no_of_harbour_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }

        json_links(fp, &harbour_links[p*2], 0, 0);
    }
    fprintf(fp, "]\n");

    fprintf(fp, "  },\n");

    fprintf(fp, "  {\n");

    fprintf(fp, "    \"type\": \"railway lines\",\n");
    fprintf(fp, "    \"points\": [");
    for (p = 0; p < no_of_railway_line_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }
        json_points(fp, &railway_line_points[p*2], 0, 0);
    }
    fprintf(fp, "],\n");
    fprintf(fp, "    \"links\": [");
    for (p = 0; p < no_of_railway_line_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }

        json_links(fp, &railway_line_links[p*2], 0, 0);
    }
    fprintf(fp, "]\n");

    fprintf(fp, "  },\n");


    fprintf(fp, "  {\n");
    fprintf(fp, "    \"type\": \"railway tunnels\",\n");
    fprintf(fp, "    \"points\": [");
    for (p = 0; p < no_of_railway_tunnel_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }
        json_points(fp, &railway_tunnel_points[p*2], 0, 0);
    }
    fprintf(fp, "],\n");
    fprintf(fp, "    \"links\": [");
    for (p = 0; p < no_of_railway_tunnel_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }

        json_links(fp, &railway_tunnel_links[p*2], 0, 0);
    }
    fprintf(fp, "]\n");
    fprintf(fp, "  }\n");
    fprintf(fp, "]\n");

    fclose(fp);
    return 0;
}


n_c_int save_map_json(char source_filename[],
                      n_c_int no_of_polygons,
                      n_c_int * polygon_id,
                      n_c_int * polygon_vertices,
                      n_c_int * polygons,
                      n_c_int no_of_harbour_points,
                      n_c_int harbour_points[],
                      n_c_int harbour_links[],
                      n_c_int no_of_railway_line_points,
                      n_c_int railway_line_points[],
                      n_c_int railway_line_links[],
                      n_c_int no_of_railway_tunnel_points,
                      n_c_int railway_tunnel_points[],
                      n_c_int railway_tunnel_links[],
                      n_c_int no_of_main_road_points,
                      n_c_int main_road_points[],
                      n_c_int main_road_links[],
                      n_c_int no_of_minor_road_points,
                      n_c_int minor_road_points[],
                      n_c_int minor_road_links[],
                      n_c_int no_of_woods,
                      n_c_int woods_id[],
                      n_c_int woods_vertices[],
                      n_c_int woods[],
                      n_c_int no_of_sands,
                      n_c_int sands_id[],
                      n_c_int sands_vertices[],
                      n_c_int sands[],
                      n_c_int no_of_sea,
                      n_c_int sea_id[],
                      n_c_int sea_vertices[],
                      n_c_int sea[],
                      n_c_int no_of_water_points,
                      n_c_int water_points[],
                      n_c_int water_point_width[],
                      n_c_int water_links[],
                      n_c_int no_of_stations,
                      n_c_int station_points[],
                      n_c_int no_of_orchards,
                      n_c_int orchards[],
                      n_c_int sea_area_percent,
                      n_c_int sea_threshold,
                      n_c_int bridge_points[],
                      n_c_int no_of_bridges,
                      n_c_int junction_points[], n_c_int no_of_junctions,
                      n_c_int image_width, n_c_int image_height,
                      char * filename)
{
    n_c_int pixels_tx = 0;
    n_c_int pixels_ty = 0;
    n_c_int tile_x = 0, tile_y = 0;
    n_c_int p = 0;
    FILE * fp;
    float long_tx = 0;
    float lat_ty = 0;
    float long_bx = 0;
    float lat_by = 0;

    get_geocoords_from_filename(source_filename,
                                &tile_x, &tile_y,
                                &long_tx, &lat_ty, &long_bx, &lat_by,
                                &pixels_tx, &pixels_ty);

    fp = fopen(filename, "w");
    if (fp == 0) return 1;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"sourceFilename\": \"%s\",\n", source_filename);
    fprintf(fp, "  \"tile\": [%d,%d],\n", tile_x, tile_y);
    fprintf(fp, "  \"offset\": [%d,%d],\n", pixels_tx, pixels_ty);
    fprintf(fp, "  \"resolution\": [%d,%d],\n", image_width, image_height);

    if (apesdk_format_used == 0)
    {
        fprintf(fp, "  \"geocoords\": {\n");
        fprintf(fp, "    \"topleft\": {\n");
        fprintf(fp, "      \"latitude\": %f,\n", lat_ty);
        fprintf(fp, "      \"longitude\": %f\n", long_tx);
        fprintf(fp, "    },\n");
        fprintf(fp, "    \"bottomright\": {\n");
        fprintf(fp, "      \"latitude\": %f,\n", lat_by);
        fprintf(fp, "      \"longitude\": %f\n", long_bx);
        fprintf(fp, "    }\n");
        fprintf(fp, "  },\n");
    }

    fprintf(fp, "  \"terrain\": [\n");

    fprintf(fp, "  {\n");
    fprintf(fp, "  \"type\": \"woodland\",\n");

    polygons_map_json(fp, no_of_woods, woods_id, woods_vertices, woods, pixels_tx, pixels_ty);

    fprintf(fp, "  },\n");

    fprintf(fp, "  {\n");
    fprintf(fp, "    \"type\": \"rivers\",\n");
    fprintf(fp, "    \"points\": [");
    for (p = 0; p < no_of_water_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }

        json_points(fp, &water_points[p*2], pixels_tx, pixels_ty);
    }
    fprintf(fp, "],\n");
    fprintf(fp, "    \"widths\": [");
    for (p = 0; p < no_of_water_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }
        fprintf(fp, "%d", water_point_width[p]);
    }
    fprintf(fp, "],\n");
    fprintf(fp, "    \"links\": [");
    for (p = 0; p < no_of_water_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }

        json_links(fp, &water_links[p*2], pixels_tx, pixels_ty);
    }
    fprintf(fp, "]\n");
    fprintf(fp, "  }\n");

    if ((apesdk_format_used == 0) || (no_of_sands!= 0))
    {
        fprintf(fp, "  ,{\n");
        fprintf(fp, "    \"type\": \"sands\",\n");

        polygons_map_json(fp, no_of_sands, sands_id, sands_vertices, sands, pixels_tx, pixels_ty);


        fprintf(fp, "  }\n");
    }

    if (sea_area_percent > sea_threshold) {

        if (apesdk_format_used == 0)
        {
            fprintf(fp, "  ,{\n");
            fprintf(fp, "    \"type\": \"lakes\",\n");
            fprintf(fp, "    \"polygons\": {}");
            fprintf(fp, "  }\n");
        }

        fprintf(fp, "  ,{\n");
        fprintf(fp, "    \"type\": \"sea\",\n");
        polygons_map_json(fp, no_of_sea, sea_id, sea_vertices, sea, pixels_tx, pixels_ty);
        fprintf(fp, "  }\n");
    }
    else
    {

        if (apesdk_format_used == 0)
        {
            fprintf(fp, "  ,{\n");
            fprintf(fp, "    \"type\": \"sea\",\n");
            fprintf(fp, "    \"polygons\": {}");
            fprintf(fp, "  },");

            fprintf(fp, "  {\n");
            fprintf(fp, "    \"type\": \"lakes\",\n");
            polygons_map_json(fp, no_of_sea, sea_id, sea_vertices, sea, pixels_tx, pixels_ty);
            fprintf(fp, "  }\n");
        }
    }

    fprintf(fp, "  ],\n");

    fprintf(fp, "  \"urban\": [\n");

    fprintf(fp, "  {\n");
    fprintf(fp, "    \"type\": \"railway stations\",\n");
    fprintf(fp, "    \"points\": [");
    for (p = 0; p < no_of_stations; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }

        json_points(fp, &station_points[p*2], pixels_tx, pixels_ty);
    }
    fprintf(fp, "]\n");
    fprintf(fp, "  },\n");

    fprintf(fp, "  {\n");
    fprintf(fp, "    \"type\": \"orchards\",\n");
    fprintf(fp, "    \"points\": [");
    for (p = 0; p < no_of_orchards; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }

        json_points(fp, &orchards[p*2], pixels_tx, pixels_ty);
    }
    fprintf(fp, "]\n");
    fprintf(fp, "  },\n");

    fprintf(fp, "  {\n");
    fprintf(fp, "    \"type\": \"bridges\",\n");
    fprintf(fp, "    \"pointdirections\": [");
    for (p = 0; p < no_of_bridges; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }
        json_point_and_direction(fp, &bridge_points[p*4], pixels_tx, pixels_ty);
    }
    fprintf(fp, "]\n");
    fprintf(fp, "  },\n");
    fprintf(fp, "  {\n");
    fprintf(fp, "    \"type\": \"junctions\",\n");
    fprintf(fp, "    \"points\": [");
    for (p = 0; p < no_of_junctions; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }
        json_points(fp, &junction_points[p*2], pixels_tx, pixels_ty);
    }
    fprintf(fp, "]\n");

    fprintf(fp, "  },\n");


    if (apesdk_format_used == 0 ||
            ((no_of_harbour_points != 0) ||
             (no_of_railway_line_points != 0) ||
             (no_of_railway_tunnel_points != 0)))
    {
        if ((apesdk_format_used == 0) || (no_of_harbour_points != 0))
        {

            fprintf(fp, "  {\n");
            fprintf(fp, "    \"type\": \"harbours\",\n");
            fprintf(fp, "    \"points\": [");
            for (p = 0; p < no_of_harbour_points; p++) {
                if (p > 0) {
                    fprintf(fp, ",");
                }
                json_points(fp, &harbour_points[p*2], pixels_tx, pixels_ty);
            }
            fprintf(fp, "],\n");
            fprintf(fp, "    \"links\": [");
            for (p = 0; p < no_of_harbour_points; p++) {
                if (p > 0) {
                    fprintf(fp, ",");
                }

                json_links(fp, &harbour_links[p*2], pixels_tx, pixels_ty);
            }
            fprintf(fp, "]\n");
            fprintf(fp, "  }\n");
        }
        if ((no_of_harbour_points == 0) ||
                (no_of_railway_line_points != 0) ||
                (no_of_railway_tunnel_points != 0))
        {
            fprintf(fp, ",\n");
        }

        if ((apesdk_format_used == 0) || (no_of_railway_line_points != 0))
        {

            fprintf(fp, "  {\n");
            fprintf(fp, "    \"type\": \"railway lines\",\n");
            fprintf(fp, "    \"points\": [");
            for (p = 0; p < no_of_railway_line_points; p++) {
                if (p > 0) {
                    fprintf(fp, ",");
                }
                json_points(fp, &railway_line_points[p*2], pixels_tx, pixels_ty);
            }
            fprintf(fp, "],\n");
            fprintf(fp, "    \"links\": [");
            for (p = 0; p < no_of_railway_line_points; p++) {
                if (p > 0) {
                    fprintf(fp, ",");
                }

                json_links(fp, &railway_line_links[p*2], pixels_tx, pixels_ty);
            }
            fprintf(fp, "]\n");
            fprintf(fp, "  }\n");
        }
        if ((no_of_railway_line_points == 0) || (no_of_railway_tunnel_points != 0))
        {
            fprintf(fp, ",\n");
        }

        if ((apesdk_format_used == 0) || (no_of_railway_tunnel_points != 0))
        {
            fprintf(fp, "  {\n");
            fprintf(fp, "    \"type\": \"railway tunnels\",\n");
            fprintf(fp, "    \"points\": [");
            for (p = 0; p < no_of_railway_tunnel_points; p++) {
                if (p > 0) {
                    fprintf(fp, ",");
                }
                json_points(fp, &railway_tunnel_points[p*2], pixels_tx, pixels_ty);
            }
            fprintf(fp, "],\n");
            fprintf(fp, "    \"links\": [");
            for (p = 0; p < no_of_railway_tunnel_points; p++) {
                if (p > 0) {
                    fprintf(fp, ",");
                }

                json_links(fp, &railway_tunnel_links[p*2], pixels_tx, pixels_ty);
            }
            fprintf(fp, "]\n");
            fprintf(fp, "  },\n");
        }
    }
    fprintf(fp, "  {\n");
    fprintf(fp, "    \"type\": \"main roads\",\n");
    fprintf(fp, "    \"points\": [");
    for (p = 0; p < no_of_main_road_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }
        json_points(fp, &main_road_points[p*2], pixels_tx, pixels_ty);
    }
    fprintf(fp, "],\n");



    fprintf(fp, "    \"links\": [");
    for (p = 0; p < no_of_main_road_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }

        json_links(fp, &main_road_links[p*2], pixels_tx, pixels_ty);
    }
    fprintf(fp, "]\n");
    fprintf(fp, "  },\n");

    fprintf(fp, "  {\n");
    fprintf(fp, "    \"type\": \"minor roads\",\n");
    fprintf(fp, "    \"points\": [");
    for (p = 0; p < no_of_minor_road_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }

        json_points(fp, &minor_road_points[p*2], pixels_tx, pixels_ty);
    }
    fprintf(fp, "],\n");
    fprintf(fp, "    \"links\": [");
    for (p = 0; p < no_of_minor_road_points; p++) {
        if (p > 0) {
            fprintf(fp, ",");
        }
        json_links(fp, &minor_road_links[p*2], pixels_tx, pixels_tx);
    }
    fprintf(fp, "]\n");
    fprintf(fp, "  },\n");

    fprintf(fp, "  {\n");
    fprintf(fp, "    \"type\": \"buildings\",\n");


    if (apesdk_format_used == 0)
    {
        polygons_map_json_building(fp, no_of_polygons, polygon_id, polygon_vertices, polygons, pixels_tx, pixels_ty);
    }
    else
    {
        polygons_map_json(fp, no_of_polygons, polygon_id, polygon_vertices, polygons, pixels_tx, pixels_ty);
    }
    fprintf(fp, "  }\n");
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);

    return 0;
}

static void process_value(json_value* value, int depth,
                          n_c_int * total_polygon_points,
                          n_c_int polygon_vertices[],
                          n_c_int polygon_id[],
                          n_c_int polygons[],
                          n_c_int * no_of_polygons,
                          n_c_int * total_wood_points,
                          n_c_int woods_vertices[],
                          n_c_int woods_id[],
                          n_c_int woods[],
                          n_c_int * no_of_woods,
                          n_c_int * total_sands_points,
                          n_c_int sands_vertices[],
                          n_c_int sands_id[],
                          n_c_int sands[],
                          n_c_int * no_of_sands,
                          n_c_int * total_sea_points,
                          n_c_int sea_vertices[],
                          n_c_int sea_id[],
                          n_c_int sea[],
                          n_c_int * no_of_sea,
                          n_c_int water_points[],
                          n_c_int water_point_width[],
                          n_c_int * no_of_water_points,
                          n_c_int water_links[],
                          n_c_int harbour_points[],
                          n_c_int * no_of_harbour_points,
                          n_c_int harbour_links[],
                          n_c_int railway_line_points[],
                          n_c_int * no_of_railway_line_points,
                          n_c_int railway_line_links[],
                          n_c_int railway_tunnel_points[],
                          n_c_int * no_of_railway_tunnel_points,
                          n_c_int railway_tunnel_links[],
                          n_c_int main_road_points[],
                          n_c_int * no_of_main_road_points,
                          n_c_int main_road_links[],
                          n_c_int minor_road_points[],
                          n_c_int * no_of_minor_road_points,
                          n_c_int minor_road_links[],
                          n_c_int junction_points[],
                          n_c_int * no_of_junctions,
                          n_c_int station_points[],
                          n_c_int * no_of_stations,
                          n_c_int orchards[],
                          n_c_int * no_of_orchards,
                          n_byte * bridge_coord_type,
                          n_c_int bridge_points[],
                          n_c_int * no_of_bridges,
                          n_byte * section,
                          n_byte * subsection,
                          n_byte * parameter,
                          n_c_int offset[],
                          n_c_int resolution[],
                          double geocoords[]);

static void process_object(json_value* value, int depth,
                           n_c_int * total_polygon_points,
                           n_c_int polygon_vertices[],
                           n_c_int polygon_id[],
                           n_c_int polygons[],
                           n_c_int * no_of_polygons,
                           n_c_int * total_wood_points,
                           n_c_int woods_vertices[],
                           n_c_int woods_id[],
                           n_c_int woods[],
                           n_c_int * no_of_woods,
                           n_c_int * total_sands_points,
                           n_c_int sands_vertices[],
                           n_c_int sands_id[],
                           n_c_int sands[],
                           n_c_int * no_of_sands,
                           n_c_int * total_sea_points,
                           n_c_int sea_vertices[],
                           n_c_int sea_id[],
                           n_c_int sea[],
                           n_c_int * no_of_sea,
                           n_c_int water_points[],
                           n_c_int water_point_width[],
                           n_c_int * no_of_water_points,
                           n_c_int water_links[],
                           n_c_int harbour_points[],
                           n_c_int * no_of_harbour_points,
                           n_c_int harbour_links[],
                           n_c_int railway_line_points[],
                           n_c_int * no_of_railway_line_points,
                           n_c_int railway_line_links[],
                           n_c_int railway_tunnel_points[],
                           n_c_int * no_of_railway_tunnel_points,
                           n_c_int railway_tunnel_links[],
                           n_c_int main_road_points[],
                           n_c_int * no_of_main_road_points,
                           n_c_int main_road_links[],
                           n_c_int minor_road_points[],
                           n_c_int * no_of_minor_road_points,
                           n_c_int minor_road_links[],
                           n_c_int junction_points[],
                           n_c_int * no_of_junctions,
                           n_c_int station_points[],
                           n_c_int * no_of_stations,
                           n_c_int orchards[],
                           n_c_int * no_of_orchards,
                           n_byte * bridge_coord_type,
                           n_c_int bridge_points[],
                           n_c_int * no_of_bridges,
                           n_byte * section,
                           n_byte * subsection,
                           n_byte * parameter,
                           n_c_int offset[],
                           n_c_int resolution[],
                           double geocoords[])
{
    int length, x;
    char * obj_name;

    if (value == NULL) {
        return;
    }

    length = value->u.object.length;
    for (x = 0; x < length; x++) {
        obj_name = value->u.object.values[x].name;
        /*
                printf("object[%d].name = %s\n", x, obj_name);
        */

        /* sections */
        if (strcmp(obj_name, "terrain") == 0) {
            *section = JSON_SECTION_TERRAIN;
        }
        if (strcmp(obj_name, "urban") == 0) {
            *section = JSON_SECTION_URBAN;
        }
        if (strcmp(obj_name, "offset") == 0) {
            *section = JSON_SECTION_OFFSET;
        }
        if (strcmp(obj_name, "resolution") == 0) {
            *section = JSON_SECTION_RESOLUTION;
        }
        if (strcmp(obj_name, "geocoords") == 0) {
            *section = JSON_SECTION_GEOCOORDS;
        }

        /* subsections */
        if (strcmp(obj_name, "topleft") == 0) {
            *subsection = JSON_SUBSECTION_TOP_LEFT;
        }
        if (strcmp(obj_name, "bottomright") == 0) {
            *subsection = JSON_SUBSECTION_BOTTOM_RIGHT;
        }

        /* parameters */
        if (strcmp(obj_name, "points") == 0) {
            *parameter = JSON_PARAM_POINTS;
            if (*subsection == JSON_SUBSECTION_RAILWAY_LINES) {
                *no_of_railway_line_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_HARBOURS) {
                *no_of_harbour_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_RAILWAY_TUNNELS) {
                *no_of_railway_tunnel_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_MAIN_ROADS) {
                *no_of_main_road_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_MINOR_ROADS) {
                *no_of_minor_road_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_RIVERS) {
                *no_of_water_points = 0;
            }
        }
        if (strcmp(obj_name, "widths") == 0) {
            *parameter = JSON_PARAM_WIDTHS;
            if (*subsection == JSON_SUBSECTION_RIVERS) {
                *no_of_water_points = 0;
            }
        }
        if (strcmp(obj_name, "links") == 0) {
            *parameter = JSON_PARAM_LINKS;
            if (*subsection == JSON_SUBSECTION_RAILWAY_LINES) {
                *no_of_railway_line_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_HARBOURS) {
                *no_of_harbour_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_RAILWAY_TUNNELS) {
                *no_of_railway_tunnel_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_MAIN_ROADS) {
                *no_of_main_road_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_MINOR_ROADS) {
                *no_of_minor_road_points = 0;
            }
            if (*subsection == JSON_SUBSECTION_RIVERS) {
                *no_of_water_points = 0;
            }
        }
        if (strcmp(obj_name, "perimeter") == 0) {
            *parameter = JSON_PARAM_PERIMETER;
        }
        if (strcmp(obj_name, "int") == 0) {
            *parameter = JSON_PARAM_INTERIOR;
        }
        if (strcmp(obj_name, "ext") == 0) {
            *parameter = JSON_PARAM_EXTERIOR;
        }
        if (strcmp(obj_name, "latitude") == 0) {
            *parameter = JSON_PARAM_LATITUDE;
        }
        if (strcmp(obj_name, "longitude") == 0) {
            *parameter = JSON_PARAM_LONGITUDE;
        }

        process_value(value->u.object.values[x].value, depth+1,
                      total_polygon_points,
                      polygon_vertices,
                      polygon_id,
                      polygons,
                      no_of_polygons,
                      total_wood_points,
                      woods_vertices,
                      woods_id,
                      woods,
                      no_of_woods,
                      total_sands_points,
                      sands_vertices,
                      sands_id,
                      sands,
                      no_of_sands,
                      total_sea_points,
                      sea_vertices,
                      sea_id,
                      sea,
                      no_of_sea,
                      water_points,
                      water_point_width,
                      no_of_water_points,
                      water_links,
                      harbour_points,
                      no_of_harbour_points,
                      harbour_links,
                      railway_line_points,
                      no_of_railway_line_points,
                      railway_line_links,
                      railway_tunnel_points,
                      no_of_railway_tunnel_points,
                      railway_tunnel_links,
                      main_road_points,
                      no_of_main_road_points,
                      main_road_links,
                      minor_road_points,
                      no_of_minor_road_points,
                      minor_road_links,
                      junction_points,
                      no_of_junctions,
                      station_points,
                      no_of_stations,
                      orchards,
                      no_of_orchards,
                      bridge_coord_type,
                      bridge_points,
                      no_of_bridges,
                      section, subsection, parameter,
                      offset, resolution, geocoords);
    }
}

static void process_array(json_value* value, int depth,
                          n_c_int * total_polygon_points,
                          n_c_int polygon_vertices[],
                          n_c_int polygon_id[],
                          n_c_int polygons[],
                          n_c_int * no_of_polygons,
                          n_c_int * total_wood_points,
                          n_c_int woods_vertices[],
                          n_c_int woods_id[],
                          n_c_int woods[],
                          n_c_int * no_of_woods,
                          n_c_int * total_sands_points,
                          n_c_int sands_vertices[],
                          n_c_int sands_id[],
                          n_c_int sands[],
                          n_c_int * no_of_sands,
                          n_c_int * total_sea_points,
                          n_c_int sea_vertices[],
                          n_c_int sea_id[],
                          n_c_int sea[],
                          n_c_int * no_of_sea,
                          n_c_int water_points[],
                          n_c_int water_point_width[],
                          n_c_int * no_of_water_points,
                          n_c_int water_links[],
                          n_c_int harbour_points[],
                          n_c_int * no_of_harbour_points,
                          n_c_int harbour_links[],
                          n_c_int railway_line_points[],
                          n_c_int * no_of_railway_line_points,
                          n_c_int railway_line_links[],
                          n_c_int railway_tunnel_points[],
                          n_c_int * no_of_railway_tunnel_points,
                          n_c_int railway_tunnel_links[],
                          n_c_int main_road_points[],
                          n_c_int * no_of_main_road_points,
                          n_c_int main_road_links[],
                          n_c_int minor_road_points[],
                          n_c_int * no_of_minor_road_points,
                          n_c_int minor_road_links[],
                          n_c_int junction_points[],
                          n_c_int * no_of_junctions,
                          n_c_int station_points[],
                          n_c_int * no_of_stations,
                          n_c_int orchards[],
                          n_c_int * no_of_orchards,
                          n_byte * bridge_coord_type,
                          n_c_int bridge_points[],
                          n_c_int * no_of_bridges,
                          n_byte * section,
                          n_byte * subsection,
                          n_byte * parameter,
                          n_c_int offset[],
                          n_c_int resolution[],
                          double geocoords[])
{
    n_c_int length, x, int_x, int_y;
    json_value* array_value;

    if (value == NULL) {
        return;
    }
    length = (n_c_int)value->u.array.length;

    if (length == 2) {
        array_value = value->u.array.values[0];
        if (array_value->type == json_integer) {
            int_x = (n_c_int)array_value->u.integer;
            array_value = value->u.array.values[1];
            int_y = (n_c_int)array_value->u.integer;

            if (*section == JSON_SECTION_OFFSET) {
                offset[0] = int_x;
                offset[1] = int_y;
            }
            if (*section == JSON_SECTION_RESOLUTION) {
                resolution[0] = int_x;
                resolution[1] = int_y;
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_BUILDINGS)) {
                if (*parameter == JSON_PARAM_EXTERIOR) {
                    polygons[(*total_polygon_points) * 2] = int_x - offset[0];
                    polygons[(*total_polygon_points) * 2 + 1] = int_y - offset[1];
                    if (*total_polygon_points < MAX_TOTAL_POLYGON_POINTS) {
                        *total_polygon_points = *total_polygon_points + 1;
                        polygon_vertices[*no_of_polygons]++;
                    }
                }
                if (*parameter == JSON_PARAM_INTERIOR) {
                    polygons[(*total_polygon_points) * 2] = int_x - offset[0];
                    polygons[(*total_polygon_points) * 2 + 1] = int_y - offset[1];
                    if (*total_polygon_points < MAX_TOTAL_POLYGON_POINTS) {
                        *total_polygon_points = *total_polygon_points + 1;
                        polygon_vertices[*no_of_polygons]++;
                    }
                }
            }

            if ((*section == JSON_SECTION_TERRAIN) &&
                    (*subsection == JSON_SUBSECTION_WOODLAND)) {
                if (*parameter == JSON_PARAM_PERIMETER) {
                    woods[(*total_wood_points) * 2] = int_x - offset[0];
                    woods[(*total_wood_points) * 2 + 1] = int_y - offset[1];
                    if (*total_wood_points < MAX_TOTAL_POLYGON_POINTS) {
                        *total_wood_points = *total_wood_points + 1;
                        woods_vertices[*no_of_woods]++;
                    }
                }
            }

            if ((*section == JSON_SECTION_TERRAIN) &&
                    (*subsection == JSON_SUBSECTION_SANDS)) {
                if (*parameter == JSON_PARAM_PERIMETER) {
                    sands[(*total_sands_points) * 2] = int_x - offset[0];
                    sands[(*total_sands_points) * 2 + 1] = int_y - offset[1];
                    if (*total_sands_points < MAX_TOTAL_POLYGON_POINTS) {
                        *total_sands_points = *total_sands_points + 1;
                        sands_vertices[*no_of_sands]++;
                    }
                }
            }

            if ((*section == JSON_SECTION_TERRAIN) &&
                    (*subsection == JSON_SUBSECTION_SEA)) {
                if (*parameter == JSON_PARAM_PERIMETER) {
                    sea[(*total_sea_points) * 2] = int_x - offset[0];
                    sea[(*total_sea_points) * 2 + 1] = int_y - offset[1];
                    if (*total_sea_points < MAX_TOTAL_POLYGON_POINTS) {
                        *total_sea_points = *total_sea_points + 1;
                        sea_vertices[*no_of_sea]++;
                    }
                }
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_JUNCTIONS)) {
                junction_points[(*no_of_junctions) * 2] = int_x - offset[0];
                junction_points[(*no_of_junctions) * 2 + 1] = int_y - offset[1];
                if (*no_of_junctions < MAX_JUNCTIONS) {
                    *no_of_junctions = *no_of_junctions + 1;
                }
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_RAILWAY_STATIONS)) {
                station_points[(*no_of_stations) * 2] = int_x - offset[0];
                station_points[(*no_of_stations) * 2 + 1] = int_y - offset[1];
                if (*no_of_stations < MAX_STATION_POINTS) {
                    *no_of_stations = *no_of_stations + 1;
                }
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_ORCHARDS)) {
                orchards[(*no_of_orchards) * 2] = int_x - offset[0];
                orchards[(*no_of_orchards) * 2 + 1] = int_y - offset[1];
                if (*no_of_orchards < MAX_STATION_POINTS) {
                    *no_of_orchards = *no_of_orchards + 1;
                }
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_RAILWAY_LINES)) {
                if (*parameter == JSON_PARAM_POINTS) {
                    railway_line_points[(*no_of_railway_line_points) * 2] = int_x - offset[0];
                    railway_line_points[(*no_of_railway_line_points) * 2 + 1] = int_y - offset[1];
                    if (*no_of_railway_line_points < MAX_ROAD_POINTS) {
                        *no_of_railway_line_points = *no_of_railway_line_points + 1;
                    }
                }
                if (*parameter == JSON_PARAM_LINKS) {
                    railway_line_links[(*no_of_railway_line_points) * 2] = int_x + 1;
                    railway_line_links[(*no_of_railway_line_points) * 2 + 1] = int_y + 1;
                    if (*no_of_railway_line_points < MAX_ROAD_POINTS) {
                        *no_of_railway_line_points = *no_of_railway_line_points + 1;
                    }
                }
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_HARBOURS)) {
                if (*parameter == JSON_PARAM_POINTS) {
                    railway_line_points[(*no_of_harbour_points) * 2] = int_x - offset[0];
                    railway_line_points[(*no_of_harbour_points) * 2 + 1] = int_y - offset[1];
                    if (*no_of_harbour_points < MAX_ROAD_POINTS) {
                        *no_of_harbour_points = *no_of_harbour_points + 1;
                    }
                }
                if (*parameter == JSON_PARAM_LINKS) {
                    harbour_links[(*no_of_harbour_points) * 2] = int_x + 1;
                    harbour_links[(*no_of_harbour_points) * 2 + 1] = int_y + 1;
                    if (*no_of_harbour_points < MAX_ROAD_POINTS) {
                        *no_of_harbour_points = *no_of_harbour_points + 1;
                    }
                }
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_RAILWAY_TUNNELS)) {
                if (*parameter == JSON_PARAM_POINTS) {
                    railway_tunnel_points[(*no_of_railway_tunnel_points) * 2] = int_x - offset[0];
                    railway_tunnel_points[(*no_of_railway_tunnel_points) * 2 + 1] = int_y - offset[1];
                    if (*no_of_railway_tunnel_points < MAX_ROAD_POINTS) {
                        *no_of_railway_tunnel_points = *no_of_railway_tunnel_points + 1;
                    }
                }
                if (*parameter == JSON_PARAM_LINKS) {
                    railway_tunnel_links[(*no_of_railway_tunnel_points) * 2] = int_x + 1;
                    railway_tunnel_links[(*no_of_railway_tunnel_points) * 2 + 1] = int_y + 1;
                    if (*no_of_railway_tunnel_points < MAX_ROAD_POINTS) {
                        *no_of_railway_tunnel_points = *no_of_railway_tunnel_points + 1;
                    }
                }
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_MAIN_ROADS)) {
                if (*parameter == JSON_PARAM_POINTS) {
                    main_road_points[(*no_of_main_road_points) * 2] = int_x - offset[0];
                    main_road_points[(*no_of_main_road_points) * 2 + 1] = int_y - offset[1];
                    if (*no_of_main_road_points < MAX_ROAD_POINTS) {
                        *no_of_main_road_points = *no_of_main_road_points + 1;
                    }
                }
                if (*parameter == JSON_PARAM_LINKS) {
                    main_road_links[(*no_of_main_road_points) * 2] = int_x + 1;
                    main_road_links[(*no_of_main_road_points) * 2 + 1] = int_y + 1;
                    if (*no_of_main_road_points < MAX_ROAD_POINTS) {
                        *no_of_main_road_points = *no_of_main_road_points + 1;
                    }
                }
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_MINOR_ROADS)) {
                if (*parameter == JSON_PARAM_POINTS) {
                    minor_road_points[(*no_of_minor_road_points) * 2] = int_x - offset[0];
                    minor_road_points[(*no_of_minor_road_points) * 2 + 1] = int_y - offset[1];
                    if (*no_of_minor_road_points < MAX_ROAD_POINTS) {
                        *no_of_minor_road_points = *no_of_minor_road_points + 1;
                    }
                }
                if (*parameter == JSON_PARAM_LINKS) {
                    minor_road_links[(*no_of_minor_road_points) * 2] = int_x + 1;
                    minor_road_links[(*no_of_minor_road_points) * 2 + 1] = int_y + 1;
                    if (*no_of_minor_road_points < MAX_ROAD_POINTS) {
                        *no_of_minor_road_points = *no_of_minor_road_points + 1;
                    }
                }
            }

            if ((*section == JSON_SECTION_TERRAIN) &&
                    (*subsection == JSON_SUBSECTION_RIVERS)) {
                if (*parameter == JSON_PARAM_POINTS) {
                    water_points[(*no_of_water_points) * 2] = int_x - offset[0];
                    water_points[(*no_of_water_points) * 2 + 1] = int_y - offset[1];
                    if (*no_of_water_points < MAX_ROAD_POINTS) {
                        *no_of_water_points = *no_of_water_points + 1;
                    }
                }
                if (*parameter == JSON_PARAM_LINKS) {
                    water_links[(*no_of_water_points) * 2] = int_x + 1;
                    water_links[(*no_of_water_points) * 2 + 1] = int_y + 1;
                    if (*no_of_water_points < MAX_ROAD_POINTS) {
                        *no_of_water_points = *no_of_water_points + 1;
                    }
                }
            }

            if ((*section == JSON_SECTION_URBAN) &&
                    (*subsection == JSON_SUBSECTION_BRIDGES)) {
                if (*bridge_coord_type == 0) {
                    bridge_points[(*no_of_bridges) * 4] = int_x - offset[0];
                    bridge_points[(*no_of_bridges) * 4 + 1] = int_y - offset[1];
                    *bridge_coord_type = 1;
                }
                else {
                    /* direction vector */
                    bridge_points[(*no_of_bridges) * 4 + 2] = int_x;
                    bridge_points[(*no_of_bridges) * 4 + 3] = int_y;
                    if (*no_of_bridges < MAX_BRIDGES) {
                        *no_of_bridges = *no_of_bridges + 1;
                    }
                    *bridge_coord_type = 0;
                }
            }
            return;
        }
    }

    if (*subsection == JSON_SUBSECTION_BUILDINGS) {
        if ((*parameter == JSON_PARAM_EXTERIOR) ||
                (*parameter == JSON_PARAM_INTERIOR)) {
            polygon_vertices[*no_of_polygons] = 0;
        }
    }
    if (*subsection == JSON_SUBSECTION_WOODLAND) {
        if (*parameter == JSON_PARAM_PERIMETER) {
            woods_vertices[*no_of_woods] = 0;
        }
    }
    if (*subsection == JSON_SUBSECTION_SANDS) {
        if (*parameter == JSON_PARAM_PERIMETER) {
            sands_vertices[*no_of_sands] = 0;
        }
    }
    if (*subsection == JSON_SUBSECTION_SEA) {
        if (*parameter == JSON_PARAM_PERIMETER) {
            sea_vertices[*no_of_sea] = 0;
        }
    }

    for (x = 0; x < length; x++) {
        array_value = value->u.array.values[x];
        if (array_value == NULL) continue;
        process_value(array_value, depth,
                      total_polygon_points,
                      polygon_vertices,
                      polygon_id,
                      polygons,
                      no_of_polygons,
                      total_wood_points,
                      woods_vertices,
                      woods_id,
                      woods,
                      no_of_woods,
                      total_sands_points,
                      sands_vertices,
                      sands_id,
                      sands,
                      no_of_sands,
                      total_sea_points,
                      sea_vertices,
                      sea_id,
                      sea,
                      no_of_sea,
                      water_points,
                      water_point_width,
                      no_of_water_points,
                      water_links,
                      harbour_points,
                      no_of_harbour_points,
                      harbour_links,
                      railway_line_points,
                      no_of_railway_line_points,
                      railway_line_links,
                      railway_tunnel_points,
                      no_of_railway_tunnel_points,
                      railway_tunnel_links,
                      main_road_points,
                      no_of_main_road_points,
                      main_road_links,
                      minor_road_points,
                      no_of_minor_road_points,
                      minor_road_links,
                      junction_points,
                      no_of_junctions,
                      station_points,
                      no_of_stations,
                      orchards,
                      no_of_orchards,
                      bridge_coord_type,
                      bridge_points,
                      no_of_bridges,
                      section, subsection, parameter,
                      offset, resolution, geocoords);
    }

    if (*subsection == JSON_SUBSECTION_BUILDINGS) {
        if (*parameter == JSON_PARAM_EXTERIOR) {
            polygon_id[*no_of_polygons] = *no_of_polygons;
        }
        if (*parameter == JSON_PARAM_INTERIOR) {
            polygon_id[*no_of_polygons] = *no_of_polygons - 1;
        }
        if ((*parameter == JSON_PARAM_EXTERIOR) ||
                (*parameter == JSON_PARAM_INTERIOR)) {
            *no_of_polygons = *no_of_polygons + 1;
        }
    }

    if (*subsection == JSON_SUBSECTION_WOODLAND) {
        if (*parameter == JSON_PARAM_PERIMETER) {
            woods_id[*no_of_woods] = *no_of_woods;
            *no_of_woods = *no_of_woods + 1;
        }
    }

    if (*subsection == JSON_SUBSECTION_SANDS) {
        if (*parameter == JSON_PARAM_PERIMETER) {
            sands_id[*no_of_sands] = *no_of_sands;
            *no_of_sands = *no_of_sands + 1;
        }
    }
    if (*subsection == JSON_SUBSECTION_SEA) {
        if (*parameter == JSON_PARAM_PERIMETER) {
            sea_id[*no_of_sea] = *no_of_sea;
            *no_of_sea = *no_of_sea + 1;
        }
    }
}

static void process_value(json_value* value, int depth,
                          n_c_int * total_polygon_points,
                          n_c_int polygon_vertices[],
                          n_c_int polygon_id[],
                          n_c_int polygons[],
                          n_c_int * no_of_polygons,
                          n_c_int * total_wood_points,
                          n_c_int woods_vertices[],
                          n_c_int woods_id[],
                          n_c_int woods[],
                          n_c_int * no_of_woods,
                          n_c_int * total_sands_points,
                          n_c_int sands_vertices[],
                          n_c_int sands_id[],
                          n_c_int sands[],
                          n_c_int * no_of_sands,
                          n_c_int * total_sea_points,
                          n_c_int sea_vertices[],
                          n_c_int sea_id[],
                          n_c_int sea[],
                          n_c_int * no_of_sea,
                          n_c_int water_points[],
                          n_c_int water_point_width[],
                          n_c_int * no_of_water_points,
                          n_c_int water_links[],
                          n_c_int harbour_points[],
                          n_c_int * no_of_harbour_points,
                          n_c_int harbour_links[],
                          n_c_int railway_line_points[],
                          n_c_int * no_of_railway_line_points,
                          n_c_int railway_line_links[],
                          n_c_int railway_tunnel_points[],
                          n_c_int * no_of_railway_tunnel_points,
                          n_c_int railway_tunnel_links[],
                          n_c_int main_road_points[],
                          n_c_int * no_of_main_road_points,
                          n_c_int main_road_links[],
                          n_c_int minor_road_points[],
                          n_c_int * no_of_minor_road_points,
                          n_c_int minor_road_links[],
                          n_c_int junction_points[],
                          n_c_int * no_of_junctions,
                          n_c_int station_points[],
                          n_c_int * no_of_stations,
                          n_c_int orchards[],
                          n_c_int * no_of_orchards,
                          n_byte * bridge_coord_type,
                          n_c_int bridge_points[],
                          n_c_int * no_of_bridges,
                          n_byte * section,
                          n_byte * subsection,
                          n_byte * parameter,
                          n_c_int offset[],
                          n_c_int resolution[],
                          double geocoords[])
{
    if (value == NULL) {
        return;
    }
    switch (value->type) {
    case json_none:
        break;
    case json_null:
        break;
    case json_object:
        process_object(value, depth+1,
                       total_polygon_points,
                       polygon_vertices,
                       polygon_id,
                       polygons,
                       no_of_polygons,
                       total_wood_points,
                       woods_vertices,
                       woods_id,
                       woods,
                       no_of_woods,
                       total_sands_points,
                       sands_vertices,
                       sands_id,
                       sands,
                       no_of_sands,
                       total_sea_points,
                       sea_vertices,
                       sea_id,
                       sea,
                       no_of_sea,
                       water_points,
                       water_point_width,
                       no_of_water_points,
                       water_links,
                       harbour_points,
                       no_of_harbour_points,
                       harbour_links,
                       railway_line_points,
                       no_of_railway_line_points,
                       railway_line_links,
                       railway_tunnel_points,
                       no_of_railway_tunnel_points,
                       railway_tunnel_links,
                       main_road_points,
                       no_of_main_road_points,
                       main_road_links,
                       minor_road_points,
                       no_of_minor_road_points,
                       minor_road_links,
                       junction_points,
                       no_of_junctions,
                       station_points,
                       no_of_stations,
                       orchards,
                       no_of_orchards,
                       bridge_coord_type,
                       bridge_points,
                       no_of_bridges,
                       section, subsection, parameter,
                       offset, resolution, geocoords);
        break;
    case json_array:
        process_array(value, depth+1,
                      total_polygon_points,
                      polygon_vertices,
                      polygon_id,
                      polygons,
                      no_of_polygons,
                      total_wood_points,
                      woods_vertices,
                      woods_id,
                      woods,
                      no_of_woods,
                      total_sands_points,
                      sands_vertices,
                      sands_id,
                      sands,
                      no_of_sands,
                      total_sea_points,
                      sea_vertices,
                      sea_id,
                      sea,
                      no_of_sea,
                      water_points,
                      water_point_width,
                      no_of_water_points,
                      water_links,
                      harbour_points,
                      no_of_harbour_points,
                      harbour_links,
                      railway_line_points,
                      no_of_railway_line_points,
                      railway_line_links,
                      railway_tunnel_points,
                      no_of_railway_tunnel_points,
                      railway_tunnel_links,
                      main_road_points,
                      no_of_main_road_points,
                      main_road_links,
                      minor_road_points,
                      no_of_minor_road_points,
                      minor_road_links,
                      junction_points,
                      no_of_junctions,
                      station_points,
                      no_of_stations,
                      orchards,
                      no_of_orchards,
                      bridge_coord_type,
                      bridge_points,
                      no_of_bridges,
                      section, subsection, parameter,
                      offset, resolution, geocoords);
        break;
    case json_integer:
        if ((*section == JSON_SECTION_TERRAIN) &&
                (*subsection == JSON_SUBSECTION_RIVERS)) {
            if (*parameter == JSON_PARAM_WIDTHS) {
                water_point_width[*no_of_water_points] = value->u.integer;
                if (*no_of_water_points < MAX_ROAD_POINTS) {
                    *no_of_water_points = *no_of_water_points + 1;
                }
            }
        }
        break;
    case json_double:
        if (*section == JSON_SECTION_GEOCOORDS) {
            if (*subsection == JSON_SUBSECTION_TOP_LEFT) {
                if (*parameter == JSON_PARAM_LATITUDE) {
                    geocoords[0] = value->u.dbl;
                }
                if (*parameter == JSON_PARAM_LONGITUDE) {
                    geocoords[1] = value->u.dbl;
                }
            }
            if (*subsection == JSON_SUBSECTION_BOTTOM_RIGHT) {
                if (*parameter == JSON_PARAM_LATITUDE) {
                    geocoords[2] = value->u.dbl;
                }
                if (*parameter == JSON_PARAM_LONGITUDE) {
                    geocoords[3] = value->u.dbl;
                }
            }
        }
        break;
    case json_string:
        /* subsections */
        if (strcmp(value->u.string.ptr, "woodland") == 0) {
            *subsection = JSON_SUBSECTION_WOODLAND;
        }
        if (strcmp(value->u.string.ptr, "rivers") == 0) {
            *subsection = JSON_SUBSECTION_RIVERS;
        }
        if (strcmp(value->u.string.ptr, "lakes") == 0) {
            *subsection = JSON_SUBSECTION_LAKES;
        }
        if (strcmp(value->u.string.ptr, "sands") == 0) {
            *subsection = JSON_SUBSECTION_SANDS;
        }
        if (strcmp(value->u.string.ptr, "sea") == 0) {
            *subsection = JSON_SUBSECTION_SEA;
        }
        if (strcmp(value->u.string.ptr, "railway stations") == 0) {
            *subsection = JSON_SUBSECTION_RAILWAY_STATIONS;
        }
        if (strcmp(value->u.string.ptr, "orchards") == 0) {
            *subsection = JSON_SUBSECTION_ORCHARDS;
        }
        if (strcmp(value->u.string.ptr, "bridges") == 0) {
            *subsection = JSON_SUBSECTION_BRIDGES;
        }
        if (strcmp(value->u.string.ptr, "junctions") == 0) {
            *subsection = JSON_SUBSECTION_JUNCTIONS;
        }
        if (strcmp(value->u.string.ptr, "harbours") == 0) {
            *subsection = JSON_SUBSECTION_HARBOURS;
        }
        if (strcmp(value->u.string.ptr, "railway lines") == 0) {
            *subsection = JSON_SUBSECTION_RAILWAY_LINES;
        }
        if (strcmp(value->u.string.ptr, "railway tunnels") == 0) {
            *subsection = JSON_SUBSECTION_RAILWAY_TUNNELS;
        }
        if (strcmp(value->u.string.ptr, "main roads") == 0) {
            *subsection = JSON_SUBSECTION_MAIN_ROADS;
        }
        if (strcmp(value->u.string.ptr, "minor roads") == 0) {
            *subsection = JSON_SUBSECTION_MINOR_ROADS;
        }
        if (strcmp(value->u.string.ptr, "buildings") == 0) {
            *subsection = JSON_SUBSECTION_BUILDINGS;
        }
        break;
    case json_boolean:
        break;
    }
}

n_c_int load_map_json(char * filename,
                      n_c_int polygon_vertices[],
                      n_c_int polygon_id[],
                      n_c_int polygons[],
                      n_c_int * no_of_polygons,
                      n_c_int woods_vertices[],
                      n_c_int woods_id[],
                      n_c_int woods[],
                      n_c_int * no_of_woods,
                      n_c_int sands_vertices[],
                      n_c_int sands_id[],
                      n_c_int sands[],
                      n_c_int * no_of_sands,
                      n_c_int sea_vertices[],
                      n_c_int sea_id[],
                      n_c_int sea[],
                      n_c_int * no_of_sea,
                      n_c_int water_points[],
                      n_c_int water_point_width[],
                      n_c_int * no_of_water_points,
                      n_c_int water_links[],
                      n_c_int harbour_points[],
                      n_c_int * no_of_harbour_points,
                      n_c_int harbour_links[],
                      n_c_int railway_line_points[],
                      n_c_int * no_of_railway_line_points,
                      n_c_int railway_line_links[],
                      n_c_int railway_tunnel_points[],
                      n_c_int * no_of_railway_tunnel_points,
                      n_c_int railway_tunnel_links[],
                      n_c_int main_road_points[],
                      n_c_int * no_of_main_road_points,
                      n_c_int main_road_links[],
                      n_c_int minor_road_points[],
                      n_c_int * no_of_minor_road_points,
                      n_c_int minor_road_links[],
                      n_c_int junction_points[],
                      n_c_int * no_of_junctions,
                      n_c_int station_points[],
                      n_c_int * no_of_stations,
                      n_c_int orchards[],
                      n_c_int * no_of_orchards,
                      n_c_int bridge_points[],
                      n_c_int * no_of_bridges,
                      n_c_int offset[],
                      n_c_int resolution[],
                      double geocoords[])
{
    FILE *fp;
    struct stat filestatus;
    int file_size;
    char* file_contents;
    json_char* json;
    json_value* value;
    n_c_int total_polygon_points = 0;
    n_c_int total_wood_points = 0;
    n_c_int total_sands_points = 0;
    n_c_int total_sea_points = 0;
    n_byte section = 0, subsection = 0, parameter = 0;
    n_byte bridge_coord_type = 0;

    memset(geocoords, 0, 4*sizeof(double));
    memset(offset, 0, 2*sizeof(n_c_int));
    memset(resolution, 0, 2*sizeof(n_c_int));
    *no_of_polygons = 0;
    *no_of_woods = 0;
    *no_of_sands = 0;
    *no_of_sea = 0;
    *no_of_water_points = 0;
    *no_of_main_road_points = 0;
    *no_of_minor_road_points = 0;
    *no_of_junctions = 0;
    *no_of_stations = 0;
    *no_of_orchards = 0;
    *no_of_bridges = 0;
    *no_of_railway_line_points = 0;
    *no_of_railway_tunnel_points = 0;
    *no_of_harbour_points = 0;

    if ( stat(filename, &filestatus) != 0) {
        fprintf(stderr, "File %s not found\n", filename);
        return 1;
    }
    file_size = filestatus.st_size;
    file_contents = (char*)malloc(filestatus.st_size);
    if ( file_contents == NULL) {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        return 1;
    }

    fp = fopen(filename, "rt");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    if ( fread(file_contents, file_size, 1, fp) != 1 ) {
        fprintf(stderr, "Unable to read content of %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    fclose(fp);

    printf("json file loaded\n");

    json = (json_char*)file_contents;

    value = json_parse(json, file_size);

    if (value == NULL) {
        fprintf(stderr, "Unable to parse data\n");
        free(file_contents);
        exit(1);
    }

    printf("json file parsed\n");

    process_value(value, 0,
                  &total_polygon_points,
                  polygon_vertices,
                  polygon_id,
                  polygons,
                  no_of_polygons,
                  &total_wood_points,
                  woods_vertices,
                  woods_id,
                  woods,
                  no_of_woods,
                  &total_sands_points,
                  sands_vertices,
                  sands_id,
                  sands,
                  no_of_sands,
                  &total_sea_points,
                  sea_vertices,
                  sea_id,
                  sea,
                  no_of_sea,
                  water_points,
                  water_point_width,
                  no_of_water_points,
                  water_links,
                  harbour_points,
                  no_of_harbour_points,
                  harbour_links,
                  railway_line_points,
                  no_of_railway_line_points,
                  railway_line_links,
                  railway_tunnel_points,
                  no_of_railway_tunnel_points,
                  railway_tunnel_links,
                  main_road_points,
                  no_of_main_road_points,
                  main_road_links,
                  minor_road_points,
                  no_of_minor_road_points,
                  minor_road_links,
                  junction_points,
                  no_of_junctions,
                  station_points,
                  no_of_stations,
                  orchards,
                  no_of_orchards,
                  &bridge_coord_type,
                  bridge_points,
                  no_of_bridges,
                  &section, &subsection, &parameter,
                  offset, resolution, geocoords);
    printf("Offset: %d, %d\n", offset[0], offset[1]);
    printf("Resolution: %d, %d\n", resolution[0], resolution[1]);
    printf("Geocoords top left: lat %lf, long %lf\n", geocoords[0], geocoords[1]);
    printf("Geocoords bottom right: lat %lf, long %lf\n", geocoords[2], geocoords[3]);
    printf("%d total polygon points\n", total_polygon_points);
    printf("%d total wood points\n", total_wood_points);
    printf("%d total sands points\n", total_sands_points);
    printf("%d total sea points\n", total_sea_points);
    printf("%d total harbour points\n", *no_of_harbour_points);
    printf("%d stations loaded\n", *no_of_stations);
    printf("%d orchards loaded\n", *no_of_orchards);
    printf("%d junction points loaded\n", *no_of_junctions);
    printf("%d main road points loaded\n", *no_of_main_road_points);
    printf("%d minor road points loaded\n", *no_of_minor_road_points);
    printf("%d water points loaded\n", *no_of_water_points);
    printf("%d polygons loaded\n", *no_of_polygons);
    printf("%d woods loaded\n", *no_of_woods);
    printf("%d sea areas loaded\n", *no_of_sea);
    printf("%d bridges loaded\n", *no_of_bridges);
    printf("%d railway line points loaded\n", *no_of_railway_line_points);
    printf("%d railway tunnel points loaded\n", *no_of_railway_tunnel_points);

    json_value_free(value);
    free(file_contents);
    return 0;
}
