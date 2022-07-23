#include "map2json.h"
#include "toolkit.h"

/**
 * @brief Removes a dark line
 * @param img containing image
 * @param width Width of the image
 * @param height Height of the image
 * @param bitsperpixel Number of bits per pixel
 * @param tx top left of the line
 * @param ty top of the line
 * @param bx bottom right of the line
 * @param by bottom of the line
 * @param line_width width of the line
 * @param max_intensity the maximum intensity value of the line
 */
void remove_dark_line(n_byte img[],
                      n_byte4 width, n_byte4 height,
                      n_c_int bitsperpixel,
                      n_c_int tx, n_c_int ty, n_c_int bx, n_c_int by,
                      n_c_int line_width,
                      n_c_int max_intensity)
{
    n_c_int linewidth = bx - tx;
    n_c_int lineheight = by - ty;
    n_c_int x, y, xx, yy, incr=1;
    n_c_int bytes_per_pixel = bitsperpixel/8;
    n_c_int half_width = line_width/2;
    n_c_int intensity;
    n_int n, n_left, n_above;

    max_intensity *= bytes_per_pixel;

    if (abs(lineheight) > abs(linewidth)) {
        /* vertical orientation */
        if (by < ty) incr = -1;
        for (y = ty; y != by; y+=incr) {
            if ((y < 0) || (y >= (int)height))
                continue;
            x = tx + ((y - ty) * linewidth / lineheight);
            if (x < 2) continue;
            n_left = (y * width + (x - half_width - 2)) * bytes_per_pixel;
            for (xx = x - half_width; xx < x - half_width + line_width; xx++) {
                if ((xx < 2) || (xx >= (int)width))
                    continue;
                n = (y * width + xx) * bytes_per_pixel;
                intensity = (n_c_int)img[n] + (n_c_int)img[n+1] + (n_c_int)img[n+2];
                if (intensity < max_intensity) {
                    img[n] = img[n_left];
                    img[n+1] = img[n_left+1];
                    img[n+2] = img[n_left+2];
                }
            }
        }
    }
    else {
        /* horizontal orientation */
        if (bx < tx) incr = -1;
        for (x = tx; x != bx; x+=incr) {
            if ((x < 0) || (x >= (int)width))
                continue;
            y = ty + ((x - tx) * lineheight / linewidth);
            if (y < 2) continue;
            n_above = ((y - half_width - 2) * width + x) * bytes_per_pixel;
            for (yy = y - half_width; yy < y - half_width + line_width; yy++) {
                if ((yy < 2) || (yy >= (int)height))
                    continue;
                n = (yy * width + x) * bytes_per_pixel;
                intensity = (n_c_int)img[n] + (n_c_int)img[n+1] + (n_c_int)img[n+2];
                if (intensity < max_intensity) {
                    img[n] = img[n_above];
                    img[n+1] = img[n_above+1];
                    img[n+2] = img[n_above+2];
                }
            }
        }
    }
}

