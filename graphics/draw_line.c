#include "draw_line.h"

// (x1, y1) is point 1, (x2, y2) is point 2
void draw_line(double x1, double y1, double x2, double y2, color_t c) {
    // dx/dy is what you have to do to move from p1 to p2
    double dx = x2 - x1;
    double dy = y2 - y1;

    // If y is the larger difference, move from y1 to y2 by ±1 and along the way have an appropriate x increase (by ±<1)
    axis direction_of_most_movement = abs(dy) > abs(dx) ? Y : X;

    // pos moves from point 1 to point 2
    double posx = x1;
    double posy = y1;
    
    if (direction_of_most_movement == Y) {
        int sgn_dy = sgn(dy);
        // For every +1 in posy, posx increases by dx/dy
        double dx_div_dy = dx / dy;

        while (true) {
            gl_draw_pixel(round(posx), round(posy), c); //double rounded to int so it draws the nearest pixel

            posy += sgn_dy; //change by ±1
            posx += sgn_dy * dx_div_dy;

            // Check if done (drew past p2). If done, return
            if (
                // Increasing
                (sgn_dy == 1 && posy >= y2)
                ||
                // Decreasing
                (sgn_dy == -1 && posy <= y2)
            ) {
                return;
            }
        }
    } else { //direction of most movement is X
        int sgn_dx = sgn(dx);
        double dy_div_dx = dy / dx;

        while (true) {
            gl_draw_pixel(round(posx), round(posy), c);
            
            posx += sgn_dx;
            posy += sgn_dx * dy_div_dx;

            if (
                // Moving right
                (sgn_dx == 1 && posx >= x2)
                ||
                // Moving left
                (sgn_dx == -1 && posx <= x2)
            ) {
                return;
            }
        }
    }
}

