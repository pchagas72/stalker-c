#include "dialogue.h"
#include "../text/text.h"
#include <string.h> // Needed for strlen and strncpy

#define TEXT_SPEED 40 // Milliseconds per character

// --- New static variables for animation ---
static char** current_conversation = NULL;
static int total_lines = 0;
static int current_line_index = 0;

static int visible_chars = 0; // How many characters are currently visible
static Uint64 last_char_time = 0; // When the last character was shown

// --- New Update Function ---
// This will be called every frame to advance the animation.
void dialogue_update() {
    if (!dialogue_is_active()) return;

    // Get the full text of the current line
    const char* full_line = current_conversation[current_line_index];
    int line_length = strlen(full_line);

    // If not all characters are visible yet...
    if (visible_chars < line_length) {
        Uint64 current_time = SDL_GetTicks();
        // and if enough time has passed since the last character...
        if (current_time > last_char_time + TEXT_SPEED) {
            visible_chars++; // reveal one more character.
            last_char_time = current_time; // reset the timer.
        }
    }
}


void dialogue_start_conversation(char** lines, int line_count) {
    if (lines && line_count > 0) {
        current_conversation = lines;
        total_lines = line_count;
        current_line_index = 0;

        // Reset animation state for the new conversation
        visible_chars = 0;
        last_char_time = SDL_GetTicks();
    }
}

void dialogue_advance() {
    if (!dialogue_is_active()) return;

    const char* full_line = current_conversation[current_line_index];
    int line_length = strlen(full_line);

    // If the line is still animating, pressing 'E' should reveal it instantly.
    if (visible_chars < line_length) {
        visible_chars = line_length;
    } else { 
        // If the line is already fully visible, advance to the next one.
        current_line_index++;
        if (current_line_index >= total_lines) {
            dialogue_end_conversation();
        } else {
            // Reset animation for the new line
            visible_chars = 0;
            last_char_time = SDL_GetTicks();
        }
    }
}

void dialogue_end_conversation() {
    current_conversation = NULL;
    total_lines = 0;
    current_line_index = 0;
    visible_chars = 0;
}

void dialogue_render(SDL_Renderer* renderer) {
    if (dialogue_is_active()) {
        // ... (dialogue box rendering is unchanged) ...
        SDL_FRect dialogue_box = { .x = 20, .y = 130, .w = 320 - 40, .h = 40 };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(renderer, &dialogue_box);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &dialogue_box);

        SDL_Color white = {255, 255, 255, 255};
        int wrap_width = 1100; 

        // --- THE CHANGE ---
        // We now render a *substring* of the full line.
        const char* full_line = current_conversation[current_line_index];
        
        // Create a temporary buffer to hold the visible part of the text
        char render_buffer[MAX_DIALOGUE_LINE_LENGTH];
        strncpy(render_buffer, full_line, visible_chars);
        render_buffer[visible_chars] = '\0'; // Null-terminate the string!

        text_render_wrapped(renderer, render_buffer, 30, 140, white, wrap_width);
    }
}

bool dialogue_is_active() {
    return current_conversation != NULL;
}
