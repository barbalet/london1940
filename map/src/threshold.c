#include "map2json.h"
#include "toolkit.h"

n_c_int proximal_threshold(n_byte img[], n_c_int width, n_c_int height,
                           n_c_int r, n_c_int g, n_c_int b, n_c_int radius,
                           n_byte result[])
{
    n_c_int i, j, dr, dg, db, diff, max_diff;

    max_diff = radius*radius*3;

    memset(result, 255, width*height*sizeof(n_byte));

    j = width*height - 1;
    
    n_c_int show_output = j / 8;
    
    for (i = (width*height*3) - 1; i >= 2; i-=3, j--) {
        dr = r - (int)img[i];
        dg = g - (int)img[i - 1];
        db = b - (int)img[i - 2];
        diff = dr*dr + dg*dg + db*db;
        if (diff < max_diff) {
            result[j] = 0;
        }
        
        if ((j % show_output) == 0)
        {
            printf("proximal_threshold %d\n", j);
        }
        
    }
    return 0;
}

n_c_int proximal_erase(n_byte img[], n_c_int width, n_c_int height,
                       n_c_int radius, n_c_int min_coverage_percent)
{
    n_int x, y, n, xx, yy, n2, hits, max_hits;

    max_hits = (radius*2)*(radius*2);

    for (y = 0; y < height; y++) {
        n = y*width;
        for (x = 0; x < width; x++,n++) {
            if (img[n] != 0) {
                continue;
            }
            hits = 0;
            for (yy = y - radius; yy < y + radius; yy++) {
                if (yy < 0) continue;
                if (yy >= height) break;
                n2 = yy*width;
                for (xx = x - radius; xx < x + radius; xx++) {
                    if (xx < 0) continue;
                    if (xx >= width) break;
                    if (img[n2+xx] == 0) {
                        hits++;
                    }
                }
            }
            if (hits * 100 / max_hits < min_coverage_percent) {
                img[n] = BACKGROUND;
            }
        }
    }
    return 0;
}

static n_c_int proximal_fill_point(n_byte img[], n_c_int width, n_c_int height,
                                   n_c_int x, n_c_int y,
                                   n_c_int r, n_c_int g, n_c_int b,
                                   n_byte * result,
                                   n_c_int bounding_box[],
                                   n_c_int depth, n_c_int max_depth)
{
    n_int n, xx, yy, left_x, right_x;

    if (depth > max_depth) {
        return 0;
    }
    if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
        return 0;
    }
    n = y*width + x;
    if (img[n] != 0) {
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

    /* fill left */
    left_x = x;
    for (xx = x-1; xx >= 0; xx--) {
        n = y*width + xx;
        if (img[n] != 0) {
            break;
        }
        n *= 3;
        if (result[n] != BACKGROUND) {
            break;
        }
        result[n] = b;
        result[n+1] = g;
        result[n+2] = r;
        left_x = xx;

        if (xx < bounding_box[0]) bounding_box[0] = xx;
    }

    /* fill right */
    right_x = x;
    for (xx = x+1; xx < width; xx++) {
        n = y*width + xx;
        if (img[n] != 0) {
            break;
        }
        n *= 3;
        if (result[n] != BACKGROUND) {
            break;
        }
        result[n] = b;
        result[n+1] = g;
        result[n+2] = r;
        right_x = xx;

        if (xx > bounding_box[2]) bounding_box[2] = xx;
    }

    /* fill in the local area */
    for (yy = y - 1; yy <= y + 1; yy++) {
        if ((yy == y) || (yy < 0)) continue;
        if (yy >= height) continue;
        for (xx = x - 1; xx <= x + 1; xx++) {
            if (xx < 0) continue;
            if (xx >= width) break;
            if ((xx == x) && (yy == y)) {
                continue;
            }
            proximal_fill_point(img, width, height, xx, yy, r, g, b,
                                result,
                                bounding_box, depth+1, max_depth);
        }
    }

    if (y < height-1) {
        if (left_x != x) {
            proximal_fill_point(img, width, height,
                                left_x + ((x-left_x)/2), y+1, r, g, b,
                                result,
                                bounding_box, depth+1, max_depth);
            proximal_fill_point(img, width, height,
                                left_x, y+1, r, g, b,
                                result,
                                bounding_box, depth+1, max_depth);
        }
        if (right_x != x) {
            proximal_fill_point(img, width, height,
                                x + ((right_x-x)/2), y+1, r, g, b,
                                result,
                                bounding_box, depth+1, max_depth);
            proximal_fill_point(img, width, height,
                                right_x, y+1, r, g, b,
                                result,
                                bounding_box, depth+1, max_depth);
        }
    }
    if (y > 0) {
        if (left_x != x) {
            proximal_fill_point(img, width, height,
                                left_x + ((x-left_x)/2), y-1, r, g, b,
                                result,
                                bounding_box, depth+1, max_depth);
            proximal_fill_point(img, width, height,
                                left_x, y-1, r, g, b,
                                result,
                                bounding_box, depth+1, max_depth);
        }
        if (right_x != x) {
            proximal_fill_point(img, width, height,
                                x + ((right_x-x)/2), y-1, r, g, b,
                                result,
                                bounding_box, depth+1, max_depth);
            proximal_fill_point(img, width, height,
                                right_x, y-1, r, g, b,
                                result,
                                bounding_box, depth+1, max_depth);
        }
    }

    return 1;
}

