// stalker-c/map/map.c

#include "map.h"
#include <SDL3/SDL_rect.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


static NPCData* find_npc_data_by_id(Map* map, const char* id) {
    for (int i = 0; i < map->npc_count; ++i) {
        if (strcmp(map->npcs[i].id, id) == 0) {
            return &map->npcs[i];
        }
    }
    return NULL; // Return NULL if not found
}

/// This is the main map functiom it works as a "parser" for the map file
void map_load_from_file(Map* map, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open map file %s\n", filename);
        return;
    }

    // This assumes the first line of the map file is "width,height"
    if (fscanf(file, "%d,%d\n", &map->width, &map->height) != 2) {
        printf("Error: Could not read map dimensions from %s\n", filename);
        fclose(file);
        return;
    }

    // Dynamically allocate memory for the tiles
    map->tiles = (int**)malloc(map->height * sizeof(int*));
    for (int i = 0; i < map->height; i++) {
        map->tiles[i] = (int*)malloc(map->width * sizeof(int));
    }

    srand(time(NULL));

    map->enemy_count = 0;

    Parser p;
    p.state = STATE_UNKNOWN;
    p.line_number = 0;

    char line_buffer[256];
    int map_y = 0;

    while (fgets(line_buffer, sizeof(line_buffer), file)){
        p.line_number++;

        if (line_buffer[0] == '\n' || line_buffer[0] == '#' || line_buffer[0] == '\r') {
        continue;
        }

        // Check for section
        if (strncmp(line_buffer, "enemy:", 6) == 0) {
            p.state = STATE_PARSING_ENEMY;
            continue;
        } else if (strncmp(line_buffer, "npc:", 4) == 0) {
            p.state = STATE_PARSING_NPC;
            continue;
        } else if (strncmp(line_buffer, "map:", 4) == 0) {
            p.state = STATE_PARSING_MAP;
            continue;
        } 

        switch (p.state){
            case STATE_PARSING_ENEMY:
                if (map->enemy_count < MAX_ENEMIES){
                    sscanf(line_buffer, "%[^=]=(%f,%f,%f,%f,%f,%f,%f,%f)",
                            map->enemies[map->enemy_count].id,
                            &map->enemies[map->enemy_count].size.x,
                            &map->enemies[map->enemy_count].size.y,
                            &map->enemies[map->enemy_count].sight_range,
                            &map->enemies[map->enemy_count].perception_radius,
                            &map->enemies[map->enemy_count].attack_range,
                            &map->enemies[map->enemy_count].walking_speed,
                            &map->enemies[map->enemy_count].stalking_speed,
                            &map->enemies[map->enemy_count].attacking_speed
                            );
                    printf("Created enemy '%s' (%f,%f)\n",
                            map->enemies[map->enemy_count].id,
                            map->enemies[map->enemy_count].size.x,
                            map->enemies[map->enemy_count].size.y);
                    map->enemy_count++;
                }
                break;

            case STATE_PARSING_MAP:
                if (map_y < map->height) {
                    for (int x = 0; x < map->width; x++) {
                        char current_char = line_buffer[x];
                        char peek_char = line_buffer[x+1];

                        map->tiles[map_y][x] = 0;

                        if (current_char == '1') {
                            map->tiles[map_y][x] = 1;
                        } else if (current_char == 'P') {
                            map->playerSpawn.x = x;
                            map->playerSpawn.y = map_y;
                        } else if (current_char == 'E' && peek_char >= '0' && peek_char <= '9') {
                            char enemy_id[4] = {'E', peek_char, '\0'};
                            for (int i = 0; i < map->enemy_count; ++i){
                                if (strcmp(map->enemies[i].id, enemy_id) == 0) {
                                    map->enemies[i].spawn_pos.x = x;
                                    map->enemies[i].spawn_pos.y = map_y;
                                    map->enemies[i].has_spawned = true;
                                    break;
                                }
                            }
                        } else if (current_char == 'N' && peek_char >= '0' && peek_char <= '9') {
                            char npc_id[4] = { 'N', peek_char, '\0' };
                            for (int i = 0; i < map->npc_count; ++i) {
                                if (strcmp(map->npcs[i].id, npc_id) == 0) {
                                    map->npcs[i].spawn_pos.x = x;
                                    map->npcs[i].spawn_pos.y = map_y;
                                    map->npcs[i].has_spawned = true;
                                    break;
                                }
                            }
                        }
                    }
                    map_y++;
                }
                break;
            case STATE_PARSING_NPC:
                // --- THIS IS THE NEW LOGIC ---
                if (strncmp(line_buffer, "define:", 7) == 0) {
                    if (map->npc_count < MAX_NPCS) {
                        NPCData* current_npc = &map->npcs[map->npc_count];
                        current_npc->dialogue_line_count = 0; // IMPORTANT: Initialize count
                        sscanf(line_buffer, "define:%[^,],%f,%f",
                               current_npc->id, &current_npc->size.x, &current_npc->size.y);
                        map->npc_count++;
                    }
                } else if (strncmp(line_buffer, "dialogue:", 9) == 0) {
                    char temp_id[4];
                    char temp_dialogue[MAX_DIALOGUE_LINE_LENGTH];
                    
                    // The format string "%[^,],\"%[^\"]\"" is specific:
                    // %[^,]    -> Read everything until a comma (the ID)
                    // ,\"       -> Match the comma and the opening quote
                    // %[^\"]\" -> Read everything until the closing quote
                    sscanf(line_buffer, "dialogue:%[^,],\"%[^\"]\"", temp_id, temp_dialogue);
                    
                    NPCData* target_npc = find_npc_data_by_id(map, temp_id);
                    if (target_npc && target_npc->dialogue_line_count < MAX_DIALOGUE_LINES) {
                        strcpy(target_npc->dialogue_lines[target_npc->dialogue_line_count], temp_dialogue);
                        target_npc->dialogue_line_count++;
                    }
                }
                break;

            case STATE_UNKNOWN:
                break;
        }
    }
}

