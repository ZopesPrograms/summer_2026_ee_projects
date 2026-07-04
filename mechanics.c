#include "mechanics.h"
#include "graphics/geometry.h"

void update_mechanics(struct mechanics* mech, bool wrap_around) {
    // Velocity increments by acceleration
    mech->vx += mech->ax;
    mech->vy += mech->ay;
    // Position increments by velocity
    mech->x += mech->vx;
    mech->y += mech->vy;

    // Bounds checking
    if (wrap_around) {
        if (mech->x < 0) {
            mech->x = MONITOR_WIDTH;
        }
        if (mech->x > MONITOR_WIDTH) {
            mech->x = 0;
        }
        
        if (mech->y < 0) {
            mech->y = MONITOR_HEIGHT;
        }
        if (mech->y > MONITOR_HEIGHT) {
            mech->y = 0;
        }
    }
}

bool are_colliding(struct mechanics obj1, struct mechanics obj2, struct point *points_obj1, struct point *points_obj2, int num_points_obj1, int num_points_obj2) {
    // Finds centers of points in order to offset
    double obj1_xoff = obj1.x;
    double obj1_yoff = obj1.y;

    double obj2_xoff = obj2.x;
    double obj2_yoff = obj2.y;

    // Checks if any of the lines of object 1 or object 2 intersect. If so, return true.
    for(int i = 0; i < num_points_obj1-1; i++) {
        struct point p1 = { points_obj1[i].x   + obj1_xoff, points_obj1[i].y   + obj1_yoff };
        struct point p2 = { points_obj1[i+1].x + obj1_xoff, points_obj1[i+1].y + obj1_yoff };
        for(int j = 0; j < num_points_obj2-1; j++) {
            struct point p3 = { points_obj2[j].x   + obj2_xoff, points_obj2[j].y   + obj2_yoff };
            struct point p4 = { points_obj2[j+1].x + obj2_xoff, points_obj2[j+1].y + obj2_yoff };
            if(lines_intersect(p1, p2, p3, p4)) return true;
        }
    }
    // No lines intersect, return false.
    return false;
}

struct point mechanics_to_point(struct mechanics mech) {
    return (struct point) {
        .x = mech.x,
        .y = mech.y
    };
}