static void proximal_clear_fill(n_byte img[],
                                n_byte result[], n_c_int width, n_c_int height,
                                n_c_int r, n_c_int g, n_c_int b,
                                n_c_int background)
{
    n_int n, i=0;

    for (n = 0; n < width*height*3; n += 3, i++) {
        if ((result[n] == b) && (result[n+1] == g) && (result[n+2] == r)) {
            result[n] = background;
            result[n+1] = background;
            result[n+2] = background;
            img[i] = background;
        }
    }
}

/* reduce, reuse, recycle */
static n_c_int polygon_reduce(n_c_int * polygon_points, n_c_int no_of_points,
                              n_c_int max_variance,
                              n_c_int width, n_c_int height)
{
    n_c_int p, p2, start_x, start_y, mid_x, mid_y, end_x, end_y;
    n_c_int dx, dy, predicted_x, predicted_y, ddx, ddy;
    n_c_int new_no_of_points;
    n_c_int points_removed = 1;

    while (points_removed != 0) {
        new_no_of_points = no_of_points;
        points_removed = 0;
        for (p = no_of_points-1; p >= 2; p-=2) {
            start_x = polygon_points[p*2];
            start_y = polygon_points[p*2+1];
            mid_x = polygon_points[(p-1)*2];
            mid_y = polygon_points[(p-1)*2+1];
            end_x = polygon_points[(p-2)*2];
            end_y = polygon_points[(p-2)*2+1];
            dx = end_x - start_x;
            dy = end_y - start_y;
            predicted_x = start_x + (dx/2);
            predicted_y = start_y + (dy/2);
            ddx = mid_x - predicted_x;
            ddy = mid_y - predicted_y;
            if (ddx*ddx + ddy*ddy > max_variance) {
                continue;
            }
            for (p2 = p-1; p2 < new_no_of_points-1; p2++) {
                polygon_points[p2*2] = polygon_points[(p2+1)*2];
                polygon_points[p2*2+1] = polygon_points[(p2+1)*2+1];
            }
            new_no_of_points--;
            points_removed = 1;
        }
        no_of_points = new_no_of_points;
    }

    /* ensure that points are inside of the image */
    for (p = 0; p < no_of_points; p++) {
        if (polygon_points[p*2] < 0) polygon_points[p*2] = 0;
        if (polygon_points[p*2] >= width) polygon_points[p*2] = width-1;
        if (polygon_points[p*2+1] < 0) polygon_points[p*2+1] = 0;
        if (polygon_points[p*2+1] >= height) polygon_points[p*2+1] = height-1;
    }
    return no_of_points;
}

static n_byte point_is_color(n_int n, n_byte img[],
			     n_c_int r, n_c_int g, n_c_int b)
{
  return ((img[n] == b) && (img[n+1] == g) && (img[n+2] == r));
}

