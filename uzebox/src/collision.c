/*
 * BubbleBobble Uzebox - Collision Detection
 * 
 * Handles all entity-entity and entity-tile collisions
 */

#include "types.h"
#include "config.h"
#include "level_data.h"
#include "game.h"

// =============================================================================
// TILE COLLISION HELPERS
// =============================================================================

// Check if entity can stand at position (for one-way platforms)
uint8_t can_stand_at(int16_t x, int16_t y, uint8_t width) {
    int8_t grid_x, grid_y;
    
    // Check bottom-center of entity
    world_to_grid(x + width/2, y + 1, &grid_x, &grid_y);
    
    if (is_tile_semi_solid( grid_x, grid_y)) {
        return 1;
    }
    
    return 0;
}

// Get ground height at position
int16_t get_ground_height(int16_t x, uint8_t width) {
    int8_t grid_x, grid_y;
    
    world_to_grid(x + width/2, 0, &grid_x, &grid_y);
    
    // Scan downward for ground
    for (int8_t y = grid_y; y < GRID_SIZE_Y; y++) {
        if (is_tile_solid( grid_x, y)) {
            return y * TILE_HEIGHT;
        }
        if (is_tile_semi_solid( grid_x, y)) {
            return y * TILE_HEIGHT;
        }
    }
    
    return GRID_SIZE_Y * TILE_HEIGHT;  // Off screen = fall
}

// =============================================================================
// PLAYER-ENEMY COLLISION
// =============================================================================

void check_player_enemy_collisions(void) {
    for (uint8_t p = 0; p < game.player_count; p++) {
        Player* player = &game.players[p];
        
        if (player->state == PLAYER_DEAD || player->state == PLAYER_BUBBLE) continue;
        if (player->invulnerable) continue;
        
        for (uint8_t e = 0; e < game.enemy_count; e++) {
            Enemy* enemy = &game.enemies[e];
            
            if (!ENTITY_IS_ACTIVE(&enemy->base)) continue;
            if (enemy->captured) continue;
            
            if (entities_collide(&player->base, &enemy->base)) {
                player_take_damage(player);
            }
        }
    }
}

// =============================================================================
// PLAYER-BUBBLE COLLISION
// =============================================================================

void check_player_bubble_collisions(void) {
    for (uint8_t p = 0; p < game.player_count; p++) {
        Player* player = &game.players[p];
        
        if (player->state == PLAYER_DEAD || player->state == PLAYER_BUBBLE) continue;
        if (player->invulnerable) continue;
        
        for (uint8_t b = 0; b < 8; b++) {
            Bubble* bubble = &game.bubbles[b];
            
            if (!ENTITY_IS_ACTIVE(&bubble->base)) continue;
            if (bubble->captured_enemy != 0xFF) continue;  // Don't hit own bubble
            
            if (entities_collide(&player->base, &bubble->base)) {
                // Pop the bubble
                bubble_force_pop(bubble);
            }
        }
    }
}

// =============================================================================
// BUBBLE-ENEMY COLLISION
// =============================================================================

void check_bubble_enemy_collisions(void) {
    for (uint8_t b = 0; b < 8; b++) {
        Bubble* bubble = &game.bubbles[b];
        
        if (!ENTITY_IS_ACTIVE(&bubble->base)) continue;
        if (bubble->captured_enemy != 0xFF) continue;  // Already captured
        
        for (uint8_t e = 0; e < game.enemy_count; e++) {
            Enemy* enemy = &game.enemies[e];
            
            if (!ENTITY_IS_ACTIVE(&enemy->base)) continue;
            if (enemy->captured) continue;
            
            if (entities_collide(&bubble->base, &enemy->base)) {
                bubble_capture_enemy(bubble, e);
                break;
            }
        }
    }
}

// =============================================================================
// BUBBLE-TILE COLLISION
// =============================================================================

void check_bubble_tile_collisions(void) {
    for (uint8_t b = 0; b < 8; b++) {
        Bubble* bubble = &game.bubbles[b];
        
        if (!ENTITY_IS_ACTIVE(&bubble->base)) continue;
        if (bubble->state == BUBBLE_FLOATING) continue;
        
        int8_t grid_x, grid_y;
        
        // Check all corners
        world_to_grid(bubble->base.x, bubble->base.y, &grid_x, &grid_y);
        
        // Bottom collision
        world_to_grid(bubble->base.x, bubble->base.y + bubble->base.height, &grid_x, &grid_y);
        if (is_tile_solid( grid_x, grid_y)) {
            bubble->base.y = grid_y * TILE_HEIGHT - bubble->base.height;
            bubble->base.vy = -bubble->base.vy / 2;
            if (bubble->base.vy > -2) bubble->base.vy = -2;
        }
        
        // Side collisions
        world_to_grid(bubble->base.x, bubble->base.y + bubble->base.height/2, &grid_x, &grid_y);
        if (bubble->base.vx > 0 && is_tile_solid( grid_x + 1, grid_y)) {
            bubble->base.x = grid_x * TILE_WIDTH - bubble->base.width;
            bubble->base.vx = -bubble->base.vx / 2;
        }
        if (bubble->base.vx < 0 && is_tile_solid( grid_x - 1, grid_y)) {
            bubble->base.x = (grid_x + 1) * TILE_WIDTH;
            bubble->base.vx = -bubble->base.vx / 2;
        }
    }
}

