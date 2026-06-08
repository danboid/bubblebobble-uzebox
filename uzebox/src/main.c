/*
 * BubbleBobble Uzebox - Main Entry Point
 *
 * Uzebox Mode 3 Game
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

    // Main game loop - like the stopwatch example
    while (1) {
        WaitVsync(1);  // Sync to ~60 FPS
        
        // Call GameLoop directly
        GameLoop();
    }
}