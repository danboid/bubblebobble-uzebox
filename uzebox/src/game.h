/*
 * BubbleBobble Uzebox - Game Module Header
 *
 * External declarations for game context and functions
 */

#ifndef GAME_H
#define GAME_H

#include "types.h"

// =============================================================================
// EXTERNAL DECLARATIONS
// =============================================================================

extern GameContext game;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Game initialization
void game_init(uint8_t player_count);

// Game main loop
void game_update(void);
void game_render(void);
void game_handle_input(void);

#endif // GAME_H