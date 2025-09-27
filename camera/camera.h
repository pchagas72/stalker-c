#ifndef CAMERA_H
#define CAMERA_H

#include <SDL3/SDL.h>
#include "../map/map.h"

// The camera is just a rectangle representing the viewport into the game world.
typedef SDL_FRect Camera;

// Forward declaring
struct player;

// This function will update the camera's position to follow the player
// and ensure it doesn't show areas outside the map.
void camera_update(Camera* camera, const struct player* player, const Map* map, float zoom);

#endif // CAMERA_H
