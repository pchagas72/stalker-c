#include "player.h"
#include <SDL3/SDL_rect.h>
#include <math.h>

void player_create(Player* player, Map *map) {
    player->rect.h = 15;
    player->rect.w = 10;
    
    const float map_pixel_width = map->width * TILE_SIZE;
    const float map_pixel_height = map->height * TILE_SIZE;

    player->rect.x = TILE_SIZE * map->playerSpawn.x;
    player->rect.y = TILE_SIZE * map->playerSpawn.y;

    player->vel.x = 0.0f;
    player->vel.y = 0.0f;
    player->stamina = 100;
    player->noise = PLAYER_IDLE_NOISE;
}

void player_render(Player* player, SDL_Renderer* renderer, const Camera *camera) {
    SDL_FRect render_rect = {
        .x = player->rect.x - camera->x,
        .y = player->rect.y - camera->y,
        .w = player->rect.w,
        .h = player->rect.h
    };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &render_rect);
}

static void player_handle_map_collision(Player* player, const Map* map) {
    const float map_pixel_width = map->width * TILE_SIZE;
    const float map_pixel_height = map->height * TILE_SIZE;

    // Clamp player to map
    //printf("Player: %f,%f\n",player->rect.x,player->rect.y);
    //printf("Map: %d,%d\n",map->width*TILE_SIZE,map->height*TILE_SIZE);
    // Clamp to map boundaries first
    if (player->rect.x < 1) {
        player->rect.x = 1;
        player->vel.x = 0;
    } else if (player->rect.x + player->rect.w > map_pixel_width-1) {
        player->rect.x = map_pixel_width - player->rect.w-1;
        player->vel.x = 0;
    }
    if (player->rect.y < 1) {
        player->rect.y = 1;
        player->vel.y = 0;
        return;
    } else if (player->rect.y + player->rect.h > map_pixel_height-1) {
        player->rect.y = map_pixel_height - player->rect.h-1;
        player->vel.y = 0;
        return;
    }

    // --- Handle X-axis collision ---
    float potential_x = player->rect.x + player->vel.x;
    int grid_x, grid_y_top, grid_y_bottom;

    if (player->vel.x > 0) { // Moving right
        grid_x = (potential_x + player->rect.w) / TILE_SIZE;
    } else { // Moving left
        grid_x = (potential_x) / TILE_SIZE;
    }
    grid_y_top = (player->rect.y) / TILE_SIZE;
    grid_y_bottom = (player->rect.y + player->rect.h) / TILE_SIZE;

    if (map->tiles[grid_y_top][grid_x] == 1 || map->tiles[grid_y_bottom][grid_x] == 1) {
        player->vel.x = 0;
    }


    // --- Handle Y-axis collision ---
    float potential_y = player->rect.y + player->vel.y;
    int grid_y, grid_x_left, grid_x_right;

    if (player->vel.y > 0) { // Moving down
        grid_y = (potential_y + player->rect.h) / TILE_SIZE;
    } else { // Moving up
        grid_y = (potential_y) / TILE_SIZE;
    }
    grid_x_left = (player->rect.x) / TILE_SIZE;
        grid_x_right = (player->rect.x + player->rect.w) / TILE_SIZE;

    if (map->tiles[grid_y][grid_x_left] == 1 || map->tiles[grid_y][grid_x_right] == 1) {
        player->vel.y = 0;
    }
}

void player_update(Player* player, const bool* keyboard_state, const Map* map) {
    // --- (Input gathering and velocity calculation is unchanged) ---
    int input_x = 0;
    int input_y = 0;
    bool is_running = keyboard_state[SDL_SCANCODE_LSHIFT];

    if (keyboard_state[SDL_SCANCODE_W]) { input_y = -1; }
    if (keyboard_state[SDL_SCANCODE_S]) { input_y = 1; }
    if (keyboard_state[SDL_SCANCODE_A]) { input_x = -1; }
    if (keyboard_state[SDL_SCANCODE_D]) { input_x = 1; }
    
    if (input_x != 0 || input_y != 0) {
        float magnitude = sqrtf((float)(input_x * input_x + input_y * input_y));
        player->vel.x += (input_x / magnitude) * PLAYER_ACCELERATION;
        player->vel.y += (input_y / magnitude) * PLAYER_ACCELERATION;
    } else {
        player->vel.x *= PLAYER_FRICTION;
        player->vel.y *= PLAYER_FRICTION;
        player->noise = PLAYER_IDLE_NOISE;
    }
    float current_speed = sqrtf(player->vel.x * player->vel.x + player->vel.y * player->vel.y);
    float max_speed;
    if (is_running && player->stamina > 0 && (input_x != 0 || input_y != 0)) {
        max_speed = MAX_PLAYER_RUNNING_SPEED;
        player->noise = PLAYER_RUNNING_NOISE;
        if (current_speed > 0.1f) player->stamina--;
    } else if (input_x != 0 || input_y != 0){
        max_speed = MAX_PLAYER_WALKING_SPEED;
        player->noise = PLAYER_WALKING_NOISE;
        if (player->stamina < 100) player->stamina++;
    }
    if (current_speed > max_speed) {
        player->vel.x = (player->vel.x / current_speed) * max_speed;
        player->vel.y = (player->vel.y / current_speed) * max_speed;
    }

    // --- NEW: Handle collision before updating position ---
    player_handle_map_collision(player, map);

    // --- Final position update ---
    player->rect.x += player->vel.x;
    // Btw this is a fix, if removed a core dump happens when the player runs
    // toward the bottom part of the map
    if (((player->rect.y + player->vel.y) + player->rect.h) < (map->height * TILE_SIZE)-(player->rect.h/2)) {
        player->rect.y += player->vel.y;
    }
}
