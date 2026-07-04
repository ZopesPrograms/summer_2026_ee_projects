#include "maths.h"
#include "malloc.h"
#include "printf.h"

// Trigonometric data to load upon trig math intialization
static double* sine_table;
static unsigned int trig_precision;
static double trig_precision_scale;

// Handy number theory functions
double pow(double base, unsigned int exponent)
{
    double ret = 1;
    for(int i = 0; i < exponent; i++) {
        ret *= base;
    }
    return ret;
}

double factorial(unsigned int power) {
    double ret = 1;
    while(power != 0) {
        ret = ret * power;
        power--;
    }
    return ret;
}

void trig_init(unsigned int precision) {
    // Determines trig calculations precision scaling
    trig_precision = precision;
    trig_precision_scale = pow(10, precision);
    
    // Determines size for sine calculations table and allocates it in memory
    int table_len = (int)(2*PI*trig_precision_scale);
    sine_table = malloc(sizeof(double)*table_len);

    double angle = 0;
    double angle_it = (double)(1.0/trig_precision_scale);
    for(int i = 0; i < table_len; i++) { // Raw taylor series expansion calculations of sine function
        sine_table[i] = angle - (pow(angle,3)/(double)factorial(3)) + (pow(angle,5)/(double)factorial(5));
        for(int j = 0; j < precision+2; j++) {
            sine_table[i] -= (pow(angle,7+(2*j))*pow(-1, (j & 1))/(double)factorial(7+2*j));
        }
        angle += angle_it; // Going through all angles which sine could be called for.
    }
}

// Basic trig functions
double sine(double angle) {
    while (angle < 0) {
        angle += 2 * PI;
    }
    while (angle > 2 * PI) {
        angle -= 2 * PI;
    }
    //double sign = (angle < PI) ? 1 : -1; 
    int index = (int)(angle*trig_precision_scale); // Calculates index that the sin of said angle SHOULD BE AT, or is closest to it.
    return sine_table[index];
}
double cosine(double angle) {
    return sine(angle+(PI/2));
}
double tan(double angle) {
    return sine(angle)/cosine(angle);
}

double abs(double num) {
    if (num < 0) {
        return -num;
    }
    return num;
}

double max(double a, double b) {
    if (a > b) {
        return a;
    }
    return b;
}

double sgn(double num) {
    if (num > 0) {
        return 1;
    }
    if (num < 0) {
        return -1;
    }
    return 0;
}

double round(double num) {
    // if num's tenth's place (1.t) has t<=4 then return (int)num else return (int)num+1
    // num*10 is xxxt.xxx
    // (int)(num*10) is xxxt
    // (int)(num*10) % 10 is t

    int t = (int)(num * 10) % 10;
    int ret = (int)num;

    if (t <= 4) {
        return (double)ret;
    }
    // else t >= 5
    return (double)ret + 1;
}

