// stalker-c/map/map.h

#ifndef MAP_H
#define MAP_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_oldnames.h>
#include <stdbool.h>
#include <string.h>
#include "../defs/defs.h"
#include "../helper/vector.h"

//#define MAP_GRID_WIDTH   40
//#define MAP_GRID_HEIGHT  30
#define TILE_SIZE        16
#define MAX_ENEMIES      10
#define MAX_NPCS         10 // New: Max number of NPCs

// Moved dialogue defines here to break the circular dependency
#define MAX_DIALOGUE_LINES 10
#define MAX_DIALOGUE_LINE_LENGTH 128

typedef enum {
    STATE_UNKNOWN,
    STATE_PARSING_ENEMY,
    STATE_PARSING_NPC,
    STATE_PARSING_MAP,
} ParserState;

// Holds enemy data
typedef struct {
    char id[4];       // e.g., "E1", "E2"
    Vector2f size;
    Vector2 spawn_pos;
    bool has_spawned;

    float sight_range;
    float perception_radius;
    float attack_range;

    float walking_speed;
    float stalking_speed;
    float attacking_speed;
} EnemyData;


// The NPCData struct now uses the defines from this file
typedef struct NPCData {
    char id[4];
    Vector2f size;
    Vector2 spawn_pos;
    bool has_spawned;
    char dialogue_lines[MAX_DIALOGUE_LINES][MAX_DIALOGUE_LINE_LENGTH];
    int dialogue_line_count;
} NPCData;

typedef struct {
    int **tiles;
    int width;
    int height;
    Vector2 playerSpawn;
    EnemyData enemies[MAX_ENEMIES];
    int enemy_count;
    NPCData npcs[MAX_NPCS]; // New: Array to hold parsed NPC data
    int npc_count;          // New: Counter for parsed NPCs
} Map;


// Parser that reads every map file
typedef struct {
    ParserState state;
    int line_number; 
    char current;
    char peek;
} Parser;

// --- Public Functions ---
void map_load_from_file(Map* map, const char* filename);
void map_render(Map* map, SDL_Renderer* renderer, const SDL_FRect *camera);
bool map_has_line_of_sight(const Map *map, Vector2f start, Vector2f end);
Vector2f map_get_random_walkable_tile(const Map* map);
void map_destroy(Map* map);

#endif // MAP_H
