#ifndef GAME_H
#define GAME_H

typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_DIALOGUE,
    GAME_STATE_PAUSE
} GameState;

// A global variable to hold the current game state
extern GameState current_game_state;

#endif // GAME_H