static n_c_int polygon_perimeter_points_from_edges(n_byte img[],
        n_c_int tx, n_c_int ty,
        n_c_int width, n_c_int height,
        n_c_int bb_width, n_c_int bb_height,
        n_c_int polygon_points[],
        n_c_int max_polygon_points,
        n_c_int r, n_c_int g, n_c_int b,
        n_c_int max_variance)
{
    n_int max_dist = 30;
    n_int max_dist2 = max_dist*max_dist;
    n_byte filled, prev_filled;
    n_int n, dx, dy, dist2, min_dist2, y_start;
    n_c_int x, y, p1, p2, no_of_points=0;
    n_c_int bx = tx + bb_width;
    n_c_int by = ty + bb_height;
    n_c_int prev_p1, segment_length, max_segment_start_index, max_segment_length;
    n_c_int found_index;

    if (tx < 0) tx = 0;
    if (ty < 0) ty = 0;
    if (bx >= width) bx = width-1;
    if (by >= height) by = height-1;

    if (bb_width > bb_height) {
      if (max_dist > bb_width/4) {
	max_dist = bb_width/4;
	max_dist2 = max_dist*max_dist;
      }
    }
    else {
      if (max_dist > bb_height/4) {
	max_dist = bb_height/4;
	max_dist2 = max_dist*max_dist;
      }
    }

    /* horizontal edges */
    for (y = ty; y < by; y++) {
      y_start = (n_int)y*(n_int)width;
      filled = 0;
      prev_filled = 0;
      for (x = tx; x <= bx; x++) {
	n = (y_start + (n_int)x)*3;
	if (point_is_color(n, img, r, g, b)) {
	  filled = 1;
	}
	else {
	  filled = 0;
	}
	if (((filled == 1) && (prev_filled == 0)) ||
	    ((filled == 0) && (prev_filled == 1)) ||
	    ((filled == 1) && (x == bx))) {
	  if (no_of_points >= max_polygon_points) {
	    y = by+1;
	    break;
	  }
	  polygon_points[no_of_points*2] = x;
	  polygon_points[no_of_points*2+1] = y;
	  no_of_points++;	    
	}
	prev_filled = filled;
      }
    }

    /* vertical edges */
    for (x = tx; x < bx; x++) {
      filled = 0;
      prev_filled = 0;
      for (y = ty; y <= by; y++) {
	n = ((n_int)y*(n_int)width + (n_int)x)*3;
	if (point_is_color(n, img, r, g, b)) {
	  filled = 1;
	}
	else {
	  filled = 0;
	}
	if (((filled == 1) && (prev_filled == 0)) ||
	    ((filled == 0) && (prev_filled == 1)) ||
	    ((filled == 1) && (y == by))) {
	  if (no_of_points >= max_polygon_points) {
	    x = bx+1;
	    break;
	  }
	  polygon_points[no_of_points*2] = x;
	  polygon_points[no_of_points*2+1] = y;
	  no_of_points++;	    
	}
	prev_filled = filled;	
      }
    }

    /* topological sort of edge points */
    prev_p1 = 0;
    max_segment_start_index = 0;
    max_segment_length = 0;
    for (p1 = 0; p1 < no_of_points-1; p1++) {
      x = polygon_points[p1*2];
      y = polygon_points[p1*2+1];
      /* get the index of the closest point to x,y */
      min_dist2 = -1;
      found_index = -1;
      for (p2 = p1+1; p2 < no_of_points; p2++) {
	dx = (n_int)(polygon_points[p2*2] - x);
	dy = (n_int)(polygon_points[p2*2+1] - y);
	dist2 = dx*dx + dy*dy;
	if ((min_dist2 == -1) || (dist2 < min_dist2)) {
	  min_dist2 = dist2;
	  found_index = p2;
	}
      }

      if (found_index == -1) continue;
      if (min_dist2 > max_dist2) {
	segment_length = (p1-1) - prev_p1;
	if (segment_length > max_segment_length) {
	  max_segment_start_index = prev_p1;
	  max_segment_length = segment_length;
	}
	prev_p1 = p1;
      }
      if (found_index == p1+1) continue;      
      /* swap points */
      x = polygon_points[(p1+1)*2];
      y = polygon_points[(p1+1)*2+1];
      polygon_points[(p1+1)*2] = polygon_points[found_index*2];
      polygon_points[(p1+1)*2+1] = polygon_points[found_index*2+1];
      polygon_points[found_index*2] = x;
      polygon_points[found_index*2+1] = y;
    }

    /* get the longest segment */
    if (max_segment_length > 0) {
      for (p2 = max_segment_start_index; p2 < max_segment_start_index + max_segment_length; p2++) {
	polygon_points[(p2-max_segment_start_index)*2] = polygon_points[p2*2];
	polygon_points[(p2-max_segment_start_index)*2+1] = polygon_points[p2*2+1];
      }
      no_of_points = max_segment_length;
    }
    
    /* reduce the number of polygon points */
    no_of_points = polygon_reduce(polygon_points, no_of_points, max_variance,
				  width, height);

    return no_of_points;
}

/*
  Detects an interior polygon inside of a larger polygon
 */
