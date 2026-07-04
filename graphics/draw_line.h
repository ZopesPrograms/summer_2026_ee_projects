#ifndef DRAW_LINE_H
#define DRAW_LINE_H

#include "gl.h"
#include <stdbool.h>
#include "../maths.h"

typedef enum {
    X,
    Y
} axis;

void draw_line(double x1, double y1, double x2, double y2, color_t c);

#endif
