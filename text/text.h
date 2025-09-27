#ifndef TEXT_H
#define TEXT_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

void text_init();
void text_render(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
void text_render_wrapped(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, int wrap_length);
void text_quit();

#endif // TEXT_H
