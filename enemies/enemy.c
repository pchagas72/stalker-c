// stalker-c/enemies/enemy.c
// This is without a doubt the most complex part of the game for now

#include "enemy.h"
#include <SDL3/SDL_rect.h>
#include <stdio.h>

void enemy_create(Enemy* enemy, const EnemyData *data, const Map *map) {
    // Set the enemy's dimensions from the parsed data
    enemy->rect.w = data->size.x;
    enemy->rect.h = data->size.y;

    // Convert grid-based spawn coordinates to pixel coordinates
    const float map_pixel_width = map->width * TILE_SIZE;
    const float map_pixel_height = map->height * TILE_SIZE;

    enemy->rect.x =  data->spawn_pos.x * TILE_SIZE;
    enemy->rect.y =  data->spawn_pos.y * TILE_SIZE;

    // Initialize enemy from level data
    enemy->sight_range = data->sight_range;
    enemy->perception_radius = data->perception_radius;
    enemy->attack_range = data->attack_range;
    enemy->walking_speed = data->walking_speed;
    enemy->stalking_speed = data->stalking_speed;
    enemy->attacking_speed = data->attacking_speed;

    // Initialize AI state and other variables
    enemy->vel.x = 0.0f;
    enemy->vel.y = 0.0f;
    enemy->current_state = AI_STATE_CLUELESS;
    enemy->current_path = NULL;
    enemy->rescan_timer = 90;
    enemy->patrol_timer = 0;
    enemy->alert_modifier = 0.5;

    // --- **THE FIX: Completed debug printf statement** ---
    printf("Initialized enemy '%s' with:\n"
           "  Size: (%.0f, %.0f)\n"
           "  Sight: %.0f, Perception: %.0f, Attack Range: %.0f\n"
           "  Speed (Walk/Stalk/Attack): %.1f/%.1f/%.1f\n",
           data->id, data->size.x, data->size.y,
           enemy->sight_range, enemy->perception_radius, enemy->attack_range,
           enemy->walking_speed, enemy->stalking_speed, enemy->attacking_speed);
}

// These logics will pretty much be universal, but the variables that coordinate
// them (anger, velocity, hearing distance, FOV) will be different for each
// type of enemy
static void enemy_logic_clueless(Enemy *enemy, const Player *player, const Map *map);
static void enemy_logic_stalking(Enemy *enemy, const Player *player, const Map *map);
static void enemy_logic_attacking(Enemy *enemy, const Player *player, const Map *map);

/// This is the function that updated the enemies
/// This function coordinates movement, state, collisions and pathfinding
void enemy_update(Enemy* enemy, const Player* player, const Map* map) {
    // Resets the speed so it doesn't add up to infinity
    enemy->vel.x = 0;
    enemy->vel.y = 0;

    // This is the enemy brain
    switch (enemy->current_state) {
        case AI_STATE_CLUELESS:
            enemy_logic_clueless(enemy, player, map);
            break;
        case AI_STATE_STALKING:
            enemy_logic_stalking(enemy, player, map);
            break;
        case AI_STATE_ATTACKING:
            enemy_logic_attacking(enemy, player, map);
            break;
        default:
            break;
    }

    // Starts collision handler
    // Could be it's own function

    // As I said before this could very well be on the map struct
    const float map_pixel_width = map->width * TILE_SIZE;
    const float map_pixel_height = map->height * TILE_SIZE;

    // Now this is important, it's what keeps the enemy from being stuck on corners
    const float COLLISION_INSET = 2.f; 

    // The collision check should be pretty straight up
    enemy->rect.x += enemy->vel.x;

    // Gets all the grids near the enemy object
    int grid_x_left = (enemy->rect.x + COLLISION_INSET) / TILE_SIZE;
    int grid_x_right = (enemy->rect.x + enemy->rect.w - COLLISION_INSET) / TILE_SIZE;
    int grid_y_top = (enemy->rect.y + COLLISION_INSET) / TILE_SIZE;
    int grid_y_bottom = (enemy->rect.y + enemy->rect.h - COLLISION_INSET) / TILE_SIZE;
    
    if (enemy->vel.x > 0) { // Moving right
        if (map->tiles[grid_y_top][grid_x_right] != 0 || map->tiles[grid_y_bottom][grid_x_right] != 0) {
            // If top right or bottom right are not empty
            // This will 100% be changed to walkable, and a function will be added to the map
            // that checks if tile is walkable or not
            enemy->rect.x = grid_x_right * TILE_SIZE - enemy->rect.w;
        }
    } else if (enemy->vel.x < 0) { // Moving left
        if (map->tiles[grid_y_top][grid_x_left] != 0 || map->tiles[grid_y_bottom][grid_x_left] != 0) {
            // Same for the right one
            enemy->rect.x =  (grid_x_left + 1) * TILE_SIZE - enemy->rect.w;
        }
    }

    // Handle Y-axis collision
    // The comments for this area are pretty much the same of the above
    enemy->rect.y += enemy->vel.y;
    
    // Recalculate grid positions with the inset
    grid_x_left = (enemy->rect.x + COLLISION_INSET) / TILE_SIZE;
    grid_x_right = (enemy->rect.x + enemy->rect.w - COLLISION_INSET) / TILE_SIZE;
    grid_y_top = (enemy->rect.y + COLLISION_INSET) / TILE_SIZE;
    grid_y_bottom = (enemy->rect.y + enemy->rect.h - COLLISION_INSET) / TILE_SIZE;

    if (enemy->vel.y > 0) { // Moving down
         if (map->tiles[grid_y_bottom][grid_x_left] != 0 || map->tiles[grid_y_bottom][grid_x_right] != 0) {
            enemy->rect.y = grid_y_bottom * TILE_SIZE - enemy->rect.h;
        }
    } else if (enemy->vel.y < 0) { // Moving up
        if (map->tiles[grid_y_top][grid_x_left] != 0 || map->tiles[grid_y_top][grid_x_right] != 0) {
            enemy->rect.y = (grid_y_top + 1) * TILE_SIZE;
        }
    }
}

