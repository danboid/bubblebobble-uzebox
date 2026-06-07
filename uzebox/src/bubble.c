/*
 * BubbleBobble Uzebox - Bubble Projectile System
 * 
 * Bubbles capture enemies and can pop to release them as fruit
 */

#include "types.h"
#include "config.h"
#include "level_data.h"
#include "game.h"

// =============================================================================
// BUBBLE INITIALIZATION
// =============================================================================

void bubble_init(Bubble* bubble, int16_t x, int16_t y, int8_t vx, int8_t vy) {
    bubble->base.flags = FLAG_ACTIVE;
    bubble->base.x = x;
    bubble->base.y = y;
    bubble->base.width = 12;
    bubble->base.height = 12;
    bubble->base.vx = vx;
    bubble->base.vy = vy;
    bubble->base.sprite_base = TILE_BUBBLE_1;
    bubble->base.frame = 0;
    bubble->base.frame_timer = 0;
    
    bubble->lifetime = BUBBLE_LIFETIME;
    bubble->state = BUBBLE_NORMAL;
    bubble->captured_enemy = 0xFF;  // No enemy captured
    bubble->float_timer = 0;
}

// =============================================================================
// SPAWN BUBBLE
// =============================================================================

Bubble* spawn_bubble(int16_t x, int16_t y, int8_t vx, int8_t vy) {
    // Find inactive bubble
    for (uint8_t i = 0; i < 8; i++) {
        if (!ENTITY_IS_ACTIVE(&game.bubbles[i].base)) {
            bubble_init(&game.bubbles[i], x, y, vx, vy);
            game.bubble_count++;
            return &game.bubbles[i];
        }
    }
    return 0;  // No slots available
}

// Fire bubble from player
void player_fire_bubble(Player* player) {
    if (player->attack_cooldown > 0) return;
    
    int16_t x = player->base.x + (player->base.direction ? player->base.width : -BUBBLE_SIZE);
    int16_t y = player->base.y;
    int8_t vx = player->base.direction ? BUBBLE_SPEED : -BUBBLE_SPEED;
    
    Bubble* bubble = spawn_bubble(x, y, vx, 0);
    if (bubble) {
        player->attack_cooldown = 24;  // Cooldown
    }
}

// =============================================================================
// BUBBLE UPDATE
// =============================================================================

void bubble_update(Bubble* bubble) {
    if (!ENTITY_IS_ACTIVE(&bubble->base)) return;
    
    // Decrease lifetime
    bubble->lifetime--;
    if (bubble->lifetime == 0) {
        bubble_start_pop(bubble);
        return;
    }
    
    // Animation
    bubble->base.frame_timer++;
    if (bubble->base.frame_timer >= 6) {
        bubble->base.frame_timer = 0;
        bubble->base.frame ^= 1;
    }
    bubble->base.sprite_base = TILE_BUBBLE_1 + bubble->base.frame;
    
    switch (bubble->state) {
        case BUBBLE_NORMAL:
            bubble_update_normal(bubble);
            break;
        case BUBBLE_FLOATING:
            bubble_update_floating(bubble);
            break;
        case BUBBLE_POPPING:
            bubble_update_popping(bubble);
            break;
    }
}

void bubble_update_normal(Bubble* bubble) {
    // Apply velocity with drag
    bubble->base.x += bubble->base.vx;
    bubble->base.y += bubble->base.vy;
    
    // Drag
    bubble->base.vx = (bubble->base.vx * 7) / 8;
    bubble->base.vy = (bubble->base.vy * 7) / 8;
    
    // Float timer
    bubble->float_timer++;
    if (bubble->float_timer > 60) {  // 1 second
        bubble->state = BUBBLE_FLOATING;
        bubble->float_timer = 0;
    }
    
    // Check for enemy collision (only if not captured)
    if (bubble->captured_enemy == 0xFF) {
        for (uint8_t i = 0; i < game.enemy_count; i++) {
            Enemy* enemy = &game.enemies[i];
            if (ENTITY_IS_ACTIVE(&enemy->base) && !enemy->captured) {
                if (entities_collide(&bubble->base, &enemy->base)) {
                    bubble_capture_enemy(bubble, i);
                    break;
                }
            }
        }
    }
    
    // Wall collision - bounce
    if (bubble->base.x <= TILE_WIDTH) {
        bubble->base.x = TILE_WIDTH + 1;
        bubble->base.vx = -bubble->base.vx / 2;
    }
    if (bubble->base.x >= (GRID_SIZE_X - 1) * TILE_WIDTH - bubble->base.width) {
        bubble->base.x = (GRID_SIZE_X - 1) * TILE_WIDTH - bubble->base.width - 1;
        bubble->base.vx = -bubble->base.vx / 2;
    }
    
    // Ceiling collision
    if (bubble->base.y <= TILE_HEIGHT) {
        bubble->base.y = TILE_HEIGHT + 1;
        bubble->base.vy = -bubble->base.vy / 2;
    }
}

void bubble_update_floating(Bubble* bubble) {
    // Float upward slowly
    bubble->base.y -= 1;
    
    // Slight random horizontal drift
    bubble->base.x += ((bubble->base.x & 0x01) ? 1 : -1);
    
    // Ceiling check
    if (bubble->base.y <= TILE_HEIGHT) {
        bubble->base.y = TILE_HEIGHT + 1;
    }
    
    // Check ceiling collision
    int8_t grid_x, grid_y;
    world_to_grid(bubble->base.x, bubble->base.y, &grid_x, &grid_y);
    if (is_tile_solid(game.level_index, grid_x, grid_y) || is_tile_solid(game.level_index, grid_x + 1, grid_y)) {
        bubble->base.y = (grid_y + 1) * TILE_HEIGHT + 1;
    }
    
    // Add attraction to other bubbles
    bubble_apply_attraction(bubble);
}

