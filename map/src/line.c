#include "map2json.h"
#include "toolkit.h"

/* tries to thin out clumped points */
n_c_int thin_line_point_clumps(n_c_int line_points[],
                               n_c_int no_of_line_points,
                               n_c_int radius,
                               n_c_int max_close_points)
{
    n_c_int i=0, j, k, ref_x, ref_y, x, y, dx, dy, close_points;
    n_c_int dist, dist2 = radius * radius;
    n_c_int dist_wide2 = (radius*2) * (radius*2);
    n_c_int wide_points, max_wide_points = max_close_points*3;

    while (i < no_of_line_points) {
        ref_x = line_points[i*2];
        ref_y = line_points[i*2+1];
        close_points = 0;
        wide_points = 0;
        for (j = no_of_line_points-1; j >= 0; j--) {
            if (j == i) continue;
            x = line_points[j*2];
            y = line_points[j*2+1];
            dx = x - ref_x;
            dy = y - ref_y;
            dist = dx*dx + dy*dy;
            if (dist <= dist_wide2) {
                wide_points++;
                if (dist <= dist2) {
                    close_points++;
                }
            }
        }
        if ((close_points > max_close_points) &&
                (wide_points > max_wide_points)) {
            /* remove this point */
            for (k = i; k < no_of_line_points-1; k++) {
                line_points[k*2] = line_points[(k+1)*2];
                line_points[k*2+1] = line_points[(k+1)*2+1];
            }
            no_of_line_points--;
        }
        else {
            i++;
        }
    }

    return no_of_line_points;
}

/* this removes any points which are too close together */
n_c_int remove_close_points(n_c_int line_points[],
                            n_c_int no_of_line_points,
                            n_c_int radius)
{
    n_c_int i=0, j, k, ref_x, ref_y, x, y, dx, dy;
    n_c_int min_dist = radius * radius;
    n_byte removals;

    while (i < no_of_line_points) {
        ref_x = line_points[i*2];
        ref_y = line_points[i*2+1];
        removals = 0;
        for (j = no_of_line_points-1; j >= 0; j--) {
            if (j == i) continue;
            x = line_points[j*2];
            y = line_points[j*2+1];
            dx = x - ref_x;
            dy = y - ref_y;
            if (dx*dx + dy*dy < min_dist) {
                /* remove this point */
                for (k = j; k < no_of_line_points-1; k++) {
                    line_points[k*2] = line_points[(k+1)*2];
                    line_points[k*2+1] = line_points[(k+1)*2+1];
                }
                no_of_line_points--;
                removals = 1;
            }
        }
        if (removals == 0) {
            i++;
        }
    }

    return no_of_line_points;
}

n_c_int show_line_points(n_byte * result, n_c_int width, n_c_int height,
                         n_c_int line_points[], n_c_int no_of_line_points,
                         n_c_int radius)
{
    n_c_int i, x, y;

    memset(result, BACKGROUND, width*height*3*sizeof(n_byte));

    for (i = 0; i < no_of_line_points; i++) {
        x = line_points[i*2];
        y = line_points[i*2+1];

        draw_point(result, (n_byte4)width, (n_byte4)height,
                   x, y, radius, 0, 0, 0);
    }
    return 0;
}

/* returns percentage of supporting evidence for the existence of a link */
static n_c_int line_evidence(n_c_int tx, n_c_int ty,
                             n_c_int bx, n_c_int by,
                             n_byte thresholded_ref[],
                             n_c_int image_width, n_c_int image_height)
{
    n_c_int evidence = 0;
    n_c_int dx, dy, i, x, y, xx, yy;
    n_int n;

    dx = bx - tx;
    dy = by - ty;
    for (i = 0; i < LINE_EVIDENCE_SAMPLES; i++) {
        x = tx + (dx * i / LINE_EVIDENCE_SAMPLES);
        if ((x < 0) || (x >= image_width)) continue;
        y = ty + (dy * i / LINE_EVIDENCE_SAMPLES);
        if ((y < 0) || (y >= image_height)) continue;
        for (yy = y-1; yy <= y+1; yy++) {
            if (yy < 0) continue;
            if (yy >= image_height) break;
            for (xx = x-1; xx <= x+1; xx++) {
                if (xx < 0) continue;
                if (xx >= image_width) break;
                n = yy*image_width + xx;
                if (thresholded_ref[n] == 0) {
                    evidence++;
                    yy = y+2;
                    break;
                }
            }
        }
    }
    return evidence * 100 / LINE_EVIDENCE_SAMPLES;
}

/* returns the index of the closest point to the given position,
   ignoring a list of existing points */