/// The enemy is clueless when he does not know that there is a player near by
/// This function will be changed because the enemy cannot forget that there was
/// a player in the area, and currently, he does
/// Because of that, in the future the state ALERTED will be added
/// This function is currently a bit of both ALERTED and CLUELESS states
static void enemy_logic_clueless(Enemy *enemy, const Player *player, const Map *map) {
    Vector2f player_pos = {player->rect.x, player->rect.y};
    Vector2f enemy_pos = {enemy->rect.x, enemy->rect.y};

    // Always check for the player first
    // If the player is detected change state immediately
    float distance_to_player = vector_magnitude(vector_subtract(player_pos, enemy_pos));
    bool detected = false;

    // If the enemy HEARS the player
    if (distance_to_player < (enemy->perception_radius * player->noise * enemy->alert_modifier)) {
        enemy->alert_modifier += 0.1;
        detected = true;
    }
    // If the enemy SEES the player
    if (!detected && distance_to_player < (enemy->sight_range * enemy->alert_modifier)) {
        if (map_has_line_of_sight(map, enemy_pos, player_pos)) {
            enemy->alert_modifier += 0.1;
            detected = true;
        }
    }

    if (detected) {
        printf("[Enemy] Player detected! Transitioning to STALKING.\n");
        if (enemy->current_path) {
            path_destroy(enemy->current_path);
            enemy->current_path = NULL;
        }
        enemy->last_known_player_pos = player_pos;
        enemy->current_state = AI_STATE_STALKING;
        return; // Exit immediately
    }

    // Patrols the area randomly
    // This function is currently weird af I need to study it
    if (--enemy->patrol_timer <= 0 || enemy->current_path == NULL) {
        if (enemy->current_path) { // Clear old path if it exists
            path_destroy(enemy->current_path);
            enemy->current_path = NULL;
        }
        // This function currently has a bug
        // If the player closes the game while the enemy is walking a random path
        // the memory IS NOT FREED and upon a new instance of the game, a core dump
        // could happen.
        // To fix this just free the damn path on closing the game
        // That's why every object must have a "free" function
        Vector2f patrol_target = map_get_random_walkable_tile(map);
        enemy->current_path = pathfinding_find_path(map, enemy_pos, patrol_target);
        enemy->patrol_timer = 900; // Reset timer
    }

    // Follows the path, this is a standard function and could be separated
    if (enemy->current_path != NULL && enemy->current_path->current_node < enemy->current_path->count) {
        Vector2f target_pos = enemy->current_path->points[enemy->current_path->current_node];
        Vector2f dir = vector_subtract(target_pos, enemy_pos);
        float distance = vector_magnitude(dir);

        if (distance < TILE_SIZE / 2.0f) {
            enemy->current_path->current_node++;
        } else {
            Vector2f norm_dir = vector_normalize(dir);
            enemy->vel.x = norm_dir.x * enemy->walking_speed;
            enemy->vel.y = norm_dir.y * enemy->walking_speed;
        }
    } else {
        // Reached destination or path failed, clear path to get a new one next tick
        if (enemy->current_path) {
            path_destroy(enemy->current_path);
            enemy->current_path = NULL;
        }
    }
}

