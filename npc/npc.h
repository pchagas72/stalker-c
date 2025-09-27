#ifndef NPC_H
#define NPC_H

#include <SDL3/SDL.h>
#include "../camera/camera.h"
#include "../map/map.h" // This safely includes NPCData and the dialogue constants

// Note: We no longer need to forward-declare NPCData

typedef struct {
    SDL_FRect rect;
    char id[4];
    char* dialogue_lines[MAX_DIALOGUE_LINES];
    int dialogue_line_count;
} NPC;

void npc_create(NPC* npc, const NPCData* data);
void npc_render(NPC* npc, SDL_Renderer* renderer, const Camera* camera);
void npc_destroy(NPC* npc);

#endif // NPC_H
