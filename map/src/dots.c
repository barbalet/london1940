#include "map2json.h"
#include "toolkit.h"

void remove_polygon(n_map_polygons * polygons, n_c_int index)
{
    n_c_int remove_vertices = polygons->vertices[index];
    n_c_int i, v, start_vertex_index = 0, total_vertices = 0;

    for (i = 0; i < index; i++) {
        start_vertex_index += polygons->vertices[i];
    }
    for (i = 0; i < polygons->count; i++) {
        total_vertices += polygons->vertices[i];
    }

    for (v = start_vertex_index; v < total_vertices - remove_vertices; v++) {
        polygons->vertices[v] = polygons->vertices[v + remove_vertices];
    }

    for (i = index; i < polygons->count-1; i++) {
        polygons->id[i] = polygons->id[i+1];
        polygons->vertices[i] = polygons->vertices[i+1];
        polygons->coords[i*2] = polygons->coords[(i+1)*2];
        polygons->coords[i*2+1] = polygons->coords[(i+1)*2+1];
    }
    polygons->count--;
}

n_c_int polygons_to_points(n_map_polygons * polygons,
                           n_c_int points[], n_c_int max_points)
{
    n_c_int i, v, no_of_points = 0, index = 0;
    n_int av_x, av_y, no_of_vertices;

    for (i = 0; i < polygons->count; i++) {
        av_x = 0;
        av_y = 0;
        no_of_vertices = polygons->vertices[i];
        for (v = 0; v < no_of_vertices; v++, index++) {
            av_x += (n_int)polygons->coords[2*index];
            av_y += (n_int)polygons->coords[2*index + 1];
        }
        points[i*2] = (n_c_int)(av_x / no_of_vertices);
        points[i*2 + 1] = (n_c_int)(av_y / no_of_vertices);
        no_of_points++;
        if (no_of_points >= max_points) break;
    }
    return no_of_points;
}

n_c_int detect_dots(n_map_polygons * polygons, n_c_int spacing,
                    n_c_int * dot_coords,
                    n_byte result[], n_c_int width, n_c_int height)
{
    n_c_int i, j, neighbour_dots;
    n_int x0, y0, x1, y1, dx, dy, dist;
    n_int spacing2 = (n_int)spacing * (n_int)spacing;
    n_int spacing3 = (n_int)(spacing*2) * (n_int)(spacing*2);
    n_byte * is_dot;

    memset(result, 255, width*height*3*sizeof(n_byte));

    if (polygons->count == 0) return 0;

    is_dot = (n_byte*)malloc(polygons->count*sizeof(n_byte));
    if (is_dot == NULL) {
        return 0;
    }

    /* get the centroid of each polygon */
    for (i = 0; i < polygons->count; i++) {
        is_dot[i] = 1;
    }
    polygons_to_points(polygons, dot_coords, polygons->count);

    /* remove any dots which are too close together */
    for (i = 0; i < polygons->count; i++) {
        x0 = (n_int)dot_coords[i*2];
        y0 = (n_int)dot_coords[i*2 + 1];
        for (j = i+1; j < polygons->count; j++) {
            if (is_dot[j] == 0) continue;
            x1 = (n_int)dot_coords[j*2];
            y1 = (n_int)dot_coords[j*2 + 1];
            dx = x1 - x0;
            dy = y1 - y0;
            dist = dx*dx + dy*dy;
            if (dist < spacing2) {
                is_dot[i] = 0;
                is_dot[j] = 0;
            }
        }
    }

    /* recreate the polygons, removing invalid dots */
    for (i = polygons->count-1; i >= 0; i--) {
        if (is_dot[i] == 0) {
            remove_polygon(polygons, i);
            for (j = i; j < polygons->count-1; j++) {
                dot_coords[j*2] = dot_coords[(j+1)*2];
                dot_coords[j*2+1] = dot_coords[(j+1)*2+1];
            }
        }
    }

    for (i = 0; i < polygons->count; i++) {
        is_dot[i] = 1;
    }

    /* remove dots which have not enough adjacent dots */
    for (i = 0; i < polygons->count; i++) {
        x0 = dot_coords[i*2];
        y0 = dot_coords[i*2 + 1];
        neighbour_dots = 0;
        for (j = 0; j < polygons->count; j++) {
            if (i == j) continue;
            x1 = dot_coords[j*2];
            y1 = dot_coords[j*2 + 1];
            dx = x1 - x0;
            dy = y1 - y0;
            dist = dx*dx + dy*dy;
            if (dist < spacing3) {
                neighbour_dots++;
            }
        }
        if (neighbour_dots == 0) {
            is_dot[i] = 0;
        }
    }

    /* recreate the polygons, removing invalid dots */
    for (i = polygons->count-1; i >= 0; i--) {
        if (is_dot[i] == 0) {
            remove_polygon(polygons, i);
            for (j = i; j < polygons->count-1; j++) {
                dot_coords[j*2] = dot_coords[(j+1)*2];
                dot_coords[j*2+1] = dot_coords[(j+1)*2+1];
            }
        }
    }

    for (i = 0; i < polygons->count; i++) {
        draw_point(result, (n_byte4)width, (n_byte4)height,
                   dot_coords[i*2], dot_coords[i*2+1],
                   4, 0, 255, 0);
    }

    free(is_dot);
    return 0;
}

void show_dots(n_byte image_data[], n_byte4 image_width, n_byte4 image_height,
               n_c_int no_of_dots, n_c_int dots[],
               n_byte r, n_byte g, n_byte b, n_c_int radius)
{
    n_c_int i;

    for (i = 0; i < no_of_dots; i++) {
        draw_point(image_data, image_width, image_height,
                   dots[i*2], dots[i*2+1], radius, r, g, b);
    }
}