/// The stalking logic is pretty decent for now, with the implementation of the
/// player FOV this function will be changed a lot, because stalking is different
/// from attacking, while stalking the enemy should try to remain UNSEEN by the
/// player at all times, an if the player sees him, he either attacks or hides.
/// To choose if it flees or attacks distance and a random 50/50 should be the weights
static void enemy_logic_stalking(Enemy *enemy, const Player *player, const Map *map) {
    Vector2f player_pos = { player->rect.x, player->rect.y };
    Vector2f enemy_pos = { enemy->rect.x, enemy->rect.y };

    // Re-scans every half a second to keep
    if (--enemy->rescan_timer <= 0) {
        enemy->rescan_timer = 90;
        float distance = vector_magnitude(vector_subtract(player_pos, enemy_pos));
        if (
                (distance < (enemy->sight_range * enemy->alert_modifier) && map_has_line_of_sight(map, enemy_pos, player_pos))
                || // Please just create a fixed boolean instead of writing this every time
                (distance < (enemy->perception_radius * player->noise * enemy->alert_modifier))
                ) {
            enemy->last_known_player_pos = player_pos;
            if (enemy->current_path) {
                printf("[ENEMY] Updated player pos\n");
                path_destroy(enemy->current_path);
                enemy->current_path = NULL;
            }
            if (distance < enemy->attack_range && map_has_line_of_sight(map, enemy_pos, player_pos)) {
                enemy->current_state = AI_STATE_ATTACKING;
                printf("[ENEMY] Entering attack mode.\n");
                return;
            }
        }
    }

    // Pathfinding

    if (enemy->current_path == NULL) {
        enemy->current_path = pathfinding_find_path(map, enemy_pos, enemy->last_known_player_pos);
        if (enemy->current_path != NULL && enemy->current_path->count > 1) {
            enemy->current_path->current_node = 1;
        }
    }

    if (enemy->current_path != NULL && enemy->current_path->current_node < enemy->current_path->count) {
        Vector2f target_pos = enemy->current_path->points[enemy->current_path->current_node];
        Vector2f dir = vector_subtract(target_pos, enemy_pos);
        float distance = vector_magnitude(dir);

        if (distance < TILE_SIZE / 2.0f) {
            enemy->current_path->current_node++;
        } else {
            Vector2f norm_dir = vector_normalize(dir);
            enemy->vel.x = norm_dir.x * enemy->stalking_speed;
            enemy->vel.y = norm_dir.y * enemy->stalking_speed;
        }
    } else {
        if (enemy->current_path) {
            path_destroy(enemy->current_path);
            enemy->current_path = NULL;
        }
        enemy->current_state = AI_STATE_CLUELESS;
        enemy->alert_modifier += .5;
    }
}

// I NEED TO ADD PATHFINDING TO THIS FUNCTION?
static void enemy_logic_attacking(Enemy *enemy, const Player *player, const Map *map) {
    Vector2f player_pos = { player->rect.x, player->rect.y };
    Vector2f enemy_pos = { enemy->rect.x, enemy->rect.y };
    float distance = vector_magnitude(vector_subtract(player_pos, enemy_pos));

    if (
            ((distance > (enemy->attack_range))
            ||
            !map_has_line_of_sight(map, enemy_pos, player_pos))
            ) {
        printf("[ENEMY] Lost player line of sight\n");
        enemy->last_known_player_pos = player_pos;
        enemy->current_state = AI_STATE_STALKING;
        return;
    }

    Vector2f dir = vector_normalize(vector_subtract(player_pos, enemy_pos));
    enemy->vel.x = dir.x * enemy->attacking_speed;
    enemy->vel.y = dir.y * enemy->attacking_speed;
}

void enemy_render(Enemy* enemy, SDL_Renderer* renderer, const SDL_FRect *camera) {
    // Create a temporary rect for rendering, adjusted by the camera's position
    SDL_FRect render_rect = {
        .x = enemy->rect.x - camera->x,
        .y = enemy->rect.y - camera->y,
        .w = enemy->rect.w,
        .h = enemy->rect.h
    };

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &render_rect);

    // The collision point indicators will also be drawn relative to the camera now
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
    SDL_FPoint corners[4] = {
        {render_rect.x, render_rect.y},
        {render_rect.x + render_rect.w, render_rect.y},
        {render_rect.x, render_rect.y + render_rect.h},
        {render_rect.x + render_rect.w, render_rect.y + render_rect.h}
    };

    for (int i = 0; i < 4; ++i) {
        SDL_FRect corner_indicator = { corners[i].x - 1.0f, corners[i].y - 1.0f, 2.0f, 2.0f };
        SDL_RenderFillRect(renderer, &corner_indicator);
    }
}
