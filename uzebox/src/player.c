/*
 * BubbleBobble Uzebox - Player Entity
 * 
 * Player state machine implementation
 */

#include "types.h"
#include "config.h"
#include "level_data.h"
#include "game.h"

// =============================================================================
// PLAYER INITIALIZATION
// =============================================================================

void player_init(Player* player, uint8_t player_index) {
    player->base.flags = FLAG_ACTIVE;
    player->base.width = 14;
    player->base.height = 16;
    player->base.vx = 0;
    player->base.vy = 0;
    player->base.direction = (player_index == 0) ? 1 : 0;  // P1 faces right, P2 faces left
    
    player->state = PLAYER_WALKING;
    player->lives = STARTING_LIVES;
    player->score = 0;
    player->player_index = player_index;
    player->attack_cooldown = 0;
    player->respawn_timer = 0;
    player->invulnerable = 0;
    player->invulnerable_timer = 0;
    player->jump_held = 0;
    player->jump_time = 0;
    player->fast_fall = 0;
    
    // Set starting position
    if (player_index == 0) {
        player->base.x = 8 * TILE_WIDTH;   // Center-left
        player->base.y = 22 * TILE_HEIGHT;  // Bottom area
    } else {
        player->base.x = 23 * TILE_WIDTH;   // Center-right
        player->base.y = 22 * TILE_HEIGHT;  // Bottom area
    }
    
    // Set sprite base tile
    player->base.sprite_base = (player_index == 0) ? TILE_P1_IDLE_1 : TILE_P2_IDLE_1;
    player->base.frame = 0;
    player->base.frame_timer = 0;
}

// =============================================================================
// PLAYER INPUT HANDLING
// =============================================================================

void player_handle_input(Player* player, uint8_t buttons, uint8_t prev_buttons) {
    if (player->state == PLAYER_DEAD) return;
    
    int8_t move_input = 0;
    uint8_t jump_pressed = 0;
    uint8_t attack_pressed = 0;
    
    // Movement
    if (buttons & BTN_LEFT) {
        move_input = -1;
        player->base.direction = 0;
    }
    if (buttons & BTN_RIGHT) {
        move_input = 1;
        player->base.direction = 1;
    }
    
    // Jump (A button or Up)
    if ((buttons & BTN_A) && !(prev_buttons & BTN_A)) {
        jump_pressed = 1;
    }
    if ((buttons & BTN_UP) && !(prev_buttons & BTN_UP)) {
        jump_pressed = 1;
    }
    
    // Attack (B button)
    if ((buttons & BTN_B) && !(prev_buttons & BTN_B)) {
        attack_pressed = 1;
    }
    
    // Process based on current state
    switch (player->state) {
        case PLAYER_WALKING:
            player_update_walking(player, move_input, jump_pressed, attack_pressed);
            break;
        case PLAYER_JUMPING:
            player_update_jumping(player, move_input, jump_pressed, attack_pressed);
            break;
        case PLAYER_ATTACKING:
            player_update_attacking(player, move_input, attack_pressed);
            break;
        case PLAYER_BUBBLE:
            // Player in bubble - no input
            break;
    }
}

// =============================================================================
// PLAYER STATE: WALKING
// =============================================================================

