#ifndef MAP2JSON_HEADERS_H
#define MAP2JSON_HEADERS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include "toolkit.h"
#include "lodepng.h"
#include "json-parser.h"

/* image dimensions in pixels */
#define MAX_TILE_WIDTH                 2000
#define MAX_TILE_HEIGHT                1300
#define MIN_SOURCE_MAP_IMAGE_WIDTH     10000
#define MIN_SOURCE_MAP_IMAGE_HEIGHT    10000

#define MAX_TOTAL_POLYGON_POINTS       (4096*640*4)
#define MAX_ROAD_POINTS                (4096*640)
#define MAX_JUNCTIONS                  (20480*4)
#define MAX_STATION_POINTS             1280
#define MAX_ORCHARD_POINTS             8000
#define MAX_BRIDGES                    2560

#define NEXT_JUNCTION 0
#define PREV_JUNCTION 1

#define MAX_POLYGON_LINE_SEGMENTS      128

/* Max number of points for an individual polygon, prior to reduction */
#define MAX_POLYGON_POINTS             (4096*8)

/* Background value for thresholded images */
#define BACKGROUND                     255

/* You may need to adjust this to avoid running out of stack space.
Stack size can be changed in /etc/security/limits.conf if needed  */
#define MAX_RECURSION_DEPTH            50000

/* This should not be a huge number. A few samples is enough to check
   that a line between points should probably exist */
#define LINE_EVIDENCE_SAMPLES          16

typedef struct {
    n_c_int * vertices;
    n_c_int * id;
    n_c_int * coords;
    n_c_int count;
} n_map_polygons;

typedef struct {
    n_c_int * points;
    n_c_int * width;
    n_c_int * links;
    n_c_int count;
} n_map_lines;

typedef struct {
    n_c_int offset[2];
    n_c_int resolution[2];
    double geocoords[4];

    n_map_polygons buildings;
    n_map_polygons woods;
    n_map_polygons sea;
    n_map_polygons sands;
    n_map_polygons orchards;

    n_map_lines water;
    n_map_lines main_road;
    n_map_lines minor_road;
    n_map_lines railway_line;
    n_map_lines railway_tunnel;
    n_map_lines harbour;

    n_c_int * junction_points;
    n_c_int no_of_junctions;

    n_c_int * station_points;
    n_c_int no_of_stations;

    n_c_int * orchard_points;
    n_c_int no_of_orchards;

    n_c_int * bridge_points;
    n_c_int no_of_bridges;
} n_map_data;

/* lodepng.c */

unsigned char * read_png_file(char * filename,
                              n_byte4 * width,
                              n_byte4 * height,
                              n_byte4 * bitsperpixel);

n_c_int write_png_file(char* filename,
                       n_byte4 width, n_byte4 height,
                       n_byte4 bitsperpixel,
                       unsigned char *buffer);

/* threshold.c */

n_c_int remove_high_frequency(n_byte img[], n_c_int width, n_c_int height,
                              n_c_int min_width);
n_c_int append_polygons(n_map_polygons * dest, n_map_polygons * src);
n_c_int subtract_image(n_byte img[], n_c_int width, n_c_int height,
                       n_byte img2[], n_byte img3[], n_c_int search_radius,
                       n_c_int background_threshold);
n_c_int proximal_threshold(n_byte img[], n_c_int width, n_c_int height,
                           n_c_int r, n_c_int g, n_c_int b, n_c_int radius,
                           n_byte result[]);
n_c_int proximal_erase(n_byte img[], n_c_int width, n_c_int height,
                       n_c_int radius, n_c_int min_coverage_percent);
n_c_int proximal_fill(n_byte img[], n_c_int width, n_c_int height,
                      n_byte result[],
                      n_c_int min_width, n_c_int min_height,
                      n_c_int max_width, n_c_int max_height,
                      n_c_int max_variance,
                      n_c_int polygon_id[],
                      n_c_int polygon_vertices[],
                      n_c_int polygons[],
                      n_c_int max_total_polygon_points,
                      n_c_int detect_interior);
n_c_int show_polygons(n_byte img[], n_c_int width, n_c_int height,
                      n_c_int no_of_polygons,
                      n_c_int polygon_id[],
                      n_c_int polygon_vertices[],
                      n_c_int polygons[]);