static n_c_int polygon_detect_interior_points(n_byte img[],
        n_c_int tx, n_c_int ty, n_c_int width, n_c_int height,
        n_c_int bb_width, n_c_int bb_height,
        n_c_int * polygon_points,
        n_c_int max_polygon_points,
        n_c_int r, n_c_int g, n_c_int b,
        n_c_int max_variance)
{
    n_int x, y, no_of_points=0, n;
    n_c_int x_step, y_step, state;
    n_c_int top_x=0, bottom_x=0, interior_tx=0, interior_bx=0;
    n_c_int top_y=0, bottom_y=0, interior_ty=0, interior_by=0;


    if (bb_width > bb_height) {
        /* horizontally oriented bounding box */
        x_step = 1 + (bb_width / 42);
        if (x_step < 2) x_step = 2;
        /* top down left to right */
        for (x = tx; x < tx + bb_width; x += x_step) {
            state = 0;
            top_y=0;
            interior_ty = 0;
            interior_by = 0;
            for (y = ty; y < ty + bb_height; y++) {
                n = (y*width + x)*3;
                if ((img[n] == b) && (img[n+1] == g) && (img[n+2] == r)) {
                    switch(state) {
                    case 0: {
                        /* start of top of larger polygon */
                        top_y = y;
                        state = 1;
                        break;
                    }
                    case 2: {
                        /* bottom of interior */
                        state = 3;
                        interior_by = y;
                        break;
                    }
                    }
                }
                else {
                    if (state == 1) {
                        /* within interior */
                        interior_ty = y;
                        state = 2;
                    }
                }
            }
            if (state == 3) {
                if ((interior_ty - top_y > 1) &&
                        (interior_by - interior_ty > 1)) {
                    if (no_of_points < max_polygon_points) {
                        polygon_points[no_of_points*2] = x;
                        polygon_points[no_of_points*2+1] = interior_ty;
                        no_of_points++;
                    }
                }
            }
        }

        /* bottom up right to left */
        for (x = tx + bb_width - 1; x >= tx; x -= x_step) {
            state = 0;
            bottom_y=0;
            interior_ty = 0;
            interior_by = 0;
            for (y = ty + bb_height - 1; y > ty; y--) {
                n = (y*width + x)*3;
                if ((img[n] == b) && (img[n+1] == g) && (img[n+2] == r)) {
                    switch(state) {
                    case 0: {
                        /* start of top of larger polygon */
                        bottom_y = y;
                        state = 1;
                        break;
                    }
                    case 2: {
                        /* bottom of interior */
                        state = 3;
                        interior_by = y;
                        break;
                    }
                    }
                }
                else {
                    if (state == 1) {
                        /* within interior */
                        interior_ty = y;
                        state = 2;
                    }
                }
            }
            if (state == 3) {
                if ((bottom_y - interior_ty > 1) &&
                        (interior_ty - interior_by > 1)) {
                    if (no_of_points < max_polygon_points) {
                        polygon_points[no_of_points*2] = x;
                        polygon_points[no_of_points*2+1] = interior_ty;
                        no_of_points++;
                    }
                }
            }
        }
    }
    else {
        /* vertical bounding box orientation */
        y_step = 1 + (bb_height / 42);
        if (y_step < 2) y_step = 2;

        /* left to right top down */
        for (y = ty; y < ty + bb_height; y += y_step) {
            state = 0;
            top_x=0;
            interior_tx = 0;
            interior_bx = 0;
            for (x = tx; x < tx + bb_width; x++) {
                n = (y*width + x)*3;
                if ((img[n] == b) && (img[n+1] == g) && (img[n+2] == r)) {
                    switch(state) {
                    case 0: {
                        /* start of top of larger polygon */
                        top_x = x;
                        state = 1;
                        break;
                    }
                    case 2: {
                        /* bottom of interior */
                        state = 3;
                        interior_bx = x;
                        break;
                    }
                    }
                }
                else {
                    if (state == 1) {
                        /* within interior */
                        interior_tx = x;
                        state = 2;
                    }
                }
            }
            if (state == 3) {
                if ((interior_tx - top_x > 1) &&
                        (interior_bx - interior_tx > 1)) {
                    if (no_of_points < max_polygon_points) {
                        polygon_points[no_of_points*2] = interior_tx;
                        polygon_points[no_of_points*2+1] = y;
                        no_of_points++;
                    }
                }
            }
        }

        /* right to left bottom up */
        for (y = ty + bb_height - 1; y > ty; y -= y_step) {
            state = 0;
            bottom_x=0;
            interior_tx = 0;
            interior_bx = 0;
            for (x = tx + bb_width - 1; x >= tx; x--) {
                n = (y*width + x)*3;
                if ((img[n] == b) && (img[n+1] == g) && (img[n+2] == r)) {
                    switch(state) {
                    case 0: {
                        /* start of top of larger polygon */
                        bottom_x = x;
                        state = 1;
                        break;
                    }
                    case 2: {
                        /* bottom of interior */
                        state = 3;
                        interior_bx = x;
                        break;
                    }
                    }
                }
                else {
                    if (state == 1) {
                        /* within interior */
                        interior_tx = x;
                        state = 2;
                    }
                }
            }
            if (state == 3) {
                if ((bottom_x - interior_tx > 1) &&
                        (interior_tx - interior_bx > 1)) {
                    if (no_of_points < max_polygon_points) {
                        polygon_points[no_of_points*2] = interior_tx;
                        polygon_points[no_of_points*2+1] = y;
                        no_of_points++;
                    }
                }
            }
        }
    }

    /* reduce the number of interior polygon points */
    no_of_points = polygon_reduce(polygon_points, no_of_points, max_variance,
                                  width, height);

    return no_of_points;
}

