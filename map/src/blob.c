#include "map2json.h"
#include "toolkit.h"

static n_c_int detect_blobs_point(n_byte img[], n_c_int width, n_c_int height,
                                  n_c_int x, n_c_int y,
                                  n_c_int r, n_c_int g, n_c_int b,
                                  n_byte result[],
                                  n_c_int bounding_box[], n_c_int search_radius,
                                  n_c_int depth, n_c_int max_depth)
{
    n_c_int n, xx, yy;

    if (depth > max_depth) {
        return 0;
    }
    if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
        return 0;
    }
    n = y*width + x;
    if (img[n] == BACKGROUND) {
        return 0;
    }
    n *= 3;
    if (result[n] != BACKGROUND) {
        return 0;
    }
    result[n] = b;
    result[n+1] = g;
    result[n+2] = r;

    if (x < bounding_box[0]) bounding_box[0] = x;
    if (y < bounding_box[1]) bounding_box[1] = y;
    if (x > bounding_box[2]) bounding_box[2] = x;
    if (y > bounding_box[3]) bounding_box[3] = y;

    for (yy = y - search_radius; yy <= y + search_radius; yy++) {
        if (yy < 0) continue;
        if (yy >= height) break;
        for (xx = x - search_radius; xx <= x + search_radius; xx++) {
            if (xx < 0) continue;
            if (xx >= width) break;
            if ((xx == x) && (yy == y)) {
                continue;
            }
            detect_blobs_point(img, width, height, xx, yy, r, g, b,
                               result,
                               bounding_box, search_radius,
                               depth+1, max_depth);
        }
    }
    return 1;
}

/* blobby blobby blobby */
n_c_int detect_blobs(n_byte img[], n_c_int width, n_c_int height,
                     n_c_int search_radius,
                     n_c_int max_blob_points,
                     n_c_int blob_points[],
                     n_c_int min_blob_size,
                     n_c_int max_blob_size,
                     n_byte result[])
{
    n_c_int no_of_blobs = 0;
    n_c_int x, y, r, g, b, blob_width, blob_height;
    n_c_int bounding_box[4];

    memset(result, BACKGROUND, width*height*3*sizeof(n_byte));

    r = (n_byte)(rand()%254);
    g = (n_byte)(rand()%254);
    b = (n_byte)(rand()%254);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            bounding_box[0] = x;
            bounding_box[1] = y;
            bounding_box[2] = x;
            bounding_box[3] = y;
            detect_blobs_point(img, width, height,
                               x, y, r, g, b,
                               result,
                               bounding_box,
                               search_radius,
                               0, MAX_RECURSION_DEPTH);
            if (no_of_blobs < max_blob_points) {
                blob_width = bounding_box[2] - bounding_box[0];
                blob_height = bounding_box[3] - bounding_box[1];
                if ((blob_width < max_blob_size) &&
                        (blob_height < max_blob_size) &&
                        (blob_width > min_blob_size) &&
                        (blob_height > min_blob_size)) {
                    blob_points[no_of_blobs*2] = bounding_box[0] + (blob_width/2);
                    if (blob_points[no_of_blobs*2] >= width) {
                        blob_points[no_of_blobs*2] = width-1;
                    }
                    blob_points[no_of_blobs*2+1] = bounding_box[1] + (blob_height/2);
                    if (blob_points[no_of_blobs*2+1] >= height) {
                        blob_points[no_of_blobs*2+1] = height-1;
                    }
                    no_of_blobs++;
                }
            }
            else {
                y = height;
                break;
            }
            r = (n_byte)(rand()%254);
            g = (n_byte)(rand()%254);
            b = (n_byte)(rand()%254);
        }
    }

    return no_of_blobs;
}

/* removes blobs from a points list */
n_c_int remove_blobs_from_points(n_c_int blob_points[], n_c_int no_of_blobs,
                                 n_c_int line_points[], n_c_int no_of_line_points,
                                 n_c_int search_radius)
{
    n_c_int i, j, k, blob_x, blob_y, x, y, dx, dy;
    n_c_int search_radius2 = search_radius * search_radius;

    for (i = no_of_line_points-1; i >= 0; i--) {
        x = line_points[i*2];
        y = line_points[i*2+1];
        for (j = 0; j < no_of_blobs; j++) {
            blob_x = blob_points[j*2];
            blob_y = blob_points[j*2+1];
            /* is it close? */
            dx = blob_x - x;
            dy = blob_y - y;
            if (dx*dx + dy*dy > search_radius2) continue;
            /* remove this line point */
            for (k = i; k < no_of_line_points - 1; k++) {
                line_points[k*2] = line_points[(k+1)*2];
                line_points[k*2+1] = line_points[(k+1)*2+1];
            }
            no_of_line_points--;
            break;
        }
    }
    return no_of_line_points;
}