n_c_int point_in_polygon(n_c_int x, n_c_int y, n_c_int * points, n_c_int no_of_points);
n_c_int remove_nogo_areas(n_byte img[], n_c_int width, n_c_int height,
                          n_c_int no_of_nogo_areas,
                          n_c_int vertices[], n_c_int nogo_areas[]);
n_c_int percent_coverage(n_byte img[], n_c_int width, n_c_int height);
n_c_int get_polygons_total_area(n_c_int width, n_c_int height,
                                n_c_int no_of_polygons,
                                n_c_int vertices[], n_c_int polygons[]);
n_c_int show_polygons_filled(n_byte img[], n_c_int width, n_c_int height,
                             n_c_int no_of_polygons,
                             n_c_int polygon_vertices[],
                             n_c_int polygons[],
                             n_c_int r, n_c_int g, n_c_int b);
n_c_int show_polygons_empty(n_byte img[], n_c_int width, n_c_int height,
                            n_c_int no_of_polygons,
                            n_c_int polygon_vertices[],
                            n_c_int polygon_id[],
                            n_c_int polygons[],
                            n_byte r, n_byte g, n_byte b);
n_c_int show_polygons_against_reference(n_byte img[], n_c_int width, n_c_int height,
                                        n_byte thresholded_ref[],
                                        n_c_int no_of_polygons,
                                        n_c_int polygon_id[],
                                        n_c_int polygon_vertices[],
                                        n_c_int polygons[],
                                        n_byte inner);

/* json.c */

n_c_int save_railway_json(char * filename,
                          n_c_int no_of_railway_line_points,
                          n_c_int railway_line_points[],
                          n_c_int railway_line_links[],
                          n_c_int no_of_railway_tunnel_points,
                          n_c_int railway_tunnel_points[],
                          n_c_int railway_tunnel_links[],
                          n_c_int no_of_harbour_points,
                          n_c_int harbour_points[],
                          n_c_int harbour_links[]);
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
                      double geocoords[]);

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
                      char * filename);

/* convert.c */

void mono_to_color(n_byte * img, n_c_int width, n_c_int height,
                   n_c_int bitsperpixel,
                   n_byte * color);

void color_to_mono(n_byte * img, n_c_int width, n_c_int height,
                   n_c_int bitsperpixel,
                   n_byte * mono);

void color_to_binary(n_byte * img, n_c_int width, n_c_int height,
                     n_c_int bitsperpixel, n_c_int threshold,
                     n_byte * mono);

/* color.c */

void color_pixel_to_binary(n_byte img[], n_c_int width, n_c_int height,
                           n_byte r, n_byte g, n_byte b,
                           n_byte thresholded[], n_byte clear);
n_int color_pixel_count(n_byte img[], n_c_int width, n_c_int height,
                        n_byte r, n_byte g, n_byte b);
void detect_color(n_byte img[],
                  n_c_int width, n_c_int height, n_c_int bitsperpixel,
                  n_c_int primary_channel,
                  n_c_int red_low, n_c_int red_high,
                  n_c_int green_low, n_c_int green_high,
                  n_c_int blue_low, n_c_int blue_high,
                  n_c_int averaging_radius);
void detect_green(n_byte img[],
                  n_c_int width, n_c_int height, n_c_int bitsperpixel,
                  n_c_int threshold,
                  n_c_int averaging_radius);
void detect_orange(n_byte img[],
                   n_c_int width, n_c_int height, n_c_int bitsperpixel,
                   n_c_int threshold_red,
                   n_c_int threshold_green,
                   n_c_int averaging_radius,
                   n_c_int min_red, n_c_int max_red);
void detect_red(n_byte img[],
                n_c_int width, n_c_int height, n_c_int bitsperpixel,
                n_c_int threshold_red,
                n_c_int threshold_green,
                n_c_int averaging_radius,
                n_c_int min_red, n_c_int max_red,
                n_c_int max_green);
void detect_blue(n_byte img[],
                 n_c_int width, n_c_int height, n_c_int bitsperpixel,
                 n_c_int threshold_red,
                 n_c_int threshold_green,
                 n_c_int averaging_radius,
                 n_c_int min_blue, n_c_int max_blue);
