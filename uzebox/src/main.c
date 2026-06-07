/*
 * BubbleBobble Uzebox - Main Entry Point
 *
 * Uzebox game entry point using kernel callbacks
 */

#include <avr/pgmspace.h>
#include "game.h"

// =============================================================================
// GAME THREAD (Called by kernel at ~60 FPS)
// =============================================================================

void GameLoop(void) {
    // Process input
    game_handle_input();

    // Update game logic
    game_update();

    // Render to screen
    game_render();
}

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int main(void) {
    // Initialize video mode
    InitializeVideoMode();

    // Initialize game with 1 player
    game_init(1);

    // Main game loop - kernel calls GameLoop() at ~60 FPS
    while (1) {
        // Wait for vertical sync
        WaitVBlank();
    }

    return 0;
}