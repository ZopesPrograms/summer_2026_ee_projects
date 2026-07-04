#include "rotate_vector.h"
#include "geometry.h"
#include "../maths.h"

void rotate_vector(struct vector *vec, double theta) {
    struct vector to_rotate = *vec;
    vec->x = cosine(theta) * to_rotate.x - sine(theta) * to_rotate.y;
    vec->y = sine(theta) * to_rotate.x + cosine(theta) * to_rotate.y;
}