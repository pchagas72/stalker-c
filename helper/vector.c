#include "vector.h"

Vector2f vector_subtract(Vector2f v1, Vector2f v2) {
    Vector2f result = { v1.x - v2.x, v1.y - v2.y };
    return result;
}

float vector_magnitude(Vector2f v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

Vector2f vector_normalize(Vector2f v) {
    float mag = vector_magnitude(v);
    if (mag > 0) {
        Vector2f result = { v.x / mag, v.y / mag };
        return result;
    }
    // Return a zero vector if the magnitude is zero to avoid division by zero
    Vector2f zero = { 0.0f, 0.0f };
    return zero;
}
