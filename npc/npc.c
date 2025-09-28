#include "npc.h"
#include <string.h>
#include <stdlib.h>

void npc_create(NPC* npc, const NPCData* data) {
    strcpy(npc->id, data->id);
    npc->rect.w = data->size.x;
    npc->rect.h = data->size.y;
    npc->rect.x = data->spawn_pos.x * TILE_SIZE;
    npc->rect.y = data->spawn_pos.y * TILE_SIZE;

    npc->dialogue_line_count = data->dialogue_line_count;
    for (int i = 0; i < npc->dialogue_line_count; ++i) {
        // strdup allocates memory and copies the string.
        npc->dialogue_lines[i] = strdup(data->dialogue_lines[i]);
    }
}

void npc_destroy(NPC* npc) {
    for (int i = 0; i < npc->dialogue_line_count; ++i) {
        free(npc->dialogue_lines[i]);
    }
}

void npc_render(NPC* npc, SDL_Renderer* renderer, const Camera* camera) {
    SDL_FRect render_rect = {
        .x = npc->rect.x - camera->x,
        .y = npc->rect.y - camera->y,
        .w = npc->rect.w,
        .h = npc->rect.h
    };

    SDL_SetRenderDrawColor(renderer, 0, 150, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &render_rect);
}
