#include "geometry.h"
#include "../maths.h"

struct point midpoint(struct point p1, struct point p2) {
    struct point mid = {
        .x = (p1.x + p2.x)/2,
        .y = (p1.y + p2.y)/2
    };
    return mid;
}
struct point point_plus_vec(struct point fulcrum, struct vector dir) {
    struct point new_location = {
        .x = fulcrum.x + dir.x,
        .y = fulcrum.y + dir.y
    };
    return new_location;
}
struct vector vec_difference(struct point p1, struct point p2) {
    struct vector diff = {
        .x = p1.x - p2.x, 
        .y = p1.y - p2.y
    };
    return diff;
}
struct vector vec_normalize(struct vector vec) {
    double normalization_factor = max(abs(vec.x), abs(vec.y));
    vec.x /= normalization_factor;
    vec.y /= normalization_factor;
    return vec;
}
struct vector vec_orthogonal(struct vector vec) {
    struct vector ortho = {
        .x = vec.y,
        .y = (-1)*vec.x 
    };
    return ortho;
}

// Copied math from Wikipedia to check whether line segments intersect. Had something to do with determinants.
bool lines_intersect(struct point p1, struct point p2, struct point p3, struct point p4) {
    double t_num = (p1.x - p3.x)*(p3.y - p4.y) - (p1.y - p3.y)*(p3.x - p4.x);
    double t_den = (p1.x - p2.x)*(p3.y - p4.y) - (p1.y - p2.y)*(p3.x - p4.x);

    double u_num = (p1.x - p3.x)*(p1.y - p2.y) - (p1.x - p2.x)*(p1.y - p3.y);
    double u_den = (p1.x - p2.x)*(p3.y - p4.y) - (p3.x - p4.x)*(p1.y - p2.y);

    if(!(t_num >= 0 && t_den > 0) && !(t_num <= 0 && t_den < 0)) return false;
    if(!(u_num >= 0 && u_den > 0) && !(u_num <= 0 && u_den < 0)) return false;

    if(abs(t_num/t_den) > 1) return false;
    if(abs(u_num/u_den) > 1) return false;
    return true;
}