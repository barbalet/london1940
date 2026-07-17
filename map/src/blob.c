#include "map2json.h"
#include "toolkit.h"

static n_c_int detect_blobs_push(n_byte img[], n_c_int width, n_c_int height,
                                 n_c_int x, n_c_int y,
                                 n_c_int r, n_c_int g, n_c_int b,
                                 n_byte result[],
                                 n_c_int bounding_box[],
                                 n_c_int ** pending,
                                 n_int * pending_capacity,
                                 n_int * pending_count)
{
    n_int n, max_points, new_capacity;
    n_c_int * resized;

    if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
        return 1;
    }
    n = (n_int)y*width + x;
    if (img[n] == BACKGROUND) {
        return 1;
    }
    n *= 3;
    if (result[n] != BACKGROUND) {
        return 1;
    }
    if (*pending_count >= *pending_capacity) {
        max_points = (n_int)width * (n_int)height;
        new_capacity = (*pending_capacity) * 2;
        if (new_capacity > max_points) {
            new_capacity = max_points;
        }
        if (new_capacity <= *pending_capacity) {
            printf("blob fill exceeded image point capacity\n");
            return 0;
        }
        resized = (n_c_int*)realloc(*pending, new_capacity*2*sizeof(n_c_int));
        if (resized == NULL) {
            printf("Unable to grow blob fill stack\n");
            return 0;
        }
        *pending = resized;
        *pending_capacity = new_capacity;
    }
    result[n] = b;
    result[n+1] = g;
    result[n+2] = r;

    (*pending)[(*pending_count)*2] = x;
    (*pending)[(*pending_count)*2+1] = y;
    (*pending_count)++;

    if (x < bounding_box[0]) bounding_box[0] = x;
    if (y < bounding_box[1]) bounding_box[1] = y;
    if (x > bounding_box[2]) bounding_box[2] = x;
    if (y > bounding_box[3]) bounding_box[3] = y;

    return 1;
}

static n_c_int detect_blobs_point(n_byte img[], n_c_int width, n_c_int height,
                                  n_c_int x, n_c_int y,
                                  n_c_int r, n_c_int g, n_c_int b,
                                  n_byte result[],
                                  n_c_int bounding_box[], n_c_int search_radius,
                                  n_c_int depth, n_c_int max_depth)
{
    n_int pending_capacity = 1024;
    n_int pending_count = 0;
    n_int pending_index = 0;
    n_int n;
    n_c_int xx, yy, current_x, current_y;
    n_c_int * pending = NULL;

    (void)depth;
    (void)max_depth;

    if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
        return 0;
    }
    n = (n_int)y*width + x;
    if ((img[n] == BACKGROUND) || (result[n*3] != BACKGROUND)) {
        return 0;
    }

    pending = (n_c_int*)malloc(pending_capacity*2*sizeof(n_c_int));
    if (pending == NULL) {
        printf("Unable to allocate blob fill stack\n");
        return 0;
    }
    if (detect_blobs_push(img, width, height, x, y, r, g, b, result,
                          bounding_box, &pending,
                          &pending_capacity, &pending_count) == 0) {
        free(pending);
        return 0;
    }

    while (pending_index < pending_count) {
        current_x = pending[pending_index*2];
        current_y = pending[pending_index*2+1];
        pending_index++;
        for (yy = current_y - search_radius; yy <= current_y + search_radius; yy++) {
            for (xx = current_x - search_radius; xx <= current_x + search_radius; xx++) {
                if ((xx == current_x) && (yy == current_y)) {
                    continue;
                }
                if (detect_blobs_push(img, width, height, xx, yy, r, g, b,
                                      result, bounding_box, &pending,
                                      &pending_capacity, &pending_count) == 0) {
                    free(pending);
                    return 1;
                }
            }
        }
    }

    free(pending);
    return (pending_count > 0);
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
