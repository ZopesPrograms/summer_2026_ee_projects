#include "rotate_points.h"
#include "point.h"
#include "../maths.h"

void rotate_points(struct point *points, int num_points, double theta) {
    for(int i = 0; i < num_points; i++) {
        struct point to_rotate = points[i];
        points[i].x = (cosine(theta) * to_rotate.x) + (sine(theta) * to_rotate.y);
        points[i].y = (-1*sine(theta) * to_rotate.x) + (cosine(theta) * to_rotate.y);
    }
}

void rotate_template_points(struct point *points_dst, struct point *points_template, int num_points, double theta) {
    for(int i = 0; i < num_points; i++) {
        struct point to_rotate = points_template[i];
        points_dst[i].x = cosine(theta) * to_rotate.x - sine(theta) * to_rotate.y;
        points_dst[i].y = sine(theta) * to_rotate.x + cosine(theta) * to_rotate.y;
    }
}

