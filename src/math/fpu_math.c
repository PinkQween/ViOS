#include "fpu_math.h"

// Inline assembly to use x86 FPU instructions

double fpu_add(double a, double b)
{
    double result;
    __asm__ __volatile__(
        "fldl %1;"  // Load a on FPU stack
        "fldl %2;"  // Load b on FPU stack
        "faddp;"    // ST(1) = ST(1) + ST(0), pop stack
        "fstpl %0;" // Store result to memory and pop
        : "=m"(result)
        : "m"(a), "m"(b));
    return result;
}

double fpu_sub(double a, double b)
{
    double result;
    __asm__ __volatile__(
        "fldl %1;"
        "fldl %2;"
        "fsubp;"
        "fstpl %0;"
        : "=m"(result)
        : "m"(a), "m"(b));
    return result;
}

double fpu_mul(double a, double b)
{
    double result;
    __asm__ __volatile__(
        "fldl %1;"
        "fldl %2;"
        "fmulp;"
        "fstpl %0;"
        : "=m"(result)
        : "m"(a), "m"(b));
    return result;
}

double fpu_div(double a, double b)
{
    double result;
    __asm__ __volatile__(
        "fldl %1;"
        "fldl %2;"
        "fdivp;"
        "fstpl %0;"
        : "=m"(result)
        : "m"(a), "m"(b));
    return result;
}

double fpu_sqrt(double a)
{
    double result;
    __asm__ __volatile__(
        "fldl %1;"
        "fsqrt;"
        "fstpl %0;"
        : "=m"(result)
        : "m"(a));
    return result;
}

// For trig functions, the x87 FPU has FSIN, FCOS instructions but
// they expect the argument in ST(0) in radians.

// NOTE: FSIN and FCOS only work correctly for angles in [-π/4, π/4]
// Outside this range, they may give incorrect results.

// Helper function to reduce angle to [-π/4, π/4] range
static double reduce_angle(double angle)
{
    // Reduce to [-2π, 2π] first
    while (angle > 2 * PI_F_32B)
        angle -= 2 * PI_F_32B;
    while (angle < -2 * PI_F_32B)
        angle += 2 * PI_F_32B;

    // Now reduce to [-π/4, π/4] using trig identities
    if (angle > PI_F_32B / 4 && angle <= 3 * PI_F_32B / 4)
    {
        // sin(x) = cos(x - π/2), cos(x) = -sin(x - π/2)
        return angle - PI_F_32B / 2;
    }
    else if (angle > 3 * PI_F_32B / 4 && angle <= 5 * PI_F_32B / 4)
    {
        // sin(x) = -sin(x - π), cos(x) = -cos(x - π)
        return angle - PI_F_32B;
    }
    else if (angle > 5 * PI_F_32B / 4 && angle <= 7 * PI_F_32B / 4)
    {
        // sin(x) = -cos(x - 3π/2), cos(x) = sin(x - 3π/2)
        return angle - 3 * PI_F_32B / 2;
    }
    else if (angle < -PI_F_32B / 4 && angle >= -3 * PI_F_32B / 4)
    {
        // sin(x) = -cos(x + π/2), cos(x) = sin(x + π/2)
        return angle + PI_F_32B / 2;
    }
    else if (angle < -3 * PI_F_32B / 4 && angle >= -5 * PI_F_32B / 4)
    {
        // sin(x) = -sin(x + π), cos(x) = -cos(x + π)
        return angle + PI_F_32B;
    }
    else if (angle < -5 * PI_F_32B / 4 && angle >= -7 * PI_F_32B / 4)
    {
        // sin(x) = cos(x + 3π/2), cos(x) = -sin(x + 3π/2)
        return angle + 3 * PI_F_32B / 2;
    }

    return angle;
}

double fpu_sin(double a)
{
    double reduced_angle = reduce_angle(a);
    double result;

    // Determine which quadrant we're in and apply appropriate transformation
    if (a > PI_F_32B / 4 && a <= 3 * PI_F_32B / 4)
    {
        // Use cosine with phase shift
        __asm__ __volatile__(
            "fldl %1;"
            "fcos;"
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a > 3 * PI_F_32B / 4 && a <= 5 * PI_F_32B / 4)
    {
        // Use negative sine
        __asm__ __volatile__(
            "fldl %1;"
            "fsin;"
            "fchs;" // Change sign
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a > 5 * PI_F_32B / 4 && a <= 7 * PI_F_32B / 4)
    {
        // Use negative cosine
        __asm__ __volatile__(
            "fldl %1;"
            "fcos;"
            "fchs;" // Change sign
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a < -PI_F_32B / 4 && a >= -3 * PI_F_32B / 4)
    {
        // Use negative cosine
        __asm__ __volatile__(
            "fldl %1;"
            "fcos;"
            "fchs;" // Change sign
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a < -3 * PI_F_32B / 4 && a >= -5 * PI_F_32B / 4)
    {
        // Use negative sine
        __asm__ __volatile__(
            "fldl %1;"
            "fsin;"
            "fchs;" // Change sign
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a < -5 * PI_F_32B / 4 && a >= -7 * PI_F_32B / 4)
    {
        // Use cosine
        __asm__ __volatile__(
            "fldl %1;"
            "fcos;"
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else
    {
        // Direct sine calculation for [-π/4, π/4]
        __asm__ __volatile__(
            "fldl %1;"
            "fsin;"
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }

    return result;
}

double fpu_cos(double a)
{
    double reduced_angle = reduce_angle(a);
    double result;

    // Determine which quadrant we're in and apply appropriate transformation
    if (a > PI_F_32B / 4 && a <= 3 * PI_F_32B / 4)
    {
        // Use negative sine
        __asm__ __volatile__(
            "fldl %1;"
            "fsin;"
            "fchs;" // Change sign
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a > 3 * PI_F_32B / 4 && a <= 5 * PI_F_32B / 4)
    {
        // Use negative cosine
        __asm__ __volatile__(
            "fldl %1;"
            "fcos;"
            "fchs;" // Change sign
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a > 5 * PI_F_32B / 4 && a <= 7 * PI_F_32B / 4)
    {
        // Use sine
        __asm__ __volatile__(
            "fldl %1;"
            "fsin;"
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a < -PI_F_32B / 4 && a >= -3 * PI_F_32B / 4)
    {
        // Use sine
        __asm__ __volatile__(
            "fldl %1;"
            "fsin;"
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a < -3 * PI_F_32B / 4 && a >= -5 * PI_F_32B / 4)
    {
        // Use negative cosine
        __asm__ __volatile__(
            "fldl %1;"
            "fcos;"
            "fchs;" // Change sign
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else if (a < -5 * PI_F_32B / 4 && a >= -7 * PI_F_32B / 4)
    {
        // Use negative sine
        __asm__ __volatile__(
            "fldl %1;"
            "fsin;"
            "fchs;" // Change sign
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }
    else
    {
        // Direct cosine calculation for [-π/4, π/4]
        __asm__ __volatile__(
            "fldl %1;"
            "fcos;"
            "fstpl %0;"
            : "=m"(result)
            : "m"(reduced_angle));
    }

    return result;
}

double fpu_tan(double a)
{
    double sin_val, cos_val;
    sin_val = fpu_sin(a);
    cos_val = fpu_cos(a);
    // Simple division for tan = sin / cos
    return fpu_div(sin_val, cos_val);
}

// Convert degrees to radians
double deg_to_rad(double deg)
{
    return deg * (PI_F_32B / 180.0);
}

double fpu_sin_deg(double deg)
{
    return fpu_sin(deg_to_rad(deg));
}

double fpu_cos_deg(double deg)
{
    return fpu_cos(deg_to_rad(deg));
}

double fpu_tan_deg(double deg)
{
    return fpu_tan(deg_to_rad(deg));
}

// Utility functions for checking special values
int fpu_isnan(double x)
{
    // Check if x is NaN by comparing it with itself
    // NaN is the only value that is not equal to itself
    return x != x;
}

int fpu_isinf(double x)
{
    // Check if x is infinity by comparing with a very large number
    // This is a simple approximation - in a real implementation you'd use
    // bit manipulation to check the IEEE 754 representation
    return (x > 1e308) || (x < -1e308);
}

double fpu_abs(double a)
{
    return (a < 0) ? fpu_sub(0.0, a) : a;
}

// Polynomial approximation of atan(t) for t in [-1,1]
double fpu_atan(double t)
{
    // Coefficients
    const double a = 1.0;
    const double b = -0.3333333333; // -1/3
    const double c = 0.2;           // 1/5

    double t2 = fpu_mul(t, t);

    double numerator = fpu_add(a, fpu_mul(b, t2));     // a + b * t^2
    double denominator = fpu_add(1.0, fpu_mul(c, t2)); // 1 + c * t^2

    double fraction = fpu_div(numerator, denominator);

    double approx = fpu_mul(t, fraction);

    // Adjust sign based on original t
    return approx;
}

double fpu_atan2(double y, double x)
{
    const double PI = PI_F_32B;
    const double PI_HALF = fpu_div(PI, 2.0);
    const double ZERO = 0.0;

    if (x > 0.0)
    {
        return fpu_atan(fpu_div(y, x));
    }
    else if (x < 0.0 && y >= 0.0)
    {
        return fpu_add(fpu_atan(fpu_div(y, x)), PI);
    }
    else if (x < 0.0 && y < 0.0)
    {
        return fpu_sub(fpu_atan(fpu_div(y, x)), PI);
    }
    else if (x == 0.0 && y > 0.0)
    {
        return PI_HALF;
    }
    else if (x == 0.0 && y < 0.0)
    {
        return -PI_HALF;
    }
    else
    {
        // Undefined for (0,0), return 0 by convention
        return ZERO;
    }
}

int min(int a, int b)
{
    if (a > b)
    {
        return b;
    }
    else
    {
        return a;
    }
}
int max(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

int abs(int a)
{
    if (a < 0) {
        return -a;
    } else {
        return a;
    }
}