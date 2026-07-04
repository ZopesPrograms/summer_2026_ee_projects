#ifndef MATHS_H
#define MATHS_H

#define PI 3.141592653589793

// Handy number theory functions
double pow(double base, unsigned int exponent);
double factorial(unsigned int pow);

// Function that calculates trig tables
void trig_init(unsigned int precision);

// Basic trig functions
double sine(double angle);
double cosine(double angle);
double tan(double angle);

double max(double, double);
double sgn(double);
double abs(double);
double round(double);

#endif