// =============================================================================
// PLAYER-PICKUP COLLISION
// =============================================================================

void check_player_pickup_collisions(void) {
    for (uint8_t p = 0; p < game.player_count; p++) {
        Player* player = &game.players[p];
        
        if (player->state == PLAYER_DEAD || player->state == PLAYER_BUBBLE) continue;
        
        for (uint8_t i = 0; i < game.pickup_count; i++) {
            Pickup* pickup = &game.pickups[i];
            
            if (!pickup->active) continue;
            
            if (entities_collide(&player->base, &pickup->base)) {
                // Collect pickup
                player->score += (pickup->type == PICKUP_WATERMELON) ? 100 : 200;
                pickup->active = 0;
                
                // Show score popup
                spawn_score_popup(pickup->base.x, pickup->base.y, 
                                 (pickup->type == PICKUP_WATERMELON) ? 100 : 200);
            }
        }
    }
}

// =============================================================================
// DEAD ENEMY (FRUIT) UPDATE
// =============================================================================

void spawn_dead_enemy(int16_t x, int16_t y, uint8_t pickup_type) {
    if (game.dead_enemy_count >= 4) return;
    
    DeadEnemy* dead = &game.dead_enemies[game.dead_enemy_count];
    
    dead->base.flags = FLAG_ACTIVE;
    dead->base.x = x;
    dead->base.y = y;
    dead->base.width = 12;
    dead->base.height = 12;
    dead->pickup_type = pickup_type;
    dead->lifetime = 120;  // 2 seconds
    
    // Random velocity (flying up and outward)
    int8_t dir = (x & 0x01) ? 1 : -1;
    dead->base.vx = dir * 4;
    dead->base.vy = -6;
    
    // Sprite
    dead->base.sprite_base = (pickup_type == PICKUP_WATERMELON) ? TILE_WATERMELON : TILE_FRIES;
    
    game.dead_enemy_count++;
}

void update_dead_enemies(void) {
    for (uint8_t i = 0; i < game.dead_enemy_count; i++) {
        DeadEnemy* dead = &game.dead_enemies[i];
        
        if (!ENTITY_IS_ACTIVE(&dead->base)) continue;
        
        // Apply physics
        dead->base.vy += 1;  // Gravity
        dead->base.x += dead->base.vx;
        dead->base.y += dead->base.vy;
        
        // Ground collision
        int8_t grid_x, grid_y;
        world_to_grid(dead->base.x, dead->base.y + dead->base.height, &grid_x, &grid_y);
        
        if (is_tile_solid( grid_x, grid_y) || grid_y >= GRID_SIZE_Y) {
            dead->base.y = grid_y * TILE_HEIGHT - dead->base.height;
            dead->base.vy = -dead->base.vy / 2;
            dead->base.vx = (dead->base.vx * 9) / 10;
            
            if (dead->base.vy < 1) {
                // Landed - become pickup
                spawn_pickup(dead->base.x, dead->base.y, dead->pickup_type);
                dead->base.flags &= ~FLAG_ACTIVE;
                game.dead_enemy_count--;
            }
        }
        
        // Lifetime
        dead->lifetime--;
        if (dead->lifetime == 0) {
            dead->base.flags &= ~FLAG_ACTIVE;
            game.dead_enemy_count--;
        }
    }
}

// =============================================================================
// PICKUP SPAWN
// =============================================================================

void spawn_pickup(int16_t x, int16_t y, uint8_t type) {
    if (game.pickup_count >= 4) return;
    
    Pickup* pickup = &game.pickups[game.pickup_count];
    
    pickup->base.flags = FLAG_ACTIVE;
    pickup->base.x = x;
    pickup->base.y = y;
    pickup->base.width = 12;
    pickup->base.height = 12;
    pickup->type = type;
    pickup->active = 1;
    pickup->base.sprite_base = (type == PICKUP_WATERMELON) ? TILE_WATERMELON : TILE_FRIES;
    
    game.pickup_count++;
}

// =============================================================================
// SCORE POPUP (Temporary display)
// =============================================================================

// Simple score popup system - just add to score display queue
void spawn_score_popup(int16_t x, int16_t y, uint16_t score) {
    // For now, just add to player score
    // Score popup rendering handled by UI
}

// =============================================================================
// RUN ALL COLLISION CHECKS
// =============================================================================

void run_collision_checks(void) {
    check_player_enemy_collisions();
    check_player_bubble_collisions();
    check_bubble_enemy_collisions();
    check_bubble_tile_collisions();
    check_player_pickup_collisions();
    update_dead_enemies();
}