n_c_int proximal_fill(n_byte img[], n_c_int width, n_c_int height,
                      n_byte result[],
                      n_c_int min_width, n_c_int min_height,
                      n_c_int max_width, n_c_int max_height,
                      n_c_int max_variance,
                      n_c_int polygon_id[],
                      n_c_int polygon_vertices[],
                      n_c_int polygons[],
                      n_c_int max_total_polygon_points,
                      n_c_int detect_interior)
{
    n_c_int x, y, r, g, b, bb_width, bb_height, v, xx, yy, n;
    n_c_int bounding_box[4];
    n_c_int polygon_points[MAX_POLYGON_POINTS*2], polygon_no_of_points;
    n_c_int polygon_count = 0;
    n_c_int polygon_coords_total = 0;
    n_c_int curr_polygon_id = 0;

    memset(result, BACKGROUND, width*height*3*sizeof(n_byte));

    r = (n_byte)(rand()%(BACKGROUND-1));
    g = (n_byte)(rand()%(BACKGROUND-1));
    b = (n_byte)(rand()%(BACKGROUND-1));

    n_c_int show_output = height / 8;
    
    for (y = 0; y < height; y++) {
        
        if (( y % show_output) == 0) {
            printf("proximal_fill %d %d\n", y, height);
        }
        
        for (x = 0; x < width; x++) {
            bounding_box[0] = x;
            bounding_box[1] = y;
            bounding_box[2] = x;
            bounding_box[3] = y;
            if (proximal_fill_point(img, width, height,
                                    x, y, r, g, b,
                                    result,
                                    bounding_box, 0,
                                    MAX_RECURSION_DEPTH) != 0) {
                bb_width = bounding_box[2] - bounding_box[0];
                bb_height = bounding_box[3] - bounding_box[1];
                if ((bb_width > max_width) || (bb_height > max_height) ||
                        (bb_width < min_width) || (bb_height < min_height)) {
                    proximal_clear_fill(img, result, width, height, r, g, b, BACKGROUND);
                }
                else {
                    polygon_no_of_points = \
                                           polygon_perimeter_points_from_edges(result,
                                                   bounding_box[0],
                                                   bounding_box[1],
                                                   width, height,
                                                   bb_width, bb_height,
                                                   polygon_points,
                                                   MAX_POLYGON_POINTS,
                                                   r, g, b,
                                                   max_variance);
                    if ((polygon_no_of_points > 2) &&
                            (polygon_coords_total + polygon_no_of_points < max_total_polygon_points)) {
                        for (v = 0; v < polygon_no_of_points; v++) {
                            /* show vertices */
                            xx = polygon_points[v*2];
                            yy = polygon_points[v*2+1];
                            n = (yy*width + xx)*3;
                            result[n] = 0;
                            result[n+1] = 0;
                            result[n+2] = 0;
                            /* store vertices in polygons array */
                            polygons[polygon_coords_total*2] = xx;
                            polygons[polygon_coords_total*2+1] = yy;
                            polygon_coords_total++;
                        }
                        polygon_id[polygon_count] = curr_polygon_id;
                        polygon_vertices[polygon_count++] = polygon_no_of_points;
                    }

                    if (detect_interior != 0) {
                        /* interior polygon */
                        polygon_no_of_points = \
                                               polygon_detect_interior_points(result,
                                                       bounding_box[0],
                                                       bounding_box[1],
                                                       width, height,
                                                       bb_width, bb_height,
                                                       polygon_points,
                                                       MAX_POLYGON_POINTS,
                                                       r, g, b,
                                                       max_variance);
                        if ((polygon_no_of_points > 2) &&
                                (polygon_coords_total + polygon_no_of_points < max_total_polygon_points)) {
                            for (v = 0; v < polygon_no_of_points; v++) {
                                /* show vertices */
                                xx = polygon_points[v*2];
                                yy = polygon_points[v*2+1];
                                n = (yy*width + xx)*3;
                                result[n] = 0;
                                result[n+1] = 0;
                                result[n+2] = 0;
                                /* store vertices in polygons array */
                                polygons[polygon_coords_total*2] = xx;
                                polygons[polygon_coords_total*2+1] = yy;
                                polygon_coords_total++;
                            }
                            polygon_id[polygon_count] = curr_polygon_id;
                            polygon_vertices[polygon_count++] = polygon_no_of_points;
                        }
                    }
                    curr_polygon_id++;
                }
                r = (n_byte)(rand()%254);
                g = (n_byte)(rand()%254);
                b = (n_byte)(rand()%254);
            }
        }
    }
    return polygon_count;
}