void player_update_walking(Player* player, int8_t move_input, uint8_t jump_pressed, uint8_t attack_pressed) {
    // Apply movement
    player->base.vx = move_input * PLAYER_SPEED;
    
    // Check for jump
    if (jump_pressed && ENTITY_IS_GROUNDED(&player->base)) {
        player->state = PLAYER_JUMPING;
        player->base.vy = -PLAYER_JUMP_FORCE;
        player->jump_time = 0;
        player->jump_held = 1;
        player->base.flags &= ~FLAG_GROUNDED;
        return;
    }
    
    // Check for attack
    if (attack_pressed && player->attack_cooldown == 0) {
        player->state = PLAYER_ATTACKING;
        player->attack_cooldown = 24;  // 0.4 seconds at 60fps
        player->base.frame = 0;
        player->base.frame_timer = 0;
        return;
    }
    
    // Animation
    if (move_input != 0) {
        player->base.frame_timer++;
        if (player->base.frame_timer >= 8) {
            player->base.frame_timer = 0;
            player->base.frame = (player->base.frame + 1) & 0x03;
        }
    } else {
        player->base.frame = 0;
    }
    
    // Update sprite
    if (move_input != 0) {
        player->base.sprite_base = (player->player_index == 0) ? TILE_P1_WALK_1 : TILE_P2_WALK_1;
    } else {
        player->base.sprite_base = (player->player_index == 0) ? TILE_P1_IDLE_1 : TILE_P2_IDLE_1;
    }
    
    // Decrease attack cooldown
    if (player->attack_cooldown > 0) {
        player->attack_cooldown--;
    }
}

// =============================================================================
// PLAYER STATE: JUMPING
// =============================================================================

void player_update_jumping(Player* player, int8_t move_input, uint8_t jump_pressed, uint8_t attack_pressed) {
    // Variable jump height
    if (jump_pressed && player->jump_held && player->jump_time < 12) {
        player->base.vy -= 1;
        player->jump_time++;
    }
    
    // Fast fall
    if (player->base.vy > 0 && (move_input != 0)) {
        player->base.vy += 1;
        if (player->base.vy > PLAYER_JUMP_FORCE * 2) {
            player->base.vy = PLAYER_JUMP_FORCE * 2;
        }
    }
    
    // Apply gravity
    player->base.vy += 1;
    if (player->base.vy > PLAYER_JUMP_FORCE * 2) {
        player->base.vy = PLAYER_JUMP_FORCE * 2;
    }
    
    // Horizontal movement (reduced while jumping)
    player->base.vx = move_input * (PLAYER_SPEED / 2);
    
    // Check for attack
    if (attack_pressed && player->attack_cooldown == 0) {
        player->state = PLAYER_ATTACKING;
        player->attack_cooldown = 24;
        player->base.frame = 0;
        player->base.frame_timer = 0;
        return;
    }
    
    // Jump animation
    if (player->base.vy < 0) {
        player->base.sprite_base = (player->player_index == 0) ? TILE_P1_JUMP_1 : TILE_P2_JUMP_1;
    } else {
        player->base.sprite_base = (player->player_index == 0) ? TILE_P1_JUMP_1 : TILE_P2_JUMP_1;
        player->base.frame = 1;  // Falling frame
    }
    
    // Decrease attack cooldown
    if (player->attack_cooldown > 0) {
        player->attack_cooldown--;
    }
}

// =============================================================================
// PLAYER STATE: ATTACKING
// =============================================================================

void player_update_attacking(Player* player, int8_t move_input, uint8_t attack_pressed) {
    // Animation timing
    player->base.frame_timer++;
    if (player->base.frame_timer >= 6) {
        player->base.frame_timer = 0;
        player->base.frame++;
    }
    
    // Attack duration
    if (player->base.frame >= 4) {
        player->state = PLAYER_WALKING;
        player->base.frame = 0;
    }
    
    // Update sprite
    player->base.sprite_base = (player->player_index == 0) ? TILE_P1_ATTACK_1 : TILE_P2_ATTACK_1;
}

// =============================================================================
// PLAYER PHYSICS UPDATE
// =============================================================================

