#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "../helper/vector.h"
#include "../map/map.h"

#define MAX_PATH_LENGTH 100

typedef struct {
    Vector2f points[MAX_PATH_LENGTH];
    int count;
    int current_node;
} Path;

// Main function to find a path from a start to an end point
Path* pathfinding_find_path(const Map* map, Vector2f start_pos, Vector2f end_pos);

// Function to free the memory used by a path
void path_destroy(Path* path);

#endif