n_c_int show_polygons(n_byte img[], n_c_int width, n_c_int height,
                      n_c_int no_of_polygons,
                      n_c_int polygon_id[],
                      n_c_int polygon_vertices[],
                      n_c_int polygons[])
{
    n_c_int p, v, tx, ty, bx=0, by=0, r, g, b, start_x=0, start_y=0, n;
    n_c_int vertex_index = 0;
    n_c_int line_width = 1;

    memset(img, BACKGROUND, width*height*3*sizeof(n_byte));

    r = (n_byte)(rand()%254);
    g = (n_byte)(rand()%254);
    b = (n_byte)(rand()%254);
    for (p = 0; p < no_of_polygons; p++) {
        if (p > 0) {
            if (polygon_id[p] != polygon_id[p-1]) {
                r = (n_byte)(rand()%254);
                g = (n_byte)(rand()%254);
                b = (n_byte)(rand()%254);
            }
        }
        for (v = 0; v < polygon_vertices[p]; v++, vertex_index++) {
            if (v < polygon_vertices[p] - 1) {
                tx = polygons[vertex_index*2];
                ty = polygons[vertex_index*2+1];
                bx = polygons[(vertex_index+1)*2];
                by = polygons[(vertex_index+1)*2+1];
                if (v == 0) {
                    /* position of the first vertex */
                    start_x = tx;
                    start_y = ty;
                }
            }
            else {
                /* last vertex links back to the first */
                tx = polygons[vertex_index*2];
                ty = polygons[vertex_index*2+1];
                bx = start_x;
                by = start_y;
            }

            draw_line(img, (n_byte4)width, (n_byte4)height, 3*8,
                      tx, ty, bx, by, line_width,
                      r, g, b);

            n = (ty*width + tx)*3;
            img[n] = 0;
            img[n+1] = 0;
            img[n+2] = 0;
        }
    }
    return 0;
}

n_c_int show_polygons_against_reference(n_byte img[], n_c_int width, n_c_int height,
                                        n_byte thresholded_ref[],
                                        n_c_int no_of_polygons,
                                        n_c_int polygon_id[],
                                        n_c_int polygon_vertices[],
                                        n_c_int polygons[],
                                        n_byte inner)
{
    n_int n, i;
    n_c_int p, v, tx, ty, bx=0, by=0, start_x=0, start_y=0;
    n_c_int vertex_index = 0;
    n_c_int line_width = 1;

    memset(img, BACKGROUND, width*height*3*sizeof(n_byte));

    for (i = (width*height) - 1; i >= 0; i--) {
        if (thresholded_ref[i] != BACKGROUND) {
            img[i*3] = 200;
            img[i*3+1] = 200;
            img[i*3+2] = 200;
        }
    }

    for (p = 0; p < no_of_polygons; p++) {
        if (p > 0) {
            if (((inner != 0) && (polygon_id[p-1] != polygon_id[p])) ||
                    ((inner == 0) && (polygon_id[p-1] == polygon_id[p]))) {
                vertex_index += polygon_vertices[p];
                continue;
            }
        }
        for (v = 0; v < polygon_vertices[p]; v++, vertex_index++) {
            if (v < polygon_vertices[p] - 1) {
                tx = polygons[vertex_index*2];
                ty = polygons[vertex_index*2+1];
                bx = polygons[(vertex_index+1)*2];
                by = polygons[(vertex_index+1)*2+1];
                if (v == 0) {
                    /* position of the first vertex */
                    start_x = tx;
                    start_y = ty;
                }
            }
            else {
                /* last vertex links back to the first */
                tx = polygons[vertex_index*2];
                ty = polygons[vertex_index*2+1];
                bx = start_x;
                by = start_y;
            }

            draw_line(img, (n_byte4)width, (n_byte4)height, 3*8,
                      tx, ty, bx, by, line_width,
                      0, 0, 255);

            n = (ty*width + tx)*3;
            img[n] = 0;
            img[n+1] = 0;
            img[n+2] = 0;
        }
    }
    return 0;
}

