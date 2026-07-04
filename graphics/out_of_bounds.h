#pragma once

#include <stdbool.h>
#include "../constants.h"
#include "point.h"

/* If an object of a given width and height is out of bounds */
bool out_of_bounds(struct point p, double width, double height) {
    return (
           p.x < -width / 2
        || p.x > MONITOR_WIDTH + width / 2
        || p.y < -height / 2
        || p.y > MONITOR_HEIGHT + height / 2
    );
}