void map_render(Map* map, SDL_Renderer* renderer, const SDL_FRect* camera) {
    const float map_pixel_width = map->width * TILE_SIZE;
    const float map_pixel_height = map->height * TILE_SIZE;

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);

    // Optimization: determine which tiles are visible to the camera
    int start_col = (camera->x) / TILE_SIZE;
    int end_col = (camera->x + camera->w) / TILE_SIZE + 1;
    int start_row = (camera->y) / TILE_SIZE;
    int end_row = (camera->y + camera->h) / TILE_SIZE + 1;

    // Clamp values to be within map bounds
    if (start_col < 0) start_col = 0;
    if (end_col > map->width) end_col = map->width;
    if (start_row < 0) start_row = 0;
    if (end_row > map->height) end_row = map->height;

    // Loop only through the visible tiles
    for (int y = start_row; y < end_row; y++) {
        for (int x = start_col; x < end_col; x++) {
            if (map->tiles[y][x] == 1) {
                // Calculate the tile's absolute world position
                float tile_world_x =  (float)x * TILE_SIZE;
                float tile_world_y = (float)y * TILE_SIZE;

                // Create a render rectangle adjusted by the camera's position
                SDL_FRect wall_rect = {
                    .x = tile_world_x - camera->x,
                    .y = tile_world_y - camera->y,
                    .w = TILE_SIZE,
                    .h = TILE_SIZE
                };
                SDL_RenderRect(renderer, &wall_rect);
            }
        }
    }
}
/// This is pretty self explanatory, returns a boolean that
/// indicates start->end line of sight exists
bool map_has_line_of_sight(const Map *map, Vector2f start, Vector2f end){
    const float map_pixel_width = map->width * TILE_SIZE;
    const float map_pixel_height = map->height * TILE_SIZE;

    Vector2f dir = vector_subtract(end, start);
    float line_length = vector_magnitude(dir);
    dir = vector_normalize(dir);

    for (float i = 0; i < line_length; i += TILE_SIZE / 2.0f) {
        float check_x = start.x + dir.x * i;
        float check_y = start.y + dir.y * i;

        int grid_x = check_x / TILE_SIZE;
        int grid_y = check_y / TILE_SIZE;

        if (grid_x < 0 || grid_x >= map->width || grid_y < 0 || grid_y >= map->height) {
            return false;
        }

        if (map->tiles[grid_y][grid_x] == 1) {
            return false;
        }
    }
    return true;
}

/// Returns a random empty tile, useful for enemy patrolling
Vector2f map_get_random_walkable_tile(const Map* map) {
    int x, y;
    int safety_counter = 0;

    // FIX: Use a do-while loop to ensure x and y are initialized before use.
    do {
        x = rand() % map->width;
        y = rand() % map->height;
        safety_counter++;
    } while (map->tiles[y][x] == 1 && safety_counter < 1000);

    const float map_pixel_width = map->width * TILE_SIZE;
    const float map_pixel_height = map->height * TILE_SIZE;

    return (Vector2f){
         (x * TILE_SIZE) + (TILE_SIZE / 2.0f),
         (y * TILE_SIZE) + (TILE_SIZE / 2.0f)
    };
}

void map_destroy(Map* map) {
    if (map->tiles) {
        for (int i = 0; i < map->height; i++) {
            free(map->tiles[i]);
        }
        free(map->tiles);
    }
}

