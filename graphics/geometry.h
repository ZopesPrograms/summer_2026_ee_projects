#pragma once
#include "point.h"
#include <stdbool.h>

struct vector {
    double x;
    double y;
};


struct point midpoint(struct point p1, struct point p2); // returns midpoint between two points
struct point point_plus_vec(struct point fulcrum, struct vector dir); // returns point + vector motion

struct vector vec_difference(struct point p1, struct point p2); // takes vector defining difference of two points
struct vector vec_normalize(struct vector vec);  // returns normalized vector
struct vector vec_orthogonal(struct vector vec); // returns orthogonal vector

// Returns truth value of whether line segments (p1, p2) and (p3, p4) intersect or not
bool lines_intersect(struct point p1, struct point p2, struct point p3, struct point p4);