static n_c_int closest_line_point(n_c_int x, n_c_int y,
                                  n_c_int line_points[], n_c_int no_of_line_points,
                                  n_c_int max_distance,
                                  n_c_int ignore_points[], n_c_int no_of_ignore_points,
                                  n_byte thresholded_ref[],
                                  n_c_int image_width, n_c_int image_height,
                                  n_c_int line_links[])
{
    n_c_int i, j, dx, dy, dist2, index, possible_x, possible_y, ignored_x, ignored_y;
    n_byte opposing, ignored;
    n_c_int max_dist2 = max_distance * max_distance;
    n_c_int closest_point_index = -1;

    for (i = 0; i < no_of_line_points; i++) {
        if (line_links[i*2+1] != 0) continue;
        /* should this point be ignored? */
        ignored = 0;
        for (j = 0; j < no_of_ignore_points; j++) {
            if (ignore_points[j] == i) {
                ignored = 1;
                break;
            }
        }
        if (ignored == 1) continue;

        /* distance to the point */
        possible_x = line_points[i*2];
        possible_y = line_points[i*2+1];
        dx = possible_x - x;
        dy = possible_y - y;
        /* check that the closest points are in opposite directions */
        /* if (opposite_signs(dx, dy, prev_dx, prev_dy) == 0) continue; */
        dist2 = dx*dx + dy*dy;
        if (dist2 < max_dist2) {
            /* is it more distant from the ignored points? */
            opposing = 1;
            /* start at index of 1 to skip over the current point */
            for (j = 1; j < no_of_ignore_points; j++) {
                index = ignore_points[j];
                ignored_x = line_points[index*2];
                ignored_y = line_points[index*2+1];
                dx = possible_x - ignored_x;
                dy = possible_y - ignored_y;
                if (dx*dx + dy*dy < dist2) {
                    opposing = 0;
                    break;
                }
            }
            if (opposing == 1) {
                /* is there supporting evidence? */
                if (line_evidence(x, y, x + dx, y + dy, thresholded_ref,
                                  image_width, image_height) >= 70) {
                    /* is close */
                    closest_point_index = i;
                    max_dist2 = dist2;
                }
            }
        }
    }
    return closest_point_index;
}

static n_c_int line_length(n_c_int line_links[],
                           n_c_int line_index, n_byte direction)
{
    n_c_int length = 0, index = line_index, start_index = line_index, idx;
    n_byte followed_link[MAX_ROAD_POINTS];

    memset(followed_link, 0, MAX_ROAD_POINTS*sizeof(n_byte));

    while (line_links[index*2 + direction] != 0) {
        idx = index;
        index = line_links[index*2 + direction]-1;
        if (index == -1) break;
        if ((index == start_index) || (followed_link[index] != 0)) {
            /* loop detected */
            break;
        }
        followed_link[idx] = 1;
        length++;
    }
    return length;
}

static void prune_line_links(n_c_int line_links[],
                             n_c_int line_index, n_byte direction)
{
    n_c_int idx, length = 0;
    n_c_int index = line_index, start_index = line_index;
    n_byte followed_link[MAX_ROAD_POINTS];

    memset(followed_link, 0, MAX_ROAD_POINTS*sizeof(n_byte));

    while (line_links[index*2 + direction] != 0) {
        idx = index;
        index = line_links[index*2 + direction]-1;
        if (index == -1) break;
        if ((index == start_index) || (followed_link[index] != 0)) {
            /* loop detected */
            break;
        }
        followed_link[idx] = 1;
        /* clear the link */
        line_links[idx*2 + direction] = 0;
        length++;
    }
}

static n_c_int prune_short_lines(n_c_int line_links[],
                                 n_c_int no_of_line_points,
                                 n_c_int min_length)
{
    n_c_int p, length, links_removed = 0;
    n_byte direction;

    for (p = 0; p < no_of_line_points; p++) {
        for (direction = NEXT_JUNCTION; direction <= PREV_JUNCTION; direction++) {
            length = line_length(line_links, p, direction);
            if (length < min_length) {
                prune_line_links(line_links, p, direction);
                links_removed += length;
            }
        }
    }
    if (links_removed > 0) {
        printf("Removed %d short links\n", links_removed);
    }
    else {
        printf("No short links removed\n");
    }
    return links_removed;
}