static n_c_int polygon_bounding_box(n_c_int no_of_vertices,
                                    n_c_int polygon[],
                                    n_c_int *tx, n_c_int *ty,
                                    n_c_int *bx, n_c_int *by)
{
    n_c_int v, x, y;

    for (v = 0; v < no_of_vertices; v++) {
        x = polygon[v*2];
        y = polygon[v*2+1];
        if (x < *tx) *tx = x;
        if (y < *ty) *ty = y;
        if (x > *bx) *bx = x;
        if (y > *by) *by = y;
    }
    return 0;
}

/* returns non-zero if the given point is in the given polygon */
n_c_int point_in_polygon(n_c_int x, n_c_int y, n_c_int points[],
                         n_c_int no_of_points)
{
    n_c_int i, j, c = 0;

    for (i = 0, j = no_of_points - 1; i < no_of_points; j = i++) {
        if (((points[i*2+1] >= y) != (points[j*2+1] >= y)) &&
                (x <= (points[j*2] - points[i*2]) * (y - points[i*2+1]) /
                 (points[j*2+1] - points[i*2+1]) + points[i*2]))
            c = 1 - c;
    }

    return c;
}

n_c_int show_polygons_filled(n_byte img[], n_c_int width, n_c_int height,
                             n_c_int no_of_polygons,
                             n_c_int polygon_vertices[],
                             n_c_int polygons[],
                             n_c_int r, n_c_int g, n_c_int b)
{
    n_int p, x, y, n;
    n_c_int tx, ty, bx, by, vertex_index = 0;

    for (p = 0; p < no_of_polygons; p++) {
        /* get the bounding box */
        tx = polygons[vertex_index*2];
        if (tx < 0) tx = 0;
        ty = polygons[vertex_index*2+1];
        if (ty < 0) ty = 0;
        bx = tx;
        if (bx >= width) bx = width - 1;
        by = ty;
        if (by >= height) by = height - 1;
        polygon_bounding_box(polygon_vertices[p], &polygons[vertex_index*2],
                             &tx, &ty, &bx, &by);
        if (tx < 0) tx = 0;
        if (ty < 0) ty = 0;
        if (bx >= width) bx = width - 1;
        if (by >= height) by = height - 1;
        for (y = (n_int)ty; y <= (n_int)by; y++) {
            for (x = (n_int)tx; x <= (n_int)bx; x++) {
                if (point_in_polygon(x, y, &polygons[vertex_index*2],
                                     polygon_vertices[p]) != 0) {
                    n = (y*width + x)*3;
                    img[n] = b;
                    img[n+1] = g;
                    img[n+2] = r;
                }
            }
        }
        vertex_index += polygon_vertices[p];
    }
    return 0;
}

/* shows the inner perimeter */
n_c_int show_polygons_empty(n_byte img[], n_c_int width, n_c_int height,
                            n_c_int no_of_polygons,
                            n_c_int polygon_vertices[],
                            n_c_int polygon_id[],
                            n_c_int polygons[],
                            n_byte r, n_byte g, n_byte b)
{
    n_int n, x, y;
    n_c_int p, tx, ty, bx, by;
    n_c_int vertex_index = polygon_vertices[0];

    for (p = 1; p < no_of_polygons; p++) {
        if (polygon_id[p] != polygon_id[p-1]) {
            vertex_index += polygon_vertices[p];
            continue;
        }
        /* get the bounding box */
        tx = polygons[vertex_index*2];
        ty = polygons[vertex_index*2+1];
        bx = tx;
        by = ty;
        polygon_bounding_box(polygon_vertices[p], &polygons[vertex_index*2],
                             &tx, &ty, &bx, &by);
        if (tx < 0) tx = 0;
        if (ty < 0) ty = 0;
        if (tx >= width) tx = width-1;
        if (ty >= height) ty = height-1;
        for (y = ty; y <= by; y++) {
            for (x = tx; x <= bx; x++) {
                if (point_in_polygon(x, y, &polygons[vertex_index*2],
                                     polygon_vertices[p]) != 0) {
                    n = (y*width + x)*3;
                    img[n] = r;
                    img[n+1] = g;
                    img[n+2] = b;
                }
            }
        }
        vertex_index += polygon_vertices[p];
    }
    return 0;
}

