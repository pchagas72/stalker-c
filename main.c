#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <stdio.h>
#include "defs/defs.h"
#include "map/map.h"
#include "player/player.h"
#include "enemies/enemy.h"
#include "npc/npc.h"
#include "camera/camera.h"
#include "game/game.h"
#include "text/text.h"
#include "dialogue/dialogue.h"


// SDL variables
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

// Game variables
GameState current_game_state;
static Camera camera;
Map current_level_map;
Player player;
Enemy enemies[MAX_ENEMIES];
int active_enemy_count = 0;
NPC npcs[MAX_NPCS];
int active_npc_count = 0;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv){
    printf("[SDL] Initializing SDL");
    SDL_SetAppMetadata("Example Renderer Points", "1.0", "com.example.renderer-points");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    printf("[SDL] Initialized video subsystem.\n");

    window = SDL_CreateWindow("SDL3 gam", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window){
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    renderer = SDL_CreateRenderer(window, NULL);

    text_init();

    if (!renderer) {
        SDL_Log("Error creating renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    printf("[SDL] Created window and renderer objects.\n");

    printf("[GAME] Initializing game objects.\n");

    map_load_from_file(&current_level_map, "level.txt");
    player_create(&player, &current_level_map);

    SDL_SetRenderLogicalPresentation(renderer, 320, 180, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    active_enemy_count = current_level_map.enemy_count;
    for (int i = 0 ; i < active_enemy_count; ++i){
        if (current_level_map.enemies[i].has_spawned) {
            printf("Spawning enemy: %s\n", current_level_map.enemies[i].id);
            enemy_create(&enemies[i], &current_level_map.enemies[i], &current_level_map);
        } else {
            printf("Warning: Enemy '%s' was defined but not placed on the map.\n", current_level_map.enemies[i].id);
        }
    }

    active_npc_count = current_level_map.npc_count;
    for (int i = 0; i < active_npc_count; ++i) {
        if (current_level_map.npcs[i].has_spawned) {
            printf("Spawning NPC: %s\n", current_level_map.npcs[i].id);
            npc_create(&npcs[i], &current_level_map.npcs[i]);
        } else {
             printf("Warning: NPC '%s' was defined but not placed on the map.\n", current_level_map.npcs[i].id);
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_E) {
            if (dialogue_is_active()) {
                dialogue_advance();
            } else {
                Vector2f player_center = { player.rect.x + player.rect.w / 2.0f, player.rect.y + player.rect.h / 2.0f };
                for (int i = 0; i < active_npc_count; ++i) {
                    Vector2f npc_center = { npcs[i].rect.x + npcs[i].rect.w / 2.0f, npcs[i].rect.y + npcs[i].rect.h / 2.0f };
                    float distance = vector_magnitude(vector_subtract(player_center, npc_center));
                    if (distance < 50.0f && map_has_line_of_sight(&current_level_map, player_center, npc_center)) {
                        dialogue_start_conversation(npcs[i].dialogue_lines, npcs[i].dialogue_line_count);
                        break;
                    }
                }
            }
        } 
        // Handle pausing separately
        else if (event->key.key == SDLK_ESCAPE) {
            if (current_game_state != GAME_STATE_PAUSE){
                current_game_state = GAME_STATE_PAUSE;
            } else if (current_game_state == GAME_STATE_PAUSE){
                current_game_state = GAME_STATE_PLAYING;
            }
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    const Uint64 frame_start_time = SDL_GetTicks();
    const bool* keyboard_state = SDL_GetKeyboardState(NULL);

    if (dialogue_is_active()) {
        current_game_state = GAME_STATE_DIALOGUE;
    } else if (current_game_state == GAME_STATE_DIALOGUE) {
        // This check prevents us from getting stuck in the dialogue state
        // after a conversation ends.
        current_game_state = GAME_STATE_PLAYING;
    }

    switch (current_game_state) {
        case GAME_STATE_PLAYING:
            player_update(&player, keyboard_state, &current_level_map);
            for (int i = 0; i < active_enemy_count; ++i){
                enemy_update(&enemies[i], &player, &current_level_map);
            }
            break;
        case GAME_STATE_DIALOGUE:
            dialogue_update();
            break;
        case GAME_STATE_PAUSE:
            break;
    }

    camera_update(&camera, &player, &current_level_map, 2.5);

    // --- Rendering
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Render the game world
    map_render(&current_level_map, renderer, &camera);
    player_render(&player, renderer, &camera);
    for (int i = 0; i < active_enemy_count; ++i) {
        enemy_render(&enemies[i], renderer, &camera);
    }
    for (int i = 0; i < active_npc_count; ++i) {
        npc_render(&npcs[i], renderer, &camera);
    }

    if (dialogue_is_active()) {
        dialogue_render(renderer);
    }

    if (current_game_state == GAME_STATE_PAUSE) {
        SDL_Color white = {255, 255, 255, 255};
        
        // --- THE NEW PART ---
        // Create a semi-transparent black rectangle for the background
        SDL_FRect paused_textbox = {
            .w = (8) * 8,
            .h = 8 + (8 * 2),
        };
        paused_textbox.x = (320 / 2) - (paused_textbox.w / 2);
        paused_textbox.y = (180 / 2) - (paused_textbox.h / 2);

        // Set the blend mode to allow for transparency
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        // Set the draw color to black with 50% opacity
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 240);
        // Draw the filled rectangle
        SDL_RenderFillRect(renderer, &paused_textbox);
        // Set the blend mode back to none for the text
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        
        SDL_FRect paused_textbox_rect = {};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        paused_textbox_rect.h = 8+(8*2);
        paused_textbox_rect.w = (8)*(6+2);
        paused_textbox_rect.x = (320/2)-(paused_textbox_rect.w)/2;
        paused_textbox_rect.y = (180/2)-(paused_textbox_rect.h)/2;
        SDL_RenderRect(renderer, &paused_textbox_rect);

        // Now, render the text on top
        text_render(renderer, "PAUSED", (320 / 2) - ((8 * 6) / 2), (180 / 2) - (8 / 2), white);
    }

    // Render everything
    SDL_RenderPresent(renderer);

    const Uint64 frame_time = SDL_GetTicks() - frame_start_time;
    if (frame_time < FRAME_DELAY) {
        SDL_Delay(FRAME_DELAY - frame_time);
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    // --- Memory Cleanup ---
    // Call the destroy function for each NPC to free the dialogue memory.
    for (int i = 0; i < active_npc_count; ++i) {
        npc_destroy(&npcs[i]);
    }
    text_quit();
}

