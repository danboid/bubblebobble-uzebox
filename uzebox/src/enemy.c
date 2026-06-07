/*
 * BubbleBobble Uzebox - Enemy Entities
 * 
 * ZenChan and Maita enemy implementations with AI behavior
 */

#include "types.h"
#include "config.h"
#include "level_data.h"
#include "game.h"

// =============================================================================
// ZENCHAN INITIALIZATION
// =============================================================================

void zenchand_init(Enemy* enemy, int16_t x, int16_t y) {
    enemy->base.flags = FLAG_ACTIVE | FLAG_SOLID;
    enemy->base.x = x;
    enemy->base.y = y;
    enemy->base.width = 14;
    enemy->base.height = 14;
    enemy->base.vx = 0;
    enemy->base.vy = 0;
    enemy->base.sprite_base = TILE_ZENCHAN_1;
    enemy->base.frame = 0;
    enemy->base.frame_timer = 0;
    enemy->base.direction = 1;  // Start walking right
    
    enemy->type = ENEMY_ZENCHAN;
    enemy->state = ENEMY_WALKING;
    enemy->ai_timer = 0;
    enemy->walk_direction = 1;
    enemy->jump_timer = 60 + (x & 0x3F);  // Random initial delay
    enemy->charging = 0;
    enemy->captured = 0;
    enemy->pop_out = 0;
}

// =============================================================================
// MAITA INITIALIZATION
// =============================================================================

void maita_init(Enemy* enemy, int16_t x, int16_t y) {
    enemy->base.flags = FLAG_ACTIVE | FLAG_SOLID;
    enemy->base.x = x;
    enemy->base.y = y;
    enemy->base.width = 14;
    enemy->base.height = 14;
    enemy->base.vx = 0;
    enemy->base.vy = 0;
    enemy->base.sprite_base = TILE_MAITA_1;
    enemy->base.frame = 0;
    enemy->base.frame_timer = 0;
    enemy->base.direction = 1;
    
    enemy->type = ENEMY_MAITA;
    enemy->state = ENEMY_WALKING;
    enemy->ai_timer = 0;
    enemy->walk_direction = 1;
    enemy->jump_timer = 90 + (x & 0x3F);
    enemy->charging = 0;
    enemy->captured = 0;
    enemy->pop_out = 0;
}

// =============================================================================
// ENEMY AI UPDATE
// =============================================================================

void enemy_update_ai(Enemy* enemy, const Player* player) {
    if (enemy->captured) return;
    if (!ENTITY_IS_ACTIVE(&enemy->base)) return;
    
    // AI timer
    enemy->ai_timer++;
    
    switch (enemy->state) {
        case ENEMY_WALKING:
            enemy_update_walking(enemy);
            break;
        case ENEMY_FALLING:
            enemy_update_falling(enemy);
            break;
        case ENEMY_JUMPING:
            enemy_update_jumping(enemy);
            break;
        case ENEMY_CAPTURED:
            // Handled by bubble
            break;
    }
    
    // Animation
    enemy_update_animation(enemy);
}

void enemy_update_walking(Enemy* enemy) {
    uint8_t speed = enemy->charging ? (ENEMY_SPEED * 2) : ENEMY_SPEED;
    
    // Apply movement
    enemy->base.vx = enemy->walk_direction * speed;
    
    // Random jump
    enemy->jump_timer--;
    if (enemy->jump_timer == 0) {
        enemy->state = ENEMY_JUMPING;
        enemy->base.vy = -6;
        enemy->jump_timer = 60 + (enemy->base.x & 0x3F);  // Reset timer
        return;
    }
    
    // Check if still on ground
    int8_t grid_x, grid_y;
    world_to_grid(enemy->base.x, enemy->base.y + 1, &grid_x, &grid_y);
    
    if (!is_tile_solid(&game.level, grid_x, grid_y)) {
        enemy->state = ENEMY_FALLING;
        return;
    }
    
    // Check for wall - turn around
    int8_t check_x = (enemy->walk_direction > 0) ? grid_x + 1 : grid_x - 1;
    if (is_tile_solid(&game.level, check_x, grid_y) || is_tile_solid(&game.level, check_x, grid_y - 1)) {
        enemy->walk_direction = -enemy->walk_direction;
    }
}

