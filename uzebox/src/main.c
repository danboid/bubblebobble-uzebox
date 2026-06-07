/*
 * BubbleBobble Uzebox - Main Entry Point
 *
 * Uzebox kernel entry point - GameLoop() is called by kernel at 60 FPS
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

    // Set up tile table and sprites - CRITICAL for video output!
    SetTileTable(tileset);
    SetSpritesTileTable(tileset);

    // Clear VRAM
    ClearVram();

    // Initialize game with 1 player
    game_init(1);

    // Main loop - call WaitVsync to sync with vertical blank
    while (1) {
        WaitVsync(1);
    }
}