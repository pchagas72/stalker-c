#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "../map/map.h"

// New function to handle the animation logic each frame
void dialogue_update();

void dialogue_start_conversation(char** lines, int line_count);
void dialogue_advance();
void dialogue_end_conversation();
void dialogue_render(SDL_Renderer* renderer);
bool dialogue_is_active();

#endif // DIALOGUE_H