/* clears the background of an image for areas which are within nogo polygons */
n_c_int remove_nogo_areas(n_byte img[], n_c_int width, n_c_int height,
                          n_c_int no_of_nogo_areas,
                          n_c_int vertices[], n_c_int nogo_areas[])
{
    n_c_int p, tx = 0, ty = 0, bx = 0, by = 0, vertex_index = 0;
    n_int x, y, n;

    for (p = 0; p < no_of_nogo_areas; p++) {
        tx = width;
        ty = height;
        bx = 0;
        by = 0;
        polygon_bounding_box(vertices[p], &nogo_areas[vertex_index*2],
                             &tx, &ty, &bx, &by);
        if (tx < 0) tx = 0;
        if (ty < 0) ty = 0;
        if (bx >= width) bx = width - 1;
        if (by >= height) by = height - 1;
        for (y = ty; y <= by; y++) {
            for (x = tx; x <= bx; x++) {
                if (point_in_polygon(x, y, &nogo_areas[vertex_index*2],
                                     vertices[p]) != 0) {
                    n = y*width + x;
                    img[n] = BACKGROUND;
                }
            }
        }
        vertex_index += vertices[p];
    }
    return 0;
}

n_c_int percent_coverage(n_byte img[], n_c_int width, n_c_int height)
{
    n_c_int i, hits=0;

    for (i = (width*height) - 1; i >= 0; i--) {
        if (img[i] != BACKGROUND) {
            hits++;
        }
    }
    return hits * 100 / (width*height);
}

n_c_int get_polygons_total_area(n_c_int width, n_c_int height,
                                n_c_int no_of_polygons,
                                n_c_int vertices[], n_c_int polygons[])
{
    n_c_int p, tx = 0, ty = 0, bx = 0, by = 0;
    n_c_int x, y, vertex_index = 0, total_area = 0;

    for (p = 0; p < no_of_polygons; p++) {
        tx = width;
        ty = height;
        bx = 0;
        by = 0;
        polygon_bounding_box(vertices[p], &polygons[vertex_index*2],
                             &tx, &ty, &bx, &by);
        if (tx < 0) tx = 0;
        if (ty < 0) ty = 0;
        if (bx >= width) bx = width - 1;
        if (by >= height) by = height - 1;
        for (y = ty; y <= by; y++) {
            for (x = tx; x <= bx; x++) {
                if (point_in_polygon(x, y, &polygons[vertex_index*2],
                                     vertices[p]) != 0) {
                    total_area++;
                }
            }
        }
        vertex_index += vertices[p];
    }
    return total_area * 100 / (width*height);
}

n_c_int subtract_image(n_byte img[], n_c_int width, n_c_int height,
                       n_byte img2[], n_byte img3[],
                       n_c_int search_radius, n_c_int background_threshold)
{
    n_int x, y, n, n2, xx, yy, hits;
    n_c_int samples = (search_radius*2+1)*(search_radius*2+1);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            n = y*width + x;
            if (img[n] == BACKGROUND) continue;

            hits = 0;
            for (yy = y - search_radius; yy <= y + search_radius; yy++) {
                if (yy < 0) continue;
                if (yy >= height) break;
                for (xx = x - search_radius; xx <= x + search_radius; xx++) {
                    if (xx < 0) continue;
                    if (xx >= width) break;
                    n2 = yy*width + xx;
                    if ((img2[n2] == BACKGROUND) && (img3[n2] == BACKGROUND)) hits++;
                }
            }
            if (hits * 100 / samples > background_threshold) {
                img[n] = BACKGROUND;
            }
        }
    }
    return 0;
}


n_c_int append_polygons(n_map_polygons * dest, n_map_polygons * src)
{
    n_c_int i, v, index = dest->count;
    n_c_int total_vertices = 0, total_vertices_src = 0;

    /* total existing destination verices */
    for (i = 0; i < dest->count; i++) {
        total_vertices += dest->vertices[i];
    }

    /* for each source polygon */
    for (i = 0; i < src->count; i++) {
        if (total_vertices + src->vertices[i] >= MAX_TOTAL_POLYGON_POINTS) break;
        dest->id[index] = index;
        dest->vertices[index] = src->vertices[i];
        for (v = 0; v < src->vertices[i]; v++, total_vertices_src++, total_vertices++) {
            dest->coords[total_vertices*2] = src->coords[total_vertices_src*2];
            dest->coords[total_vertices*2+1] = src->coords[total_vertices_src*2+1];
        }
        index++;
    }
    dest->count = index;
    src->count = 0;
    return 0;
}

n_c_int remove_high_frequency(n_byte img[], n_c_int width, n_c_int height,
                              n_c_int min_width)
{
    n_c_int x, y, n, p, state, start_pos;

    for (y = 0; y < height; y++) {
        state = 0;
        for (x = 0; x < width; x++) {
            n = y*width + x;
            if (state == 0) {
                if (img[n] != BACKGROUND) {
                    state = 1;
                    start_pos = x;
                }
            }
            else {
                if (img[n] == BACKGROUND) {
                    state = 0;
                    if (x - start_pos <= min_width) {
                        for (p = start_pos; p < x; p++) {
                            n = y*width + p;
                            img[n] = BACKGROUND;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