void bubble_update_popping(Bubble* bubble) {
    // Pop animation complete
    bubble->base.flags &= ~FLAG_ACTIVE;
    game.bubble_count--;
    
    // Handle captured enemy
    if (bubble->captured_enemy != 0xFF) {
        Enemy* enemy = &game.enemies[bubble->captured_enemy];
        
        if (bubble->pop_out) {
            // Release enemy
            enemy_release(enemy);
            enemy->base.x = bubble->base.x;
            enemy->base.y = bubble->base.y;
        } else {
            // Kill enemy
            enemy_die(enemy);
        }
    }
}

// =============================================================================
// BUBBLE PHYSICS
// =============================================================================

void bubble_physics_update(Bubble* bubble) {
    if (!ENTITY_IS_ACTIVE(&bubble->base)) return;
    if (bubble->state == BUBBLE_FLOATING) return;
    
    // Tile collision (only in normal state)
    if (bubble->state == BUBBLE_NORMAL) {
        int8_t grid_x, grid_y;
        
        // Bottom collision
        world_to_grid(bubble->base.x, bubble->base.y + bubble->base.height, &grid_x, &grid_y);
        if (is_tile_solid(game.level_index, grid_x, grid_y)) {
            bubble->base.y = grid_y * TILE_HEIGHT - bubble->base.height;
            bubble->base.vy = -bubble->base.vy / 2;
            if (bubble->base.vy > -2) bubble->base.vy = -2;
        }
    }
}

// =============================================================================
// BUBBLE CAPTURE ENEMY
// =============================================================================

void bubble_capture_enemy(Bubble* bubble, uint8_t enemy_index) {
    Enemy* enemy = &game.enemies[enemy_index];
    
    bubble->captured_enemy = enemy_index;
    bubble->state = BUBBLE_FLOATING;
    bubble->float_timer = 0;
    bubble->base.vx = 0;
    bubble->base.vy = 0;
    
    enemy_capture(enemy);
}

// =============================================================================
// BUBBLE POP
// =============================================================================

void bubble_start_pop(Bubble* bubble) {
    bubble->state = BUBBLE_POPPING;
    bubble->base.frame = 0;
    bubble->base.frame_timer = 0;
    
    // Random pop out
    bubble->pop_out = (bubble->base.x & 0x03) == 0;
}

void bubble_force_pop(Bubble* bubble) {
    bubble_start_pop(bubble);
}

// =============================================================================
// BUBBLE ATTRACTION
// =============================================================================

void bubble_apply_attraction(Bubble* bubble) {
    int16_t center_x = 0;
    int16_t center_y = 0;
    uint8_t count = 0;
    
    // Calculate center of all bubbles
    for (uint8_t i = 0; i < 8; i++) {
        if (ENTITY_IS_ACTIVE(&game.bubbles[i].base)) {
            center_x += game.bubbles[i].base.x;
            center_y += game.bubbles[i].base.y;
            count++;
        }
    }
    
    if (count > 1) {
        center_x /= count;
        center_y /= count;
        
        // Attract toward center
        int16_t dx = center_x - bubble->base.x;
        int16_t dy = center_y - bubble->base.y;
        
        bubble->base.x += (dx > 0) ? 1 : -1;
        bubble->base.y += (dy > 0) ? 1 : -1;
    }
}

// =============================================================================
// BUBBLE-PLAYER COLLISION
// =============================================================================

uint8_t bubble_check_player_collision(Bubble* bubble, Player* player) {
    if (!ENTITY_IS_ACTIVE(&bubble->base)) return 0;
    if (player->state == PLAYER_DEAD || player->state == PLAYER_BUBBLE) return 0;
    if (bubble->captured_enemy != 0xFF) return 0;  // Don't damage player from own bubble
    
    // Check collision
    return entities_collide(&bubble->base, &player->base);
}

// =============================================================================
// BUBBLE RENDERING
// =============================================================================

void bubble_render(Bubble* bubble) {
    if (!ENTITY_IS_ACTIVE(&bubble->base)) return;
    
    uint8_t tile = bubble->base.sprite_base;
    
    if (bubble->captured_enemy != 0xFF) {
        // Render enemy inside bubble
        Enemy* enemy = &game.enemies[bubble->captured_enemy];
        if (ENTITY_IS_ACTIVE(&enemy->base)) {
            // Render bubble first, then enemy on top
            // draw_sprite(bubble->base.x, bubble->base.y, TILE_BUBBLE_1, 0);
            // draw_sprite(bubble->base.x + 2, bubble->base.y + 2, enemy->base.sprite_base, enemy->base.direction);
        }
    } else {
        // Render just the bubble
        // draw_sprite(bubble->base.x, bubble->base.y, tile, 0);
    }
}

// =============================================================================
// UPDATE ALL BUBBLES
// =============================================================================

void update_bubbles(void) {
    for (uint8_t i = 0; i < 8; i++) {
        bubble_update(&game.bubbles[i]);
        bubble_physics_update(&game.bubbles[i]);
    }
}