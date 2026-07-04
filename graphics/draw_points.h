#ifndef DRAW_POINTS_H
#define DRAW_POINTS_H

#include "gl.h"
#include "draw_line.h"
#include "point.h"

void draw_points(struct point *points, int num_points, int x_offset, int y_offset, color_t c);

#endif