n_c_int connect_line_sections(n_c_int line_points[], n_c_int no_of_line_points,
                              n_c_int max_distance, n_c_int line_links[])
{
    n_c_int i, j, dx, dy, dist, min_dist, min_index;
    n_c_int tx, ty, bx, by, connections = 0;
    n_c_int max_distance2 = max_distance * max_distance;

    for (i = 0; i < no_of_line_points; i++) {
        if ((line_links[i*2] != 0) && (line_links[i*2+1] != 0)) {
            continue;
        }
        if ((line_links[i*2] == 0) && (line_links[i*2+1] == 0)) {
            continue;
        }
        tx = line_points[i*2];
        ty = line_points[i*2+1];
        min_dist = max_distance2;
        min_index = -1;
        for (j = i+1; j < no_of_line_points; j++) {
            if ((line_links[j*2] != 0) && (line_links[j*2+1] != 0)) {
                continue;
            }
            if ((line_links[j*2] == 0) && (line_links[j*2+1] == 0)) {
                continue;
            }
            bx = line_points[j*2];
            by = line_points[j*2+1];
            dx = bx - tx;
            dy = by - ty;
            dist = dx*dx + dy*dy;
            if (dist < min_dist) {
                min_dist = dist;
                min_index = j;
            }
        }
        if (min_index == -1) continue;
        connections++;
        if (line_links[i*2] == 0) {
            line_links[i*2] = min_index+1;
        }
        else {
            line_links[i*2+1] = min_index+1;
        }
        if (line_links[min_index*2+1] == 0) {
            line_links[min_index*2+1] = i+1;
        }
        else {
            if (line_links[min_index*2] == 0) {
                line_links[min_index*2] = i+1;
            }
        }
    }
    return connections;
}


n_c_int link_line_points(n_c_int line_points[], n_c_int no_of_line_points,
                         n_c_int max_distance, n_c_int line_links[],
                         n_byte thresholded_ref[],
                         n_c_int image_width, n_c_int image_height,
                         n_c_int min_link_length)
{
    n_c_int i, j, x, y, closest_point_index;
    n_c_int no_of_ignore_points = 0, no_of_links = 0;
    n_c_int ignore_points[5];

    /* all zeros mean that all points are not linked */
    memset(line_links, 0, no_of_line_points*2*sizeof(n_c_int));

    for (i = 0; i < no_of_line_points; i++) {
        x = line_points[i*2];
        y = line_points[i*2+1];

        /* ignore the current point when looking for close points */
        no_of_ignore_points = 1;
        ignore_points[0] = i;

        /* if there is already an assigned previous point then
           add it to the ignore list */
        if (line_links[i*2+1] != 0) {
            no_of_ignore_points = 2;
            ignore_points[1] = line_links[i*2+1]-1;
        }

        /* find the two closest points */
        closest_point_index =
            closest_line_point(x, y, line_points, no_of_line_points,
                               max_distance, ignore_points,
                               no_of_ignore_points,
                               thresholded_ref, image_width, image_height,
                               line_links);
        if (closest_point_index > -1) {
            /* link to the next point */
            line_links[i*2] = closest_point_index+1;
            /* next point links back */
            line_links[closest_point_index*2+1] = i+1;
            ignore_points[no_of_ignore_points++] = closest_point_index;
            no_of_links++;
        }
    }

    if (min_link_length > 0) {
        no_of_links -= prune_short_lines(line_links, no_of_line_points,
                                         min_link_length);
    }

    for (i = 0; i < no_of_line_points; i++) {
        if (line_links[i*2] == 0) continue;
        if (line_links[i*2+1] != 0) continue;
        for (j = 0; j < no_of_line_points; j++) {
            if (j == i) continue;
            if (line_links[j*2]-1 == i) {
                line_links[i*2+1] = j+1;
            }
        }
    }

    for (i = 0; i < no_of_line_points; i++) {
        if (line_links[i*2+1] == 0) continue;
        if (line_links[i*2] != 0) continue;
        for (j = 0; j < no_of_line_points; j++) {
            if (j == i) continue;
            if (line_links[j*2+1]-1 == i) {
                line_links[i*2] = j+1;
            }
        }
    }

    return no_of_links;
}

n_c_int show_lines(n_byte result[], n_c_int width, n_c_int height,
                   n_c_int bitsperpixel,
                   n_c_int line_points[], n_c_int no_of_line_points,
                   n_c_int line_links[], n_c_int line_width,
                   n_c_int r, n_c_int g, n_c_int b, n_byte show_junctions,
                   n_byte clear)
{
    n_c_int i, tx, ty, bx, by, index, p;

    if (clear != 0) memset(result, BACKGROUND, width*height*3*sizeof(n_byte));

    for (i = 0; i < no_of_line_points; i++) {
        tx = line_points[i*2];
        ty = line_points[i*2+1];

        for (p = NEXT_JUNCTION; p <= PREV_JUNCTION; p++) {
            if (line_links[i*2+p] == 0) {
                if (show_junctions != 0) {
                    if (p==0) {
                        draw_point(result, (n_byte4)width, (n_byte4)height,
                                   tx, ty, 5, 0, 0, 255);
                    }
                    else {
                        draw_point(result, (n_byte4)width, (n_byte4)height,
                                   tx, ty, 5, 255, 0, 255);
                    }
                }
                continue;
            }
            index = line_links[i*2+p]-1;
            bx = line_points[index*2];
            by = line_points[index*2+1];

            draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                      tx, ty, bx, by, line_width, r, g, b);
        }
    }
    return 0;
}

