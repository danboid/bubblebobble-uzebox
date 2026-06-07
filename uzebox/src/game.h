/*
 * BubbleBobble Uzebox - Game Module Header
 *
 * External declarations for game context and functions
 */

#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "config.h"

// =============================================================================
// EXTERNAL DECLARATIONS
// =============================================================================

extern GameContext game;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Game state management
void game_init(uint8_t player_count);
void game_update(void);
void game_render(void);
void game_handle_input(void);

// Level management
void load_level(uint8_t level_index);
void next_level(void);
void reset_level(void);

// =============================================================================
// GAME STATES
// =============================================================================

typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_LEVEL_COMPLETE,
    STATE_GAME_OVER,
    STATE_PAUSED
} GameState;

#endif // GAME_H