void enemy_update_falling(Enemy* enemy) {
    // Apply gravity
    enemy->base.vy += 1;
    if (enemy->base.vy > ENEMY_SPEED * 4) {
        enemy->base.vy = ENEMY_SPEED * 4;
    }
    
    // Check if landed
    int8_t grid_x, grid_y;
    world_to_grid(enemy->base.x, enemy->base.y + 1, &grid_x, &grid_y);
    
    if (is_tile_solid(&game.level, grid_x, grid_y) && enemy->base.vy > 0) {
        enemy->state = ENEMY_WALKING;
        enemy->base.y = grid_y * TILE_HEIGHT - enemy->base.height;
        enemy->base.vy = 0;
    }
}

void enemy_update_jumping(Enemy* enemy) {
    // Reduced horizontal control while jumping
    enemy->base.vx = enemy->walk_direction * (ENEMY_SPEED / 2);
    
    // Apply gravity
    enemy->base.vy += 1;
    if (enemy->base.vy > ENEMY_SPEED * 4) {
        enemy->base.vy = ENEMY_SPEED * 4;
    }
    
    // Check if landed
    int8_t grid_x, grid_y;
    world_to_grid(enemy->base.x, enemy->base.y + 1, &grid_x, &grid_y);
    
    if (is_tile_solid(&game.level, grid_x, grid_y) && enemy->base.vy >= 0) {
        enemy->state = ENEMY_WALKING;
        enemy->base.y = grid_y * TILE_HEIGHT - enemy->base.height;
        enemy->base.vy = 0;
    }
}

void enemy_update_animation(Enemy* enemy) {
    if (enemy->type == ENEMY_ZENCHAN) {
        // ZenChan animation
        if (enemy->charging) {
            enemy->base.sprite_base = TILE_ZENCHAN_2;  // Charging frame
        } else {
            enemy->base.frame_timer++;
            if (enemy->base.frame_timer >= 12) {
                enemy->base.frame_timer = 0;
                enemy->base.frame ^= 1;  // Toggle between 0 and 1
            }
            enemy->base.sprite_base = TILE_ZENCHAN_1 + enemy->base.frame;
        }
    } else {
        // Maita animation
        enemy->base.frame_timer++;
        if (enemy->base.frame_timer >= 10) {
            enemy->base.frame_timer = 0;
            enemy->base.frame ^= 1;
        }
        enemy->base.sprite_base = TILE_MAITA_1 + enemy->base.frame;
    }
}

// =============================================================================
// ENEMY PHYSICS UPDATE
// =============================================================================

void enemy_physics_update(Enemy* enemy, Level* level) {
    if (enemy->captured) return;
    if (!ENTITY_IS_ACTIVE(&enemy->base)) return;
    
    // Apply velocity
    int16_t new_x = enemy->base.x + enemy->base.vx;
    int16_t new_y = enemy->base.y + enemy->base.vy;
    
    // Horizontal collision
    int8_t grid_x, grid_y;
    world_to_grid(new_x, enemy->base.y, &grid_x, &grid_y);
    
    if (enemy->base.vx > 0) {
        if (is_tile_solid(level, grid_x + 1, grid_y) || is_tile_solid(level, grid_x + 1, grid_y + 1)) {
            new_x = (grid_x + 1) * TILE_WIDTH - enemy->base.width;
            enemy->walk_direction = -enemy->walk_direction;
        }
    } else if (enemy->base.vx < 0) {
        if (is_tile_solid(level, grid_x, grid_y) || is_tile_solid(level, grid_x, grid_y + 1)) {
            new_x = (grid_x + 1) * TILE_WIDTH;
            enemy->walk_direction = -enemy->walk_direction;
        }
    }
    
    // Vertical collision
    world_to_grid(new_x, new_y, &grid_x, &grid_y);
    
    if (enemy->base.vy > 0) {
        if (is_tile_solid(level, grid_x, grid_y + 1)) {
            new_y = (grid_y + 1) * TILE_HEIGHT - enemy->base.height;
            enemy->base.vy = 0;
            if (enemy->state == ENEMY_FALLING || enemy->state == ENEMY_JUMPING) {
                enemy->state = ENEMY_WALKING;
            }
        }
    } else if (enemy->base.vy < 0) {
        if (is_tile_solid(level, grid_x, grid_y)) {
            new_y = (grid_y + 1) * TILE_HEIGHT;
            enemy->base.vy = 0;
        }
    }
    
    // Boundary checks
    if (new_x < TILE_WIDTH) {
        new_x = TILE_WIDTH;
        enemy->walk_direction = 1;
    }
    if (new_x > (GRID_SIZE_X - 1) * TILE_WIDTH - enemy->base.width) {
        new_x = (GRID_SIZE_X - 1) * TILE_WIDTH - enemy->base.width;
        enemy->walk_direction = -1;
    }
    
    enemy->base.x = new_x;
    enemy->base.y = new_y;
}