void detect_blue_green(n_byte img[],
                       n_c_int width, n_c_int height, n_c_int bitsperpixel,
                       n_c_int threshold_red,
                       n_c_int threshold_blue_green_diff,
                       n_c_int averaging_radius,
                       n_c_int min_blue, n_c_int max_blue,
                       n_byte result[]);

/* random.c */

n_byte4 rand_num(n_byte4 * seed);

/* draw.c */
void draw_line(n_byte img[],
               n_byte4 width, n_byte4 height,
               n_c_int bitsperpixel,
               n_c_int tx, n_c_int ty, n_c_int bx, n_c_int by,
               n_c_int line_width,
               n_c_int r, n_c_int g, n_c_int b);

void draw_point(n_byte img[],
                n_byte4 width, n_byte4 height,
                n_c_int x, n_c_int y,
                n_c_int point_radius,
                n_c_int r, n_c_int g, n_c_int b);

/* skeleton.c */
void skeletonize(n_byte img[], n_c_int width, n_c_int height,
                 n_byte result[], n_c_int bitsperpixel,
                 n_c_int max_line_width);
void skeletonize_simple(n_byte img[], n_c_int width, n_c_int height,
                        n_byte result[], n_c_int bitsperpixel,
                        n_c_int max_line_width);
n_c_int skeleton_to_points(n_byte img[], n_c_int width, n_c_int height,
                           n_c_int bitsperpixel, n_c_int point_spacing,
                           n_c_int line_points[], n_c_int max_points,
                           n_c_int sample_radius);

/* line.c */
n_c_int show_line_points(n_byte * result, n_c_int width, n_c_int height,
                         n_c_int line_points[], n_c_int no_of_line_points,
                         n_c_int radius);

n_c_int link_line_points(n_c_int line_points[], n_c_int no_of_line_points,
                         n_c_int max_distance, n_c_int line_links[],
                         n_byte thresholded_ref[],
                         n_c_int image_width, n_c_int image_height,
                         n_c_int min_link_length);

n_c_int show_lines(n_byte result[], n_c_int width, n_c_int height, n_c_int bitsperpixel,
                   n_c_int line_points[], n_c_int no_of_line_points,
                   n_c_int line_links[], n_c_int line_width,
                   n_c_int r, n_c_int g, n_c_int b, n_byte show_junctions,
                   n_byte clear);
n_c_int show_lines_with_width(n_byte result[], n_c_int width, n_c_int height,
                              n_c_int bitsperpixel,
                              n_c_int line_points[], n_c_int no_of_line_points,
                              n_c_int line_links[], n_c_int line_widths[],
                              n_c_int r, n_c_int g, n_c_int b);
n_c_int remove_close_points(n_c_int line_points[],
                            n_c_int no_of_line_points,
                            n_c_int radius);
n_c_int thin_line_point_clumps(n_c_int line_points[],
                               n_c_int no_of_line_points,
                               n_c_int radius,
                               n_c_int max_close_points);
n_c_int show_six_lines(n_byte result[], n_c_int width, n_c_int height,
                       n_c_int bitsperpixel,
                       n_c_int line_points1[], n_c_int line_widths1[],
                       n_c_int no_of_line_points1, n_c_int line_links1[],
                       n_c_int r1, n_c_int g1, n_c_int b1,
                       n_c_int line_points2[], n_c_int no_of_line_points2,
                       n_c_int line_links2[], n_c_int line_width2,
                       n_c_int r2, n_c_int g2, n_c_int b2,
                       n_c_int line_points3[], n_c_int no_of_line_points3,
                       n_c_int line_links3[], n_c_int line_width3,
                       n_c_int r3, n_c_int g3, n_c_int b3,
                       n_c_int line_points4[], n_c_int no_of_line_points4,
                       n_c_int line_links4[], n_c_int line_width4,
                       n_c_int r4, n_c_int g4, n_c_int b4,
                       n_c_int line_points5[], n_c_int no_of_line_points5,
                       n_c_int line_links5[], n_c_int line_width5,
                       n_c_int r5, n_c_int g5, n_c_int b5,
                       n_c_int line_points6[], n_c_int no_of_line_points6,
                       n_c_int line_links6[], n_c_int line_width6,
                       n_c_int r6, n_c_int g6, n_c_int b6,
                       n_c_int junction_points[], n_c_int no_of_junctions,
                       n_byte background_r, n_byte background_g, n_byte background_b);