void player_physics_update(Player* player, GameContext* ctx) {
    if (player->state == PLAYER_DEAD || player->state == PLAYER_BUBBLE) return;
    
    // Apply velocity
    int16_t new_x = player->base.x + player->base.vx;
    int16_t new_y = player->base.y + player->base.vy;
    
    // Horizontal collision
    int8_t grid_x, grid_y;
    world_to_grid(new_x, player->base.y, &grid_x, &grid_y);
    
    if (player->base.vx > 0) {
        // Moving right - check right edge
        if (is_tile_solid(ctx, grid_x + 1, grid_y) || is_tile_solid(ctx, grid_x + 1, grid_y + 1)) {
            new_x = (grid_x + 1) * TILE_WIDTH - player->base.width;
            player->base.vx = 0;
        }
    } else if (player->base.vx < 0) {
        // Moving left - check left edge
        if (is_tile_solid(ctx, grid_x, grid_y) || is_tile_solid(ctx, grid_x, grid_y + 1)) {
            new_x = (grid_x + 1) * TILE_WIDTH;
            player->base.vx = 0;
        }
    }
    
    // Vertical collision
    world_to_grid(new_x, new_y, &grid_x, &grid_y);
    uint8_t was_grounded = ENTITY_IS_GROUNDED(&player->base);
    
    if (player->base.vy > 0) {
        // Falling - check below
        if (is_tile_solid(ctx, grid_x, grid_y + 1)) {
            new_y = (grid_y + 1) * TILE_HEIGHT - player->base.height;
            player->base.vy = 0;
            player->base.flags |= FLAG_GROUNDED;
            
            // Transition to walking if we were jumping
            if (player->state == PLAYER_JUMPING) {
                player->state = PLAYER_WALKING;
            }
        } else {
            player->base.flags &= ~FLAG_GROUNDED;
        }
    } else if (player->base.vy < 0) {
        // Rising - check above
        if (is_tile_solid(ctx, grid_x, grid_y) || is_tile_solid(ctx, grid_x + 1, grid_y)) {
            new_y = (grid_y + 1) * TILE_HEIGHT;
            player->base.vy = 0;
        }
    }
    
    // Boundary checks
    if (new_x < TILE_WIDTH) new_x = TILE_WIDTH;
    if (new_x > (GRID_SIZE_X - 1) * TILE_WIDTH - player->base.width) {
        new_x = (GRID_SIZE_X - 1) * TILE_WIDTH - player->base.width;
    }
    
    // Apply position
    player->base.x = new_x;
    player->base.y = new_y;
}

// =============================================================================
// PLAYER DAMAGE & DEATH
// =============================================================================

void player_take_damage(Player* player) {
    if (player->invulnerable) return;
    if (player->state == PLAYER_DEAD) return;
    
    player->lives--;
    player->state = PLAYER_DEAD;
    player->respawn_timer = 180;  // 3 seconds
}

void player_respawn(Player* player) {
    if (player->lives == 0) return;  // Game over
    
    player->state = PLAYER_WALKING;
    player->invulnerable = 1;
    player->invulnerable_timer = 120;  // 2 seconds invulnerability
    player->base.vx = 0;
    player->base.vy = 0;
    
    // Reset position
    if (player->player_index == 0) {
        player->base.x = 8 * TILE_WIDTH;
        player->base.y = 22 * TILE_HEIGHT;
    } else {
        player->base.x = 23 * TILE_WIDTH;
        player->base.y = 22 * TILE_HEIGHT;
    }
}

void player_update_death(Player* player) {
    if (player->state != PLAYER_DEAD) return;
    
    player->respawn_timer--;
    if (player->respawn_timer == 0) {
        player_respawn(player);
    }
}

void player_update_invulnerability(Player* player) {
    if (!player->invulnerable) return;
    
    player->invulnerable_timer--;
    if (player->invulnerable_timer == 0) {
        player->invulnerable = 0;
    }
}

// =============================================================================
// PLAYER RENDERING
// =============================================================================

void player_render(const Player* player) {
    if (!ENTITY_IS_ACTIVE(&player->base)) return;
    if (player->state == PLAYER_DEAD) return;
    
    // Flash when invulnerable
    if (player->invulnerable && (player->invulnerable_timer & 0x08)) return;
    
    uint8_t tile = player->base.sprite_base + player->base.frame;
    
    // Draw sprite at position
    // Note: Using Uzebox API functions
    uint8_t x = player->base.x / TILE_WIDTH;
    uint8_t y = player->base.y / TILE_HEIGHT;
    
    // This will be implemented with the rendering system
    // draw_sprite(x * TILE_WIDTH, y * TILE_HEIGHT, tile, player->base.direction);
}