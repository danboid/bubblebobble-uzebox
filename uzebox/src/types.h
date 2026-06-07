/*
 * BubbleBobble Uzebox - Game Types and Structures
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include "config.h"

// =============================================================================
// VECTOR TYPES
// =============================================================================

typedef struct {
    int8_t x;
    int8_t y;
} Vec2i;

typedef struct {
    int16_t x;
    int16_t y;
} Vec2s;

typedef struct {
    uint8_t x;
    uint8_t y;
} Vec2u;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} Rect;

// =============================================================================
// ENTITY BASE
// =============================================================================

typedef struct Entity {
    int16_t x;              // X position (fixed point, 8.8)
    int16_t y;              // Y position (fixed point, 8.8)
    int8_t  vx;             // X velocity
    int8_t  vy;             // Y velocity
    uint8_t width;          // Hitbox width
    uint8_t height;         // Hitbox height
    uint8_t sprite_base;    // Base tile index for sprite
    uint8_t frame;          // Current animation frame
    uint8_t frame_timer;    // Timer for animation
    uint8_t direction;      // 0=left, 1=right
    uint8_t flags;          // Entity flags
} __attribute__((packed)) Entity;

// Entity flags
#define FLAG_ACTIVE       0x01
#define FLAG_SOLID        0x02
#define FLAG_SENSOR       0x04
#define FLAG_FLIP_X       0x08
#define FLAG_GROUNDED     0x10

// =============================================================================
// PLAYER
// =============================================================================

typedef struct Player {
    Entity base;
    
    uint8_t  state;         // Current player state
    uint8_t  lives;         // Remaining lives
    uint16_t score;         // Player score
    uint8_t  player_index;  // 0 = P1 (Bubble), 1 = P2 (Bobble)
    uint8_t  attack_cooldown;
    uint8_t  respawn_timer;
    uint8_t  invulnerable;
    uint8_t  invulnerable_timer;
    uint8_t  jump_held;
    uint8_t  jump_time;
    uint8_t  fast_fall;
} __attribute__((packed)) Player;

// =============================================================================
// ENEMY
// =============================================================================

typedef struct Enemy {
    Entity base;
    
    uint8_t  type;          // ENEMY_ZENCHAN or ENEMY_MAITA
    uint8_t  state;         // Current enemy state
    uint8_t  ai_timer;      // Timer for AI behavior
    uint8_t  walk_direction;// Current walking direction
    uint8_t  jump_timer;    // Time until next jump
    uint8_t  charging;      // Is charging attack (ZenChan)
    uint8_t  captured;      // Is captured in bubble
    uint8_t  pop_out;       // Will pop out of bubble
} __attribute__((packed)) Enemy;

// =============================================================================
// BUBBLE (Projectile)
// =============================================================================

typedef struct Bubble {
    Entity base;
    
    uint8_t  lifetime;      // Frames until pop
    uint8_t  state;         // 0=normal, 1=floating, 2=popping
    uint8_t  captured_enemy;// Index of captured enemy (0xFF = none)
    uint8_t  float_timer;
    uint8_t  pop_out;   // Time spent floating
} __attribute__((packed)) Bubble;

// Bubble states
#define BUBBLE_NORMAL     0
#define BUBBLE_FLOATING   1
#define BUBBLE_POPPING     2

// =============================================================================
// PICKUP
// =============================================================================

typedef struct Pickup {
    Entity base;
    
    uint8_t  type;          // Pickup type (fries, watermelon)
    uint8_t  active;
} __attribute__((packed)) Pickup;

// Pickup types
#define PICKUP_WATERMELON  0
#define PICKUP_FRIES       1

// =============================================================================
// DEAD ENEMY (Flying fruit)
// =============================================================================

typedef struct DeadEnemy {
    Entity base;
    
    uint8_t  pickup_type;   // Type of pickup to spawn
    uint8_t  lifetime;      // Frames until despawn
} __attribute__((packed)) DeadEnemy;

// =============================================================================
// LEVEL TILE
// =============================================================================

typedef struct Tile {
    uint8_t type;           // Block type
} Tile;

// =============================================================================
// LEVEL DATA
// =============================================================================

typedef struct Level {
    Tile tiles[GRID_SIZE_Y][GRID_SIZE_X];  // 32x28 tile map
    uint8_t enemy_count;
    uint8_t enemies[8];     // Enemy spawn data (type, x, y)
} Level;

// =============================================================================
// GAME CONTEXT
// =============================================================================

typedef struct GameContext {
    uint8_t  state;         // Current game state
    uint8_t  level_index;   // Current level (0-3)
    uint8_t  player_count;  // 1 or 2 players
    uint16_t frame_counter; // Global frame counter
    
    // Players
    Player players[2];
    
    // Entities
    Enemy   enemies[8];
    uint8_t enemy_count;
    
    Bubble  bubbles[8];
    uint8_t bubble_count;
    
    Pickup  pickups[4];
    uint8_t pickup_count;
    
    DeadEnemy dead_enemies[4];
    uint8_t dead_enemy_count;
    
    // Level transition
    uint8_t transition_timer;
    int16_t camera_y;
    
    // Input
    uint8_t prev_buttons[2];
    uint8_t buttons[2];
    
} __attribute__((packed)) GameContext;

// =============================================================================
// ENTITY MANAGEMENT MACROS
// =============================================================================

#define FOR_EACH_PLAYER(ctx, i)      for(uint8_t i = 0; i < (ctx)->player_count; i++)
#define FOR_EACH_ENEMY(ctx, i)       for(uint8_t i = 0; i < (ctx)->enemy_count; i++)
#define FOR_EACH_BUBBLE(ctx, i)      for(uint8_t i = 0; i < (ctx)->bubble_count; i++)
#define FOR_EACH_PICKUP(ctx, i)      for(uint8_t i = 0; i < (ctx)->pickup_count; i++)

#define ENTITY_SET_ACTIVE(e, v)      ((e)->flags = ((e)->flags & ~FLAG_ACTIVE) | ((v) ? FLAG_ACTIVE : 0))
#define ENTITY_IS_ACTIVE(e)          ((e)->flags & FLAG_ACTIVE)
#define ENTITY_SET_GROUNDED(e, v)    ((e)->flags = ((e)->flags & ~FLAG_GROUNDED) | ((v) ? FLAG_GROUNDED : 0))
#define ENTITY_IS_GROUNDED(e)        ((e)->flags & FLAG_GROUNDED)

// =============================================================================
// COLLISION HELPERS
// =============================================================================

// Check if two entities collide
#define entities_collide(a, b) ( \
    (a)->x < (b)->x + (b)->width && \
    (a)->x + (a)->width > (b)->x && \
    (a)->y < (b)->y + (b)->height && \
    (a)->y + (a)->height > (b)->y \
)

// Get entity center X
#define entity_center_x(e) ((e)->x + ((e)->width >> 1))
// Get entity center Y
#define entity_center_y(e) ((e)->y + ((e)->height >> 1))

// Check if point is in entity
#define point_in_entity(px, py, e) ( \
    (px) >= (e)->x && (px) < (e)->x + (e)->width && \
    (py) >= (e)->y && (py) < (e)->y + (e)->height \
)

#endif // TYPES_H