n_c_int detect_junctions(n_c_int line_points1[], n_c_int no_of_line_points1,
                         n_c_int line_links1[],
                         n_c_int line_points2[], n_c_int no_of_line_points2,
                         n_c_int line_links2[],
                         n_c_int junction_points[], n_c_int max_junctions,
                         n_c_int join_ends_radius);
n_c_int line_join_ends(n_c_int line_points[], n_c_int no_of_line_points,
                       n_c_int line_links[], n_c_int join_ends_radius);
n_c_int detect_crossings(n_c_int line_points1[], n_c_int no_of_line_points1,
                         n_c_int line_links1[],
                         n_c_int line_points2[], n_c_int no_of_line_points2,
                         n_c_int line_links2[],
                         n_c_int line_points3[], n_c_int no_of_line_points3,
                         n_c_int line_links3[],
                         n_c_int crossing_points[], n_c_int max_crossings);
n_c_int show_crossings(n_byte result[], n_c_int width, n_c_int height,
                       n_c_int bitsperpixel,
                       n_c_int crossing_points[], n_c_int no_of_crossings,
                       n_c_int r1, n_c_int g1, n_c_int b1,
                       n_c_int r2, n_c_int g2, n_c_int b2,
                       n_c_int crossing_size, n_c_int line_width);
n_c_int line_intersection(n_c_int line1_tx, n_c_int line1_ty,
                          n_c_int line1_bx, n_c_int line1_by,
                          n_c_int line2_tx, n_c_int line2_ty,
                          n_c_int line2_bx, n_c_int line2_by,
                          n_c_int * ix, n_c_int * iy);
n_c_int point_dist_from_line(n_c_int x0, n_c_int y0,
                             n_c_int x1, n_c_int y1,
                             n_c_int point_x, n_c_int point_y);
n_c_int potential_roads(n_byte img[], n_c_int width, n_c_int height,
                        n_byte result[],
                        n_c_int min_width, n_c_int max_width);
n_c_int detect_line_point_widths(n_byte img[], n_c_int width, n_c_int height,
                                 n_c_int points[], n_c_int no_of_points,
                                 n_c_int point_width[], n_c_int max_width);
n_c_int connect_line_sections(n_c_int line_points[], n_c_int no_of_line_points,
                              n_c_int max_distance, n_c_int line_links[]);

/* tile.c */

n_c_int create_map_data(n_c_int max_total_polygon_points,
                        n_c_int max_road_points,
                        n_c_int max_junctions,
                        n_c_int max_station_points,
                        n_c_int max_orchard_points,
                        n_c_int max_bridges,
                        n_map_data * mapdata);
n_c_int free_map_data(n_map_data * mapdata);

n_c_int breakup_large_map(char * large_filename,
                          n_byte large_map[], n_c_int large_width, n_c_int large_height,
                          n_c_int image_bitsperpixel,
                          char * tiles_directory,
                          n_c_int overlap_percent);

/* blob.c */

n_c_int detect_blobs(n_byte img[], n_c_int width, n_c_int height,
                     n_c_int search_radius,
                     n_c_int max_blob_points,
                     n_c_int blob_points[],
                     n_c_int min_blob_size,
                     n_c_int max_blob_size,
                     n_byte result[]);
n_c_int remove_blobs_from_points(n_c_int blob_points[], n_c_int no_of_blobs,
                                 n_c_int line_points[], n_c_int no_of_line_points,
                                 n_c_int search_radius);

/* dots.c */

n_c_int detect_dots(n_map_polygons * polygons, n_c_int spacing,
                    n_c_int * dot_coords,
                    n_byte result[], n_c_int width, n_c_int height);
n_c_int polygons_to_points(n_map_polygons * polygons,
                           n_c_int points[], n_c_int max_points);
void show_dots(n_byte image_data[], n_byte4 image_width, n_byte4 image_height,
               n_c_int no_of_dots, n_c_int dots[],
               n_byte r, n_byte g, n_byte b, n_c_int radius);

/* texture.c */

n_int detect_texture(n_byte img[], n_c_int width, n_c_int height,
                     n_c_int patch_size, n_c_int texture_threshold,
                     n_byte result []);

#endif