n_c_int show_lines_with_width(n_byte result[], n_c_int width, n_c_int height,
                              n_c_int bitsperpixel,
                              n_c_int line_points[], n_c_int no_of_line_points,
                              n_c_int line_links[], n_c_int line_widths[],
                              n_c_int r, n_c_int g, n_c_int b)
{
    n_c_int i, tx, ty, bx, by, index, p;

    memset(result, BACKGROUND, width*height*3*sizeof(n_byte));

    for (i = 0; i < no_of_line_points; i++) {
        tx = line_points[i*2];
        ty = line_points[i*2+1];

        for (p = NEXT_JUNCTION; p <= PREV_JUNCTION; p++) {
            if (line_links[i*2+p] == 0) break;
            index = line_links[i*2+p]-1;
            bx = line_points[index*2];
            by = line_points[index*2+1];

            draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                      tx, ty, bx, by, line_widths[i], r, g, b);
        }
    }
    return 0;
}

n_c_int show_six_lines(n_byte result[], n_c_int width, n_c_int height,
                       n_c_int bitsperpixel,
                       n_c_int line_points1[], n_c_int line_widths1[],
                       n_c_int no_of_line_points1,
                       n_c_int line_links1[],
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
                       n_byte background_r, n_byte background_g, n_byte background_b)
{
    n_c_int tx, ty, bx, by, index, p;
    n_int i, n;
    n_int pixels = (n_int)width*(n_int)height;

    if (bitsperpixel != 24) return 1;

    for (i = 0; i < pixels; i++) {
        n = i*3;
        result[n] = background_r;
        result[n+1] = background_g;
        result[n+2] = background_b;
    }

    for (i = 0; i < (n_int)no_of_line_points1; i++) {
        tx = line_points1[i*2];
        ty = line_points1[i*2+1];

        for (p = NEXT_JUNCTION; p <= PREV_JUNCTION; p++) {
            if (line_links1[i*2+p] == 0) continue;
            index = line_links1[i*2+p]-1;
            bx = line_points1[index*2];
            by = line_points1[index*2+1];

            draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                      tx, ty, bx, by, line_widths1[i], r1, g1, b1);
        }
    }

    for (i = 0; i < (n_int)no_of_line_points2; i++) {
        tx = line_points2[i*2];
        ty = line_points2[i*2+1];

        for (p = NEXT_JUNCTION; p <= PREV_JUNCTION; p++) {
            if (line_links2[i*2+p] == 0) continue;
            index = line_links2[i*2+p]-1;
            bx = line_points2[index*2];
            by = line_points2[index*2+1];

            draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                      tx, ty, bx, by, line_width2, r2, g2, b2);
        }
    }

    for (i = 0; i < (n_int)no_of_line_points3; i++) {
        tx = line_points3[i*2];
        ty = line_points3[i*2+1];

        for (p = NEXT_JUNCTION; p <= PREV_JUNCTION; p++) {
            if (line_links3[i*2+p] == 0) continue;
            index = line_links3[i*2+p]-1;
            bx = line_points3[index*2];
            by = line_points3[index*2+1];

            draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                      tx, ty, bx, by, line_width2, r3, g3, b3);
        }
    }

    for (i = 0; i < (n_int)no_of_line_points4; i++) {
        tx = line_points4[i*2];
        ty = line_points4[i*2+1];

        for (p = NEXT_JUNCTION; p <= PREV_JUNCTION; p++) {
            if (line_links4[i*2+p] == 0) continue;
            index = line_links4[i*2+p]-1;
            bx = line_points4[index*2];
            by = line_points4[index*2+1];

            draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                      tx, ty, bx, by, line_width2, r4, g4, b4);
        }
    }

    for (i = 0; i < (n_int)no_of_line_points5; i++) {
        tx = line_points5[i*2];
        ty = line_points5[i*2+1];

        for (p = NEXT_JUNCTION; p <= PREV_JUNCTION; p++) {
            if (line_links5[i*2+p] == 0) continue;
            index = line_links5[i*2+p]-1;
            bx = line_points5[index*2];
            by = line_points5[index*2+1];

            draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                      tx, ty, bx, by, line_width2, r5, g5, b5);
        }
    }

    for (i = 0; i < (n_int)no_of_line_points6; i++) {
        tx = line_points6[i*2];
        ty = line_points6[i*2+1];

        for (p = NEXT_JUNCTION; p <= PREV_JUNCTION; p++) {
            if (line_links6[i*2+p] == 0) continue;
            index = line_links6[i*2+p]-1;
            bx = line_points6[index*2];
            by = line_points6[index*2+1];

            draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                      tx, ty, bx, by, line_width2, r6, g6, b6);
        }
    }

    for (i = 0; i < (n_int)no_of_junctions; i++) {
        tx = junction_points[i*2];
        ty = junction_points[i*2+1];
        draw_point(result, (n_byte4)width, (n_byte4)height,
                   tx, ty, 6, 0, 0, 255);
        draw_point(result, (n_byte4)width, (n_byte4)height,
                   tx, ty, 3, BACKGROUND, BACKGROUND, BACKGROUND);
    }
    return 0;
}

