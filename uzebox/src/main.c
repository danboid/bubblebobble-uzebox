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

    // Start the kernel - it will call GameLoop() at ~60 FPS
    // GameLoop() is defined in game.c
    StartMusicPlayer(0);
    
    while (1) {
        // Kernel handles game loop via GameLoop() callback
    }

    return 0;
}