#include "text.h"
#include <SDL3/SDL_error.h>
#include <stdio.h>

static TTF_Font* font = NULL;

void text_init() {
    if (!TTF_Init()) {
        printf("Failed to initialize SDL_ttf\n");
        return;
    }

    font = TTF_OpenFont("res/PublicPixel.ttf", 8*4); // Load font at size 16
    if (!font) {
        printf("Failed to load font\n");
    }
}

void text_render(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color) {
    if (font == NULL) return;

    SDL_Surface* surface = TTF_RenderText_Solid(font, text,0, color);
    if (surface == NULL) {
        printf("Failed to render text surface: %s\n", SDL_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("Failed to create texture from surface: %s\n", SDL_GetError());
        SDL_DestroySurface(surface);
        return;
    }

    const float scale = 4.0f;

    SDL_FRect dest_rect = { x, y, surface->w/scale, surface->h/scale };

    SDL_RenderTexture(renderer, texture, NULL, &dest_rect);

    // Clean up
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void text_render_wrapped(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, int wrap_length) {
    if (font == NULL) return;

    SDL_Surface* surface = TTF_RenderText_Solid_Wrapped(font, text, 0, color, wrap_length);
    if (surface == NULL) {
        printf("Failed to render wrapped text surface: %s\n", SDL_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("Failed to create texture from surface: %s\n", SDL_GetError());
        SDL_DestroySurface(surface);
        return;
    }

    const float scale = 4.0f;
    // We scale the final rectangle just like before.
    SDL_FRect dest_rect = { x, y, surface->w / scale, surface->h / scale };

    SDL_RenderTexture(renderer, texture, NULL, &dest_rect);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void text_quit() {
    TTF_CloseFont(font);
    TTF_Quit();
}