/* returns the intersection point of two lines */
n_c_int line_intersection(n_c_int line1_tx, n_c_int line1_ty,
                          n_c_int line1_bx, n_c_int line1_by,
                          n_c_int line2_tx, n_c_int line2_ty,
                          n_c_int line2_bx, n_c_int line2_by,
                          n_c_int * ix, n_c_int * iy)
{
    double a1, b1, c1, a2, b2, c2, det_inv, m1, m2, dm;
    n_c_int tx=0, ty=0, bx=0, by=0;

    if ((line1_bx - line1_tx) != 0)
        m1 = (double)(line1_by - line1_ty) / (double)(line1_bx - line1_tx);
    else
        m1 = (double)1e+10;

    if ((line2_bx - line2_tx) != 0)
        m2 = (double)(line2_by - line2_ty) / (double)(line2_bx - line2_tx);
    else
        m2 = (double)1e+10;

    dm = (double)(m1 - m2);
    if (dm < 0) dm = -dm;
    if (dm > 0.000001f)
    {
        a1 = m1;
        a2 = m2;
        b1 = -1;
        b2 = -1;

        c1 = ((double)line1_ty - m1 * (double)line1_tx);
        c2 = ((double)line2_ty - m2 * (double)line2_tx);

        det_inv = 1 / (a1 * b2 - a2 * b1);

        *ix = (n_c_int)((b1 * c2 - b2 * c1) * det_inv);
        *iy = (n_c_int)((a2 * c1 - a1 * c2) * det_inv);

        if (line1_tx < line1_bx) {
            tx = line1_tx;
            bx = line1_bx;
        }
        else {
            tx = line1_bx;
            bx = line1_tx;
        }
        if (line1_ty < line1_by) {
            ty = line1_ty;
            by = line1_by;
        }
        else {
            ty = line1_by;
            by = line1_ty;
        }
        if ((*ix >= tx) && (*ix <= bx) && (*iy >= ty) && (*iy <= by))
        {
            if (line2_tx < line2_bx) {
                tx = line2_tx;
                bx = line2_bx;
            }
            else {
                tx = line2_bx;
                bx = line2_tx;
            }
            if (line2_ty < line2_by) {
                ty = line2_ty;
                by = line2_by;
            }
            else {
                ty = line2_by;
                by = line2_ty;
            }
            if ((*ix >= tx) && (*ix <= bx) && (*iy >= ty) && (*iy <= by))
            {
                return 1;
            }
        }
    }
    else
    {
        /* lines are parallel */
        *ix = -1;
        *iy = -1;
    }

    return 0;
}

/* returns the distance of a point from a line */
n_c_int point_dist_from_line(n_c_int x0, n_c_int y0,
                             n_c_int x1, n_c_int y1,
                             n_c_int point_x, n_c_int point_y)
{
    n_c_int perpendicular_dist = 999999;
    n_c_int dx = x1 - x0;
    n_c_int dy = y1 - y0;
    n_c_int line_length2 = dx*dx + dy*dy;

    if (line_length2 > 0)
    {
        n_c_int perp_line_x0 = point_x - dy;
        n_c_int perp_line_y0 = point_y - dx;
        n_c_int perp_line_x1 = point_x + dy;
        n_c_int perp_line_y1 = point_y + dx;
        n_c_int ix = -1;
        n_c_int iy = -1;

        line_intersection(x0, y0, x1, y1,
                          perp_line_x0, perp_line_y0, perp_line_x1, perp_line_y1,
                          &ix, &iy);

        if (!((ix == -1) && (iy == -1)))
        {
            dx = ix - point_x;
            dy = iy - point_y;
            perpendicular_dist = (n_c_int)sqrt((dx * dx) + (dy * dy));
        }
    }

    return perpendicular_dist;
}

/* is the source point linked to the destination point? */
static n_c_int linked_to(n_c_int line_points[], n_c_int line_links[],
                         n_c_int source_index, n_c_int dest_index,
                         n_byte direction)
{
    n_c_int index;

    if (line_links[source_index*2+(n_c_int)direction] == 0) return 0;
    index = line_links[source_index*2+(n_c_int)direction]-1;
    if (index == dest_index) {
        return 1;
    }
    return 0;
}

