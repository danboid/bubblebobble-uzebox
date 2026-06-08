/*
 * BubbleBobble Uzebox - Main Entry Point
 *
 * Uzebox Mode 3 Game
 */

#include <uzebox.h>
#include <avr/pgmspace.h>
#include "game.h"
#include "tileset.inc"

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int main(void) {
    // Initialize video mode
    InitializeVideoMode();

    // Set up tile table (font tiles are included in tileset when FONT_TILES=1)
    SetTileTable(tileset);

    // Clear VRAM
    ClearVram();

    // Main game loop
    while (1) {
        WaitVsync(1);
        GameLoop();
    }
}