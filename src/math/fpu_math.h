#ifndef FPU_MATH_H
#define FPU_MATH_H

#define PI_F_32B 3.14159265358979323846f

// Basic floating point math using FPU

// Adds two doubles
double fpu_add(double a, double b);

// Subtracts two doubles
double fpu_sub(double a, double b);

// Multiplies two doubles
double fpu_mul(double a, double b);

// Divides two doubles
double fpu_div(double a, double b);

// Computes square root using FPU
double fpu_sqrt(double a);

// Computes sine of a double (radians)
double fpu_sin(double a);

// Computes cosine of a double (radians)
double fpu_cos(double a);

// Computes tangent of a double (radians)
double fpu_tan(double a);

// Convert degrees to radians
double deg_to_rad(double deg);

// Degree-based trig functions
double fpu_sin_deg(double deg);
double fpu_cos_deg(double deg);
double fpu_tan_deg(double deg);

// Utility functions for checking special values
int fpu_isnan(double x);
int fpu_isinf(double x);

double fpu_abs(double a);
double fpu_atan(double t);
double fpu_atan2(double y, double x);

int min(int a, int b);
int max(int a, int b);
int abs(int a);

#endif // FPU_MATH_H