// =============================================================================
// ENEMY CAPTURE (in bubble)
// =============================================================================

void enemy_capture(Enemy* enemy) {
    if (enemy->captured) return;
    
    enemy->captured = 1;
    enemy->state = ENEMY_CAPTURED;
    enemy->base.vx = 0;
    enemy->base.vy = 0;
    enemy->base.flags &= ~FLAG_SOLID;  // No longer solid
}

void enemy_release(Enemy* enemy) {
    if (!enemy->captured) return;
    
    enemy->captured = 0;
    enemy->state = ENEMY_WALKING;
    enemy->base.flags |= FLAG_SOLID;
    
    // ZenChan becomes aggressive when released
    if (enemy->type == ENEMY_ZENCHAN) {
        enemy->charging = 1;
    }
    
    // Random chance to pop out
    enemy->pop_out = (enemy->base.x & 0x01);
}

void enemy_die(Enemy* enemy) {
    enemy->base.flags &= ~FLAG_ACTIVE;
    enemy->captured = 0;
    
    // Spawn dead enemy (flying fruit)
    spawn_dead_enemy(enemy->base.x, enemy->base.y, 
                     (enemy->type == ENEMY_ZENCHAN) ? PICKUP_WATERMELON : PICKUP_FRIES);
}

// =============================================================================
// ENEMY-PLAYER COLLISION
// =============================================================================

uint8_t enemy_check_player_collision(const Enemy* enemy, const Player* player) {
    if (!ENTITY_IS_ACTIVE(&enemy->base)) return 0;
    if (player->state == PLAYER_DEAD || player->state == PLAYER_BUBBLE) return 0;
    
    return entities_collide(&enemy->base, &player->base);
}

// =============================================================================
// ENEMY RENDERING
// =============================================================================

void enemy_render(const Enemy* enemy) {
    if (!ENTITY_IS_ACTIVE(&enemy->base)) return;
    if (enemy->captured) return;  // Rendered by bubble
    
    uint8_t tile = enemy->base.sprite_base;
    
    // Draw sprite
    // draw_sprite(enemy->base.x / TILE_WIDTH * TILE_WIDTH, 
    //             enemy->base.y / TILE_HEIGHT * TILE_HEIGHT,
    //             tile, enemy->base.direction);
}

// =============================================================================
// SPAWN ENEMIES FROM LEVEL DATA
// =============================================================================

void spawn_enemies_from_level(Level* level) {
    game.enemy_count = level->enemy_count;
    
    for (uint8_t i = 0; i < game.enemy_count && i < 8; i++) {
        uint8_t type = level->enemies[i * 3 + 0];
        uint8_t tile_x = level->enemies[i * 3 + 1];
        uint8_t tile_y = level->enemies[i * 3 + 2];
        
        int16_t world_x = tile_x * TILE_WIDTH;
        int16_t world_y = tile_y * TILE_HEIGHT;
        
        if (type == ENEMY_ZENCHAN) {
            zenchand_init(&game.enemies[i], world_x, world_y);
        } else {
            maita_init(&game.enemies[i], world_x, world_y);
        }
    }
}