#include "common/analysis/FastRegression.hpp"

FastRegression::FastRegression()
{
    reset();
}

void FastRegression::reset()
{
    n = 0;
    sum_x = 0;
    sum_y = 0;
    sum_x_sq = 0;
    sum_y_sq = 0;
    sum_xy = 0;
}

void FastRegression::addPoint(float x_val, float y_val)
{
    n++;
    sum_x += x_val;
    sum_y += y_val;
    // Cast to avoid overflow on sums
    sum_x_sq += (double) x_val * x_val;
    sum_xy += (double) x_val * y_val;
    sum_y_sq += (double) y_val * y_val;
}

bool FastRegression::compute(float &slope, float &offset, float &error_std)
{
    if (n < 2) return false;

    // Calculation of the denominator in integer (perfect precision)
    double denom = (n * sum_x_sq) - (sum_x * sum_x);
    
    if (std::abs(denom) < 1e-9) return false; // Division by zero

    // The ESP32-S3 has an FPU, float is fast, but double ensures 
    // no precision loss on division of very large numbers.
    
    // Slope (a)
    double a = ((n * sum_xy) - (sum_x * sum_y)) / denom;
    // Offset (b)
    double b = (sum_y - (a * sum_x)) / n;

    // Error (RMSE) - Trick: direct calculation from sums
    double sse = sum_y_sq - (b * sum_y) - (a * sum_xy);
    if (sse < 0) sse = 0; // Mathematical safety

    // Fill output variables
    slope = (float) a;
    offset = (float) b;
    error_std = (float) sqrtf(sse / n);

    return true;
}