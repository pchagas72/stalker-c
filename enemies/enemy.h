#ifndef ENEMY_H
#define ENEMY_H

#include <SDL3/SDL.h>
#include "../player/player.h"
#include "../helper/vector.h"
#include "../helper/pathfinding.h"
#include "../map/map.h"

typedef enum {
    AI_STATE_CLUELESS,  // Walks randomly through the map
    AI_STATE_STALKING,  // Stalks player
    AI_STATE_HIDING,    // Hides from the player FOV if seen while stalking
    AI_STATE_PEEKING,   // Peeks to see if player is more vulnerable
    AI_STATE_ATTACKING, // Attacks if player is vulnerable
    AI_STATE_FLEEING,   // FLees if the player is well prepared to fight
    AI_STATE_HEALING,   // Stays hidden while healing
    AI_STATE_INVESTIGATING, // Checks the radius of last player location
} AI_State;

typedef struct {
    SDL_FRect rect;
    Vector2f vel;

    float sight_range;
    float perception_radius;
    float attack_range;

    float walking_speed;
    float stalking_speed;
    float attacking_speed;

    int rescan_timer;
    int patrol_timer;
    int attacking_timer;
    float alert_modifier;
    AI_State current_state;
    Vector2f last_known_player_pos;
    Path *current_path;
} Enemy;

void enemy_create(Enemy* enemy, const EnemyData *data, const Map *map);
void enemy_update(Enemy* enemy, const Player* player, const Map *level_map);
void enemy_render(Enemy* enemy, SDL_Renderer* renderer, const SDL_FRect *camera);

#endif // ENEMY_H
