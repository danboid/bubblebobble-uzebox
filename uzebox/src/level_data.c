/*
 * BubbleBobble Uzebox - Level Data Implementation
 *
 * Level loading functions
 */

#include <uzebox.h>
#include "types.h"
#include "config.h"
#include "level_data.h"

// =============================================================================
// LEVEL FUNCTIONS
// =============================================================================

// Load level data from flash into VRAM and GameContext
void load_level(GameContext* ctx, uint8_t level_index) {
    const uint8_t* data = pgm_read_word(&level_data[level_index]);

    // Load 32x28 tiles (width x height) directly to VRAM
    for (uint8_t y = 0; y < 28; y++) {
        for (uint8_t x = 0; x < 32; x++) {
            uint8_t tile = pgm_read_byte(data++);
            vram[(y * VRAM_TILES_H) + x] = tile;
        }
    }

    // Load enemy spawn data
    const uint8_t* enemy_data = pgm_read_word(&level_enemies[level_index]);
    ctx->enemy_count = pgm_read_byte(enemy_data++);

    for (uint8_t i = 0; i < ctx->enemy_count && i < 8; i++) {
        ctx->enemies[i].base.sprite_base = pgm_read_byte(enemy_data++);
        ctx->enemies[i].base.x = pgm_read_byte(enemy_data++) * 8;
        ctx->enemies[i].base.y = pgm_read_byte(enemy_data++) * 8;
        ctx->enemies[i].base.flags = FLAG_ACTIVE;
    }
}