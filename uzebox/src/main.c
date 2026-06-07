/*
 * BubbleBobble Uzebox - Main Entry Point
 *
 * Based on the stopwatch example pattern
 */

#include <avr/pgmspace.h>
#include <uzebox.h>
#include "game.h"
#include "tileset.inc"

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int main(void) {
    // Initialize video mode
    InitializeVideoMode();

    // Set up tile table and sprites
    SetTileTable(tileset);
    SetSpritesTileTable(tileset);

    // Clear VRAM
    ClearVram();

    // Initialize game with 1 player
    game_init(1);

    // Main game loop - like the stopwatch example
    while (1) {
        WaitVsync(1);  // Sync to ~60 FPS
        
        // Call GameLoop directly
        GameLoop();
    }
}