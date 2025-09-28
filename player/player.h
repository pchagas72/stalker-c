#ifndef PLAYER_H
#define PLAYER_H

/// Our player is the second highest in the list of imports
/// To evade import loops, we'll use a grading system, in which the map is the 
/// most important component and will not include any other object
/// The player comes second in this, for he imports the map

#include <SDL3/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "../map/map.h"
#include "../camera/camera.h"

/// Player Physics
/// The player physics is extensive, and very different from the enemy physics
/// Maybe I'll actually cap the physics of the player a bit to make it simpler
#define PLAYER_ACCELERATION 0.15f
#define PLAYER_FRICTION 0.70f
#define MAX_PLAYER_WALKING_SPEED 1.3f
#define MAX_PLAYER_RUNNING_SPEED 2.0f

/// Player sensors
/// These are really not sensors, they just interact with the map
/// For now we have noise applied, but soon we'll add field of view
#define PLAYER_WALKING_NOISE 1.f
#define PLAYER_RUNNING_NOISE 5.f
#define PLAYER_IDLE_NOISE 0.5f

/// Main player struct, everything player related needs to be here
typedef struct player {
    SDL_FRect rect;
    Vector2f vel;
    uint8_t stamina;
    float noise;
} Player;

/// Starts the player object, builds it
void player_create(Player* player, Map *map);
/// Updates the player movement, noise, collision...
void player_update(Player* player, const bool* keyboard_state, const Map *map);
/// Renders the player, soon I'll add textures
void player_render(Player* player, SDL_Renderer* renderer, const Camera *camera);

#endif // PLAYER_H
