#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

typedef struct {
    float x;
    float y;
} Vector2f;

typedef struct {
    int x;
    int y;
} Vector2;

// Subtracts v2 from v1 (v1 - v2) and returns the resulting vector
Vector2f vector_subtract(Vector2f v1, Vector2f v2);

// Calculates the magnitude (length) of a vector
float vector_magnitude(Vector2f v);

// Returns a normalized version of the vector (a vector with the same direction but a length of 1)
Vector2f vector_normalize(Vector2f v);

#endif // VECTOR_H
