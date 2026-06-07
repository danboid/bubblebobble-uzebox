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
void player_init(Player* player, uint8_t player_index);
void player_update(Player* player);
void player_render(const Player* player);
void player_shoot(Player* player);
void player_take_damage(Player* player);
void player_handle_input(Player* player, uint8_t buttons, uint8_t prev_buttons);
void player_physics_update(Player* player, GameContext* ctx);
void player_update_death(Player* player);
void player_update_invulnerability(Player* player);
void player_fire_bubble(Player* player);

// Enemy functions
void enemy_update(Enemy* enemy);
void enemy_update_ai(Enemy* enemy, const Player* player);
void enemy_render(const Enemy* enemy);
void enemy_capture(Enemy* enemy);
void enemy_release(Enemy* enemy);
void enemy_die(Enemy* enemy);
void enemy_update_walking(Enemy* enemy);
void enemy_update_falling(Enemy* enemy);
void enemy_update_jumping(Enemy* enemy);
void enemy_update_animation(Enemy* enemy);
void spawn_dead_enemy(int16_t x, int16_t y, uint8_t pickup_type);

// Bubble functions
void bubble_update(Bubble* bubble);
void bubble_render(Bubble* bubble);
void bubble_start_pop(Bubble* bubble);
void bubble_capture_enemy(Bubble* bubble, uint8_t enemy_index);
void bubble_apply_attraction(Bubble* bubble);
void bubble_force_pop(Bubble* bubble);
void bubble_update_normal(Bubble* bubble);
void bubble_update_floating(Bubble* bubble);
void bubble_update_popping(Bubble* bubble);
void update_bubbles(void);

// Collision functions
void check_player_enemy_collisions(void);
void check_player_bubble_collisions(void);
void check_player_pickup_collisions(void);
void update_dead_enemies(void);
void run_collision_checks(void);

// Spawn functions
void spawn_pickup(int16_t x, int16_t y, uint8_t type);
void spawn_score_popup(int16_t x, int16_t y, uint16_t score);

#endif // GAME_H