/*
 * BubbleBobble Uzebox - Main Entry Point
 *
 * Uzebox kernel entry point - GameLoop() is called by kernel at 60 FPS
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

    // Main loop - kernel calls GameLoop() at ~60 FPS
    while (1) {
        // Do nothing - kernel handles the game loop
    }
}