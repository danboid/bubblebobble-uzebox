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

// Level management
void load_level(GameContext* ctx, uint8_t level_index);

// Player functions
void player_update(Player* player);
void player_render(Player* player);
void player_shoot(Player* player);

// Enemy functions
void enemy_update(Enemy* enemy);
void enemy_render(Enemy* enemy);
void enemy_capture(Enemy* enemy);
void enemy_release(Enemy* enemy);
void enemy_die(Enemy* enemy);

// Bubble functions
void bubble_update(Bubble* bubble);
void bubble_render(Bubble* bubble);
void bubble_start_pop(Bubble* bubble);
void bubble_capture_enemy(Bubble* bubble, uint8_t enemy_index);
void bubble_apply_attraction(Bubble* bubble);

#endif // GAME_H