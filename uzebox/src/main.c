/*
 * BubbleBobble Uzebox - Main Entry Point
 *
 * Uzebox game entry point using kernel callbacks
 */

#include <avr/pgmspace.h>
#include <uzebox.h>
#include "game.h"

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