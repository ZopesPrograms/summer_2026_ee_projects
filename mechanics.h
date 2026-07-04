#ifndef MECHANICS_H
#define MECHANICS_H

#include "constants.h"
#include "graphics/point.h"
#include <stdbool.h>

struct mechanics {
    double x;
    double y;
    double vx;
    double vy;
    double ax;
    double ay;
    double rotation; //heading/bearing from North. pi/2 is East
};

/* Base mechanics updater (updates x and y according to vx, vy, assumes inertia).
* @param wrap_around if true (like for rocket and saucer), spawns at the left side if gone off the right side
*/
void update_mechanics(struct mechanics* mech, bool wrap_around);

// Template function for checking whether two objects are colliding, using their locations (as stored in obj1 and obj2) and their points (points_obj1 and points_obj2).
bool are_colliding(struct mechanics obj1, struct mechanics obj2, struct point *points_obj1, struct point *points_obj2, int num_points_obj1, int num_points_obj2);

struct point mechanics_to_point(struct mechanics mech);

#endif