n_c_int remove_lines_from_large_map(char * filename,
                                    n_byte large_map[],
                                    n_c_int large_width, n_c_int large_height,
                                    n_c_int bitsperpixel)
{
    const n_int line_width = 10;
    n_int index;
    n_c_int tx, ty, bx, by;
    n_c_int max_intensity = 90;

    if (strstr(filename, "160")) {
        const n_int lines[] = {
            1790,956, 1804,12118,
            4270,950, 4283,12109,
            6757,944, 6776,12105,
            9243,937, 9263,12102,
            548,2198, 10484,2177,
            554,4677, 10490,4659,
            559,7159, 10494,7140,
            563,9638, 10500,9620,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }

    if (strstr(filename, "161")) {
        const n_int lines[] = {
            486,2235, 10408,2227,
            487,4713, 10413,4704,
            489,7189, 10414,7182,
            488,9664, 10415,9662,
            1725,996, 1728,12141,
            4203,1071, 4204,12136,
            6686,993, 6689,12136,
            9166,992, 9174,12138,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }

    if (strstr(filename, "162")) {
        const n_int lines[] = {
            509,2182, 10443,2176,
            507,4661, 10440,4658,
            505,7137, 10442,7138,
            504,9615, 10446,9617,
            2983,942, 2986,12092,
            5469,938, 5469,12090,
            7960,628, 7961,12096,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }

    if (strstr(filename, "170")) {
        const n_int lines[] = {
            654,3432, 10567,3420,
            654,5906, 10569,5901,
            654,8385, 10570,8378,
            656,10867, 10572,10860,
            1889,955, 1898,12101,
            4365,951, 4373,12097,
            6849,947, 6854,12098,
            9327,947, 9339,12099,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }

    if (strstr(filename, "171")) {
        const n_int lines[] = {
            534,3418, 10481,3437,
            533,5893, 10480,5912,
            529,8367, 10474,8392,
            526,10846, 10469,10866,
            1783,944, 1765,12087,
            4261,948, 4249,12092,
            6753,950, 6736,12098,
            9236,957, 9222,12105,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }

    if (strstr(filename, "172")) {
        const n_int lines[] = {
            459,3426, 10400,3459,
            452,5904, 10395,5941,
            445,8381, 10387,8417,
            440,10859, 10380,10893,
            2946,957, 2916,12103,
            5431,964, 5397,12114,
            7921,972, 7890,12124,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }

    if (strstr(filename, "173")) {
        const n_int lines[] = {
            645,1704, 10589,1629,
            663,4183, 10608,4112,
            681,6663, 10625,6590,
            696,9142, 10640,9069,
            713,11623, 10654,11550,
            2626,945, 2703,12103,
            5111,927, 5184,12083,
            7604,909, 7677,12065,
            10088,890, 10160,12052,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }

    if (strstr(filename, "182")) {
        const n_int lines[] = {
            606,2276, 10545,2313,
            595,4751, 10537,4796,
            584,7235, 10528,7279,
            575,9714, 10517,9760,
            1851,1090, 1804,12192,
            4334,1052, 4284,12201,
            6827,1060, 6779,12215,
            9312,1071, 9266,12229,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }

    if (strstr(filename, "183")) {
        const n_int lines[] = {
            540,3070, 10485,3095,
            529,8031, 10473,8060,
            525,10511, 10467,10537,
            1785,1106, 1759,12249,
            4267,1113, 4241,12254,
            6764,1100, 6737,12260,
            9249,1106, 9223,12267,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }

    if (strstr(filename, "184")) {
        const n_int lines[] = {
            523,3429, 10455,3429,
            523,5909, 10456,5908,
            524,8387, 10454,8386,
            525,10866, 10456,10864,
            2999,952, 3004,12102,
            5484,952, 5486,12098,
            7970,985, 7979,12100,
            0,0, 0,0
        };
        index = 0;
        while (lines[index] != 0) {
            tx = lines[index];
            ty = lines[index+1];
            bx = lines[index+2];
            by = lines[index+3];
            remove_dark_line(large_map,
                             (n_byte4)large_width, (n_byte4)large_height,
                             bitsperpixel,
                             tx, ty, bx, by, line_width, max_intensity);
            index += 4;
        }
    }
    return 0;
}

n_c_int get_large_map_metadata(char * filename,
                               n_c_int * tiles_across, n_c_int * tiles_down,
                               n_c_int * large_tx, n_c_int * large_ty,
                               n_c_int * large_bx, n_c_int * large_by,
                               float * tx_long, float * ty_lat,
                               float * bx_long, float * by_lat)
{
    n_c_int retval = 1;

    /* NOTE: coordinates are in ordinance survey grid squares,
       rather than latitude and longitude */

    if (strstr(filename, "160")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 564;
        *large_ty = 961;
        *large_bx = 10482;
        *large_by = 12102;
        *tx_long = 495;
        *ty_lat = 225;
        *bx_long = 535;
        *by_lat = 180;
        retval = 0;
    }
    else if (strstr(filename, "161")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 491;
        *large_ty = 996;
        *large_bx = 10404;
        *large_by = 12138;
        *tx_long = 535;
        *ty_lat = 225;
        *bx_long = 575;
        *by_lat = 180;
        retval = 0;
    }
    else if (strstr(filename, "162")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 506;
        *large_ty = 946;
        *large_bx = 10442;
        *large_by = 12098;
        *tx_long = 570;
        *ty_lat = 225;
        *bx_long = 610;
        *by_lat = 180;
        retval = 0;
    }
    else if (strstr(filename, "170")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 654;
        *large_ty = 956;
        *large_bx = 10572;
        *large_by = 12096;
        *tx_long = 495;
        *ty_lat = 180;
        *bx_long = 535;
        *by_lat = 135;
        retval = 0;
    }
    else if (strstr(filename, "171")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 542;
        *large_ty = 938;
        *large_bx = 10462;
        *large_by = 12106;
        *tx_long = 535;
        *ty_lat = 180;
        *bx_long = 575;
        *by_lat = 135;
        retval = 0;
    }
    else if (strstr(filename, "172")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 469;
        *large_ty = 949;
        *large_bx = 10371;
        *large_by = 12135;
        *tx_long = 570;
        *ty_lat = 180;
        *bx_long = 610;
        *by_lat = 135;
        retval = 0;
    }
    else if (strstr(filename, "173")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 644;
        *large_ty = 963;
        *large_bx = 10657;
        *large_by = 12048;
        *tx_long = 602;
        *ty_lat = 173;
        *bx_long = 642;
        *by_lat = 128;
        retval = 0;
    }
    else if (strstr(filename, "182")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 614;
        *large_ty = 1036;
        *large_bx = 10507;
        *large_by = 12235;
        *tx_long = 495;
        *ty_lat = 145;
        *bx_long = 535;
        *by_lat = 100;
        retval = 0;
    }
    else if (strstr(filename, "183")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 544;
        *large_ty = 1090;
        *large_bx = 10464;
        *large_by = 12268;
        *tx_long = 535;
        *ty_lat = 138;
        *bx_long = 575;
        *by_lat = 93;
        retval = 0;
    }
    else if (strstr(filename, "184")) {
        *tiles_across = 7;
        *tiles_down = 11;
        *large_tx = 526;
        *large_ty = 955;
        *large_bx = 10456;
        *large_by = 12101;
        *tx_long = 570;
        *ty_lat = 150;
        *bx_long = 610;
        *by_lat = 105;
        retval = 0;
    }
    return retval;
}


/* copy an individual tile from a larger map image */
void copy_tile(n_byte large_map[], n_c_int large_width, n_byte tile_data[],
               n_c_int tx, n_c_int ty, n_c_int bx, n_c_int by)
{
    n_c_int x, y, n, n_tile, ch;
    n_c_int tile_width = bx - tx;

    for (y = ty; y < by; y++) {
        for (x = tx; x < bx; x++) {
            n = (y*large_width + x)*3;
            n_tile = ((y - ty)*tile_width + (x - tx))*3;
            for (ch=0; ch < 3; ch++) {
                tile_data[n_tile+ch] = large_map[n+ch];
            }
        }
    }
}

/* breaks up a large map image into smaller tiles and saves them
   to a tiles directory */
n_c_int breakup_large_map(char * large_filename,
                          n_byte large_map[], n_c_int large_width, n_c_int large_height,
                          n_c_int image_bitsperpixel,
                          char * tiles_directory,
                          n_c_int overlap_percent)
{
    n_c_int tile_x, tile_y, tx, ty, bx, by, tile_width, tile_height;
    n_byte * tile_data;
    n_c_int bytesperpixel = (image_bitsperpixel/8);
    n_c_int start_latitude, start_longitude;
    n_c_int end_latitude, end_longitude;
    char tile_filename[256];
    n_c_int tiles_across = 12;
    n_c_int tiles_down = 22;
    n_c_int large_tx = 0;
    n_c_int large_ty = 0;
    n_c_int large_bx = large_width;
    n_c_int large_by = large_height;
    float tx_long = 1;
    float ty_lat = 3;
    float bx_long = 4;
    float by_lat = 7;
    float fraction;

    if (get_large_map_metadata(large_filename,
                               &tiles_across, &tiles_down,
                               &large_tx, &large_ty,
                               &large_bx, &large_by,
                               &tx_long, &ty_lat,
                               &bx_long, &by_lat) != 0) {
        printf("No metadata for map %s\n", large_filename);
        return 2;
    }

    remove_lines_from_large_map(large_filename,
                                large_map, large_width, large_height,
                                image_bitsperpixel);

    tile_width = (large_bx - large_tx) / tiles_across;
    tile_height = (large_by - large_ty) / tiles_down;

    /* expand the tile size for overlap */
    tile_width = tile_width * (100 + overlap_percent) / 100;
    tile_height = tile_height * (100 + overlap_percent) / 100;

    tile_data = (n_byte*)malloc(tile_width*tile_height*bytesperpixel*sizeof(n_byte));
    if (tile_data == NULL) {
        printf("Unable to allocate memory for tile.\n");
        return 1;
    }

    for (tile_y = 0; tile_y < tiles_down; tile_y++) {
        /* pixel vertical coords */
        ty = large_ty + (tile_y * (large_by - large_ty) / tiles_down);
        by = ty + tile_height;
        if (by > large_by) {
            by = large_by;
            ty = large_by - tile_height;
        }

        /* vertical geocoords */
        fraction = (ty - large_ty) / (float)(large_by - large_ty);
        start_latitude = (n_c_int)((ty_lat + (fraction * (by_lat - ty_lat))) * 1000);
        fraction = (by - large_ty) / (float)(large_by - large_ty);
        end_latitude = (n_c_int)((ty_lat + (fraction * (by_lat - ty_lat))) * 1000);

        for (tile_x = 0; tile_x < tiles_across; tile_x++) {
            /* pixel horizontal coords */
            tx = large_tx + (tile_x * (large_bx - large_tx) / tiles_across);
            bx = tx + tile_width;
            if (bx > large_bx) {
                bx = large_bx;
                tx = large_bx - tile_width;
            }

            /* horizontal geocoords */
            fraction = (tx - large_tx) / (float)(large_bx - large_tx);
            start_longitude = (n_c_int)((tx_long + (fraction * (bx_long - tx_long))) * 1000);
            fraction = (bx - large_tx) / (float)(large_bx - large_tx);
            end_longitude = (n_c_int)((tx_long + (fraction * (bx_long - tx_long))) * 1000);

            sprintf(tile_filename,"%s/%d_%d#%d_%d#%d_%d#%d_%d.png", tiles_directory,
                    tile_y, tile_x, start_latitude, start_longitude,
                    end_latitude, end_longitude,
                    tx, ty);
            printf("Tile: %s\n", tile_filename);

            copy_tile(large_map, large_width, tile_data, tx, ty, bx, by);
            write_png_file(tile_filename, tile_width, tile_height, 24, tile_data);
        }
    }

    free(tile_data);
    return 0;
}

static n_c_int free_map_lines(n_map_lines * maplines)
{
    if (maplines->points != NULL) {
        free(maplines->points);
    }
    if (maplines->width != NULL) {
        free(maplines->width);
    }
    if (maplines->links != NULL) {
        free(maplines->links);
    }
    return 0;
}

static n_c_int free_map_polygons(n_map_polygons * poly)
{
    if (poly->vertices != NULL) {
        free(poly->vertices);
    }
    if (poly->id != NULL) {
        free(poly->id);
    }
    if (poly->coords != NULL) {
        free(poly->coords);
    }
    return 0;
}

/* free as in map data */
n_c_int free_map_data(n_map_data * mapdata)
{
    free_map_polygons(&mapdata->buildings);
    free_map_polygons(&mapdata->woods);
    free_map_polygons(&mapdata->sea);
    free_map_polygons(&mapdata->sands);
    free_map_polygons(&mapdata->orchards);

    free_map_lines(&mapdata->water);
    free_map_lines(&mapdata->main_road);
    free_map_lines(&mapdata->minor_road);
    free_map_lines(&mapdata->harbour);
    free_map_lines(&mapdata->railway_line);
    free_map_lines(&mapdata->railway_tunnel);

    if (mapdata->junction_points != NULL) {
        free(mapdata->junction_points);
    }
    if (mapdata->station_points != NULL) {
        free(mapdata->station_points);
    }
    if (mapdata->orchard_points != NULL) {
        free(mapdata->orchard_points);
    }
    if (mapdata->bridge_points != NULL) {
        free(mapdata->bridge_points);
    }
    return 0;
}

static n_c_int create_map_polygons(n_c_int max_total_polygon_points,
                                   n_map_polygons * poly)
{
    poly->count = 0;
    poly->vertices = NULL;
    poly->id = NULL;
    poly->coords = NULL;

    poly->vertices = (n_c_int*)malloc(max_total_polygon_points*sizeof(n_c_int));
    if (poly->vertices == NULL) {
        printf("Unable to allocate memory for building polygon vertices (%d bytes)\n",
               (int)max_total_polygon_points*(int)sizeof(n_c_int));
        return 1;
    }

    poly->id = (n_c_int*)malloc(max_total_polygon_points*sizeof(n_c_int));
    if (poly->id == NULL) {
        printf("Unable to allocate memory for building polygon id (%d bytes)\n",
               (int)max_total_polygon_points*(int)sizeof(n_c_int));
        return 1;
    }

    poly->coords = (n_c_int*)malloc(max_total_polygon_points*2*sizeof(n_c_int));
    if (poly->coords == NULL) {
        printf("Unable to allocate memory for building polygons (%d bytes)\n",
               (int)max_total_polygon_points*2*(int)sizeof(n_c_int));
        return 1;
    }
    return 0;
}

n_c_int create_map_lines(n_c_int max_points,
                         n_map_lines * maplines,
                         n_byte has_width)
{
    maplines->count = 0;
    maplines->points = NULL;
    maplines->width = NULL;
    maplines->links = NULL;

    maplines->points = (n_c_int*)malloc(max_points*2*sizeof(n_c_int));
    if (maplines->points == NULL) {
        printf("Unable to allocate memory for line points (%d bytes)\n",
               (int)max_points*2*(int)sizeof(n_c_int));
        return 1;
    }

    maplines->width = NULL;
    if (has_width != 0) {
        maplines->width = (n_c_int*)malloc(max_points*sizeof(n_c_int));
        if (maplines->width == NULL) {
            printf("Unable to allocate memory for line point width (%d bytes)\n",
                   (int)max_points*(int)sizeof(n_c_int));
            return 1;
        }
    }

    maplines->links = (n_c_int*)malloc(max_points*2*sizeof(n_c_int));
    if (maplines->links == NULL) {
        printf("Unable to allocate memory for line links (%d bytes)\n",
               (int)max_points*2*(int)sizeof(n_c_int));
        return 1;
    }

    return 0;
}

n_c_int create_map_data(n_c_int max_total_polygon_points,
                        n_c_int max_road_points,
                        n_c_int max_junctions,
                        n_c_int max_station_points,
                        n_c_int max_orchard_points,
                        n_c_int max_bridges,
                        n_map_data * mapdata)
{
    memset(mapdata->offset, 0, 2*sizeof(n_c_int));
    memset(mapdata->resolution, 0, 2*sizeof(n_c_int));
    memset(mapdata->geocoords, 0, 4*sizeof(double));
    mapdata->water.count = 0;
    mapdata->main_road.count = 0;
    mapdata->minor_road.count = 0;
    mapdata->harbour.count = 0;
    mapdata->railway_line.count = 0;
    mapdata->railway_tunnel.count = 0;
    mapdata->no_of_junctions = 0;
    mapdata->no_of_stations = 0;
    mapdata->no_of_orchards = 0;
    mapdata->no_of_bridges = 0;

    mapdata->buildings.vertices = NULL;
    mapdata->buildings.id = NULL;
    mapdata->buildings.coords = NULL;

    mapdata->woods.vertices = NULL;
    mapdata->woods.id = NULL;
    mapdata->woods.coords = NULL;

    mapdata->sea.vertices = NULL;
    mapdata->sea.id = NULL;
    mapdata->sea.coords = NULL;

    mapdata->sands.vertices = NULL;
    mapdata->sands.id = NULL;
    mapdata->sands.coords = NULL;

    mapdata->orchards.vertices = NULL;
    mapdata->orchards.id = NULL;
    mapdata->orchards.coords = NULL;

    mapdata->water.points = NULL;
    mapdata->water.width = NULL;
    mapdata->water.links = NULL;

    mapdata->main_road.points = NULL;
    mapdata->main_road.links = NULL;

    mapdata->minor_road.points = NULL;
    mapdata->minor_road.links = NULL;

    mapdata->harbour.points = NULL;
    mapdata->harbour.links = NULL;

    mapdata->railway_line.points = NULL;
    mapdata->railway_line.links = NULL;

    mapdata->railway_tunnel.points = NULL;
    mapdata->railway_tunnel.links = NULL;

    mapdata->junction_points = NULL;
    mapdata->station_points = NULL;
    mapdata->bridge_points = NULL;

    if (create_map_polygons(max_total_polygon_points,
                            &mapdata->buildings) != 0) {
        free_map_data(mapdata);
        return 1;
    }

    if (create_map_polygons(max_total_polygon_points,
                            &mapdata->woods) != 0) {
        free_map_data(mapdata);
        return 1;
    }

    if (create_map_polygons(max_total_polygon_points,
                            &mapdata->sea) != 0) {
        free_map_data(mapdata);
        return 1;
    }

    if (create_map_polygons(max_total_polygon_points,
                            &mapdata->sands) != 0) {
        free_map_data(mapdata);
        return 1;
    }

    if (create_map_polygons(max_total_polygon_points,
                            &mapdata->orchards) != 0) {
        free_map_data(mapdata);
        return 1;
    }

    if (create_map_lines(max_road_points, &mapdata->water, 1) != 0) {
        free_map_data(mapdata);
    }

    if (create_map_lines(max_road_points, &mapdata->main_road, 0) != 0) {
        free_map_data(mapdata);
    }

    if (create_map_lines(max_road_points, &mapdata->minor_road, 0) != 0) {
        free_map_data(mapdata);
    }

    if (create_map_lines(max_road_points, &mapdata->railway_line, 0) != 0) {
        free_map_data(mapdata);
    }

    if (create_map_lines(max_road_points, &mapdata->harbour, 0) != 0) {
        free_map_data(mapdata);
    }

    if (create_map_lines(max_road_points, &mapdata->railway_tunnel, 0) != 0) {
        free_map_data(mapdata);
    }

    mapdata->junction_points = (n_c_int*)malloc(max_junctions*2*sizeof(n_c_int));
    if (mapdata->junction_points == NULL) {
        printf("Unable to allocate memory for junction points (%d bytes)\n",
               (int)max_junctions*2*(int)sizeof(n_c_int));
        free_map_data(mapdata);
        return 1;
    }

    mapdata->station_points = (n_c_int*)malloc(max_station_points*2*sizeof(n_c_int));
    if (mapdata->station_points == NULL) {
        printf("Unable to allocate memory for station points (%d bytes)\n",
               (int)max_station_points*2*(int)sizeof(n_c_int));
        free_map_data(mapdata);
        return 1;
    }

    mapdata->orchard_points = (n_c_int*)malloc(max_orchard_points*2*sizeof(n_c_int));
    if (mapdata->orchard_points == NULL) {
        printf("Unable to allocate memory for orchard points (%d bytes)\n",
               (int)max_orchard_points*2*(int)sizeof(n_c_int));
        free_map_data(mapdata);
        return 1;
    }

    mapdata->bridge_points = (n_c_int*)malloc(max_bridges*4*sizeof(n_c_int));
    if (mapdata->bridge_points == NULL) {
        printf("Unable to allocate memory for junction points (%d bytes)\n",
               (int)max_bridges*4*(int)sizeof(n_c_int));
        free_map_data(mapdata);
        return 1;
    }

    return 0;
}