/* is the given point a junction? */
static n_byte is_junction(n_c_int line_points[], n_c_int line_links[],
                          n_c_int no_of_line_points, n_c_int index)
{
    n_c_int j, link_ctr=0, p=0;

    for (j = 0; j < no_of_line_points; j++) {
        if (j == index) continue;
        for (p = NEXT_JUNCTION; p <= PREV_JUNCTION; p++) {
            if (linked_to(line_points, line_links, j, index, p) == 1) {
                link_ctr++;
                if (link_ctr > 2) {
                    return (n_byte)1;
                }
            }
        }
    }
    return (n_byte)0;
}

static n_c_int add_junction(n_c_int junction_points[], n_c_int max_junctions,
                            n_c_int no_of_junction_points,
                            n_c_int x, n_c_int y)
{
    n_c_int i;

    if (no_of_junction_points >= max_junctions) {
        return no_of_junction_points;
    }

    for (i = 0; i < no_of_junction_points; i++) {
        if ((abs(junction_points[i*2] - x) < 5) && (abs(junction_points[i*2+1] - y) < 5)) {
            return no_of_junction_points;
        }
    }

    junction_points[no_of_junction_points*2] = x;
    junction_points[no_of_junction_points*2+1] = y;
    no_of_junction_points++;
    return no_of_junction_points;
}


n_c_int detect_junctions(n_c_int line_points1[], n_c_int no_of_line_points1,
                         n_c_int line_links1[],
                         n_c_int line_points2[], n_c_int no_of_line_points2,
                         n_c_int line_links2[],
                         n_c_int junction_points[], n_c_int max_junctions,
                         n_c_int join_ends_radius)
{
    n_c_int i, j, p1, p2, index, no_of_junction_points = 0, link_ctr;
    n_c_int line1_tx, line1_ty, line1_bx, line1_by, dx, dy, dist, max_dist, join_index;
    n_c_int line2_tx, line2_ty, line2_bx, line2_by, ix=0, iy=0;
    n_c_int join_ends_radius2 = join_ends_radius * join_ends_radius;

    for (i = 0; i < no_of_line_points1; i++) {
        if (is_junction(line_points1, line_links1, no_of_line_points1, i) == 1) {
            no_of_junction_points =
                add_junction(junction_points, max_junctions,
                             no_of_junction_points, line_points1[i*2], line_points1[i*2+1]);
            if (no_of_junction_points >= max_junctions) {
                break;
            }
        }
    }

    for (i = 0; i < no_of_line_points2; i++) {
        if (is_junction(line_points2, line_links2, no_of_line_points2, i) == 1) {
            no_of_junction_points =
                add_junction(junction_points, max_junctions,
                             no_of_junction_points,
                             line_points2[i*2], line_points2[i*2+1]);
            if (no_of_junction_points >= max_junctions) {
                break;
            }
        }
    }

    for (i = 0; i < no_of_line_points2; i++) {
        /* detect intersections with the first set of points */
        line1_tx = line_points2[i*2];
        line1_ty = line_points2[i*2+1];
        link_ctr = 0;
        for (p1 = NEXT_JUNCTION; p1 <= PREV_JUNCTION; p1++) {
            if (line_links2[i*2+p1] == 0) continue;
            index = line_links2[i*2+p1]-1;
            line1_bx = line_points2[index*2];
            line1_by = line_points2[index*2+1];
            link_ctr++;

            for (j = 0; j < no_of_line_points1; j++) {
                line2_tx = line_points1[j*2];
                line2_ty = line_points1[j*2+1];

                for (p2 = NEXT_JUNCTION; p2 <= PREV_JUNCTION; p2++) {
                    if (line_links1[j*2+p2] == 0) continue;
                    index = line_links1[j*2+p2]-1;
                    line2_bx = line_points1[index*2];
                    line2_by = line_points1[index*2+1];

                    if (line_intersection(line1_tx, line1_ty,
                                          line1_bx, line1_by,
                                          line2_tx, line2_ty,
                                          line2_bx, line2_by,
                                          &ix, &iy) == 1) {
                        no_of_junction_points =
                            add_junction(junction_points, max_junctions,
                                         no_of_junction_points, ix, iy);
                        if (no_of_junction_points >= max_junctions) {
                            return no_of_junction_points;
                        }
                    }
                }
            }
        }
        if (link_ctr < 2) {
            /* end of the line. Are there any close points in the first points list? */
            max_dist = join_ends_radius2;
            join_index = -1;
            for (j = 0; j < no_of_line_points1; j++) {
                line2_tx = line_points1[j*2];
                line2_ty = line_points1[j*2+1];
                dx = line2_tx - line1_tx;
                dy = line2_ty - line1_ty;
                dist = dx*dx + dy*dy;
                if (dist < max_dist) {
                    max_dist = dist;
                    join_index = j;
                }
            }
            if (join_index > -1) {
                line2_tx = line_points1[join_index*2];
                line2_ty = line_points1[join_index*2+1];
                line_points2[i*2] = line2_tx;
                line_points2[i*2+1] = line2_ty;
                no_of_junction_points =
                    add_junction(junction_points, max_junctions,
                                 no_of_junction_points, line2_tx, line2_ty);
                if (no_of_junction_points >= max_junctions) {
                    return no_of_junction_points;
                }
            }
        }
    }

    return no_of_junction_points;
}

static n_c_int crossing_exists(n_c_int crossing_points[], n_c_int no_of_crossings,
                               n_c_int x, n_c_int y)
{
    n_c_int i;

    for (i = 0; i < no_of_crossings; i++) {
        if ((crossing_points[i*4] == x) && (crossing_points[i*4+1] == y)) return 1;
    }
    return 0;
}

n_c_int detect_crossings(n_c_int line_points1[], n_c_int no_of_line_points1,
                         n_c_int line_links1[],
                         n_c_int line_points2[], n_c_int no_of_line_points2,
                         n_c_int line_links2[],
                         n_c_int line_points3[], n_c_int no_of_line_points3,
                         n_c_int line_links3[],
                         n_c_int crossing_points[], n_c_int max_crossings)
{
    n_c_int i, j, p1, p2, index, no_of_crossing_points = 0;
    n_c_int line1_tx, line1_ty, line1_bx, line1_by, vec_length, dx, dy;
    n_c_int line2_tx, line2_ty, line2_bx, line2_by, ix=0, iy=0;

    for (i = 0; i < no_of_line_points3; i++) {
        /* detect intersections with the first set of points */
        line1_tx = line_points3[i*2];
        line1_ty = line_points3[i*2+1];
        for (p1 = NEXT_JUNCTION; p1 <= PREV_JUNCTION; p1++) {
            if (line_links3[i*2+p1] == 0) break;
            index = line_links3[i*2+p1]-1;

            line1_bx = line_points3[index*2];
            line1_by = line_points3[index*2+1];

            for (j = 0; j < no_of_line_points1; j++) {
                line2_tx = line_points1[j*2];
                line2_ty = line_points1[j*2+1];

                for (p2 = NEXT_JUNCTION; p2 <= PREV_JUNCTION; p2++) {
                    if (line_links1[j*2+p2] == 0) break;
                    index = line_links1[j*2+p2]-1;
                    line2_bx = line_points1[index*2];
                    line2_by = line_points1[index*2+1];

                    if (line_intersection(line1_tx, line1_ty,
                                          line1_bx, line1_by,
                                          line2_tx, line2_ty,
                                          line2_bx, line2_by,
                                          &ix, &iy) == 1) {
                        if (crossing_exists(crossing_points, no_of_crossing_points,
                                            ix, iy) == 0) {
                            dx = line2_bx - ix;
                            dy = line2_by - iy;
                            vec_length = (n_c_int)sqrt(dx*dx + dy*dy);
                            if (vec_length > 0) {
                                crossing_points[no_of_crossing_points*4] = ix;
                                crossing_points[no_of_crossing_points*4+1] = iy;
                                /* this is a vector in the range 0 - 1000,
                                   indicating direction of the crossing */
                                crossing_points[no_of_crossing_points*4+2] = dx*1000/vec_length;
                                crossing_points[no_of_crossing_points*4+3] = dy*1000/vec_length;
                                no_of_crossing_points++;
                                if (no_of_crossing_points >= max_crossings) {
                                    return no_of_crossing_points;
                                }
                            }
                        }
                    }
                }
            }

            for (j = 0; j < no_of_line_points2; j++) {
                line2_tx = line_points2[j*2];
                line2_ty = line_points2[j*2+1];

                for (p2 = NEXT_JUNCTION; p2 <= PREV_JUNCTION; p2++) {
                    if (line_links2[j*2+p2] == 0) break;
                    index = line_links2[j*2+p2]-1;
                    line2_bx = line_points2[index*2];
                    line2_by = line_points2[index*2+1];

                    if (line_intersection(line1_tx, line1_ty,
                                          line1_bx, line1_by,
                                          line2_tx, line2_ty,
                                          line2_bx, line2_by,
                                          &ix, &iy) == 1) {
                        if (crossing_exists(crossing_points, no_of_crossing_points,
                                            ix, iy) == 0) {
                            dx = line2_bx - ix;
                            dy = line2_by - iy;
                            vec_length = (n_c_int)sqrt(dx*dx + dy*dy);
                            if (vec_length > 0) {
                                crossing_points[no_of_crossing_points*4] = ix;
                                crossing_points[no_of_crossing_points*4+1] = iy;
                                /* this is a vector in the range 0 - 1000,
                                   indicating direction of the crossing */
                                crossing_points[no_of_crossing_points*4+2] = dx*1000/vec_length;
                                crossing_points[no_of_crossing_points*4+3] = dy*1000/vec_length;
                                no_of_crossing_points++;
                                if (no_of_crossing_points >= max_crossings) {
                                    return no_of_crossing_points;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return no_of_crossing_points;
}

n_c_int show_crossings(n_byte result[], n_c_int width, n_c_int height,
                       n_c_int bitsperpixel,
                       n_c_int crossing_points[], n_c_int no_of_crossings,
                       n_c_int r1, n_c_int g1, n_c_int b1,
                       n_c_int r2, n_c_int g2, n_c_int b2,
                       n_c_int crossing_size, n_c_int line_width)
{
    n_c_int i, mid_x, mid_y, vec_x, vec_y, dx, dy;

    for (i = 0; i < no_of_crossings; i++) {
        mid_x = crossing_points[i*4];
        mid_y = crossing_points[i*4+1];
        /* this is a vector in the range 0 - 1000, indicating direction of the crossing */
        vec_x = crossing_points[i*4+2];
        vec_y = crossing_points[i*4+3];
        dx = vec_x*crossing_size/2000;
        dy = vec_y*crossing_size/2000;
        draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                  mid_x + dx, mid_y + dy,
                  mid_x - dx, mid_y - dy,
                  line_width, r1, g1, b1);
        draw_line(result, (n_byte4)width, (n_byte4)height, bitsperpixel,
                  mid_x + dx, mid_y + dy,
                  mid_x - dx, mid_y - dy,
                  line_width*70/100, r2, g2, b2);
    }
    return 0;
}

n_c_int potential_roads(n_byte img[], n_c_int width, n_c_int height,
                        n_byte result[],
                        n_c_int min_width, n_c_int max_width)
{
    n_c_int x, y, xx, yy, w, n;

    memset(result, BACKGROUND, width * height * sizeof(n_byte));

    for (y = 0; y < height; y++) {
        for (x = 0; x < width - max_width; x++) {
            n = y*width + x;
            if (img[n] == BACKGROUND) continue;
            if (img[n+1] != BACKGROUND) continue;
            if (img[n+min_width] != BACKGROUND) continue;
            for (w = min_width+1; w < max_width; w++) {
                n = y*width + x + w;
                if (img[n] != BACKGROUND) {
                    for (xx = x+1; xx < x + w; xx++) {
                        n = y*width + xx;
                        result[n] = 0;
                    }
                    w = max_width;
                    break;
                }
            }
        }
    }

    for (x = 0; x < width; x++) {
        for (y = 0; y < height - max_width; y++) {
            n = y*width + x;
            if (img[n] == BACKGROUND) continue;
            if (img[n+width] != BACKGROUND) continue;
            if (img[n+(width * min_width)] != BACKGROUND) continue;
            for (w = min_width+1; w < max_width; w++) {
                n = (y+w)*width + x;
                if (img[n] != BACKGROUND) {
                    for (yy = y+1; yy < y + w; yy++) {
                        n = yy*width + x;
                        result[n] = 0;
                    }
                    w = max_width;
                    break;
                }
            }
        }
    }
    return 0;
}

n_c_int detect_line_point_widths(n_byte img[], n_c_int width, n_c_int height,
                                 n_c_int points[], n_c_int no_of_points,
                                 n_c_int point_width[], n_c_int max_width)
{
    n_c_int p, x, y, n[12], w, w2, i, ctr, no_of_widths = 0;
    n_c_int max_radius = max_width/2;

    for (p = 0; p < no_of_points; p++) {
        x = points[p*2];
        y = points[p*2+1];
        point_width[p] = 1;
        for (w = 1; w <= max_radius; w++) {
            if ((x + w >= width) || (x - w < 0)) break;
            if ((y + w >= height) || (y - w < 0)) break;
            w2 = w * 3 / 4;
            n[0] = y*width + x - w;
            n[1] = (y + w2)*width + x - w;
            n[2] = (y - w2)*width + x - w;

            n[3] = y*width + x + w;
            n[4] = (y + w2)*width + x + w;
            n[5] = (y - w2)*width + x + w;

            n[6] = (y - w)*width + x;
            n[7] = (y - w)*width + x + w2;
            n[8] = (y - w)*width + x - w2;

            n[9] = (y + w)*width + x;
            n[10] = (y + w)*width + x + w2;
            n[11] = (y + w)*width + x - w2;
            ctr = 0;
            for (i = 0; i < 12; i++) {
                if (img[n[i]] != BACKGROUND) {
                    ctr++;
                }
            }
            if (ctr < 3) {
                if (w > 1) no_of_widths++;
                point_width[p] = 1 + (w/2);
                break;
            }
        }
    }

    return no_of_widths;
}
