/*
 * BubbleBobble Uzebox Port - Configuration
 * 
 * Uzebox Mode 3 Extended Video Mode:
 * - 32x28 tiles (256x224 pixels)
 * - 8x8 pixel tiles
 * - 256 colors
 * 
 * Target: ATMega644p @ 28.636 MHz
 */

#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// SCREEN & VIDEO SETTINGS (Mode 3 Extended - No Scrolling)
// =============================================================================

// VRAM dimensions for Mode 3 extended (no scrolling)
#define VRAM_TILES_H     32    // Screen width in tiles (256 pixels)
#define VRAM_TILES_V     28    // Screen height in tiles (224 pixels)

// Tile size
#define TILE_WIDTH       8
#define TILE_HEIGHT      8

// Screen resolution
#define SCREEN_WIDTH     (VRAM_TILES_H * TILE_WIDTH)  // 256 pixels
#define SCREEN_HEIGHT    (VRAM_TILES_V * TILE_HEIGHT) // 224 pixels

// =============================================================================
// GAME CONSTANTS
// =============================================================================

// Grid/Level dimensions (matches original)
#define GRID_SIZE_X       32   // Level width in tiles
#define GRID_SIZE_Y       28   // Level height in tiles

// Player settings
#define PLAYER_SPEED      2    // Pixels per frame
#define PLAYER_JUMP_FORCE  8
#define PLAYER_SIZE       16   // Player sprite size in pixels

// Enemy settings
#define ENEMY_SPEED       1
#define ENEMY_SIZE        16

// Bubble settings
#define BUBBLE_SPEED      4
#define BUBBLE_SIZE       12
#define BUBBLE_LIFETIME   180  // Frames (3 seconds at 60fps)

// Starting lives
#define STARTING_LIVES    3

// =============================================================================
// SPRITE TILE INDICES (in tile data array)
// =============================================================================

// Player sprites (16x16, 4 tiles each)
#define TILE_P1_IDLE_1    0
#define TILE_P1_WALK_1    4
#define TILE_P1_JUMP_1    8
#define TILE_P1_ATTACK_1  12

#define TILE_P2_IDLE_1    16
#define TILE_P2_WALK_1    20
#define TILE_P2_JUMP_1    24
#define TILE_P2_ATTACK_1  28

// Bubble sprites
#define TILE_BUBBLE_1     32
#define TILE_BUBBLE_2     36

// Enemy sprites (ZenChan)
#define TILE_ZENCHAN_1    40
#define TILE_ZENCHAN_2    44

// Enemy sprites (Maita)  
#define TILE_MAITA_1      48
#define TILE_MAITA_2      52

// Pickup sprites
#define TILE_WATERMELON   56
#define TILE_FRIES        58

// Level tile sprites
#define TILE_SOLID        60
#define TILE_SEMI_SOLID    62

// UI sprites
#define TILE_FONT_START   64
#define TILE_HEART        96

// =============================================================================
// INPUT BUTTONS (SNES Controller)
// =============================================================================

#define BTN_RIGHT    0x01
#define BTN_LEFT     0x02
#define BTN_DOWN     0x04
#define BTN_UP       0x08
#define BTN_START    0x10
#define BTN_A        0x20
#define BTN_B        0x40
#define BTN_SELECT   0x80

// =============================================================================
// GAME STATES
// =============================================================================

typedef enum {
    STATE_TITLE,
    STATE_GAME,
    STATE_LEVEL_TRANSITION,
    STATE_GAME_OVER,
    STATE_SCORE_SCREEN
} GameState;

// Player states
typedef enum {
    PLAYER_WALKING,
    PLAYER_JUMPING,
    PLAYER_ATTACKING,
    PLAYER_DEAD,
    PLAYER_BUBBLE
} PlayerState;

// Enemy types
typedef enum {
    ENEMY_ZENCHAN,
    ENEMY_MAITA
} EnemyType;

// Enemy states
typedef enum {
    ENEMY_WALKING,
    ENEMY_FALLING,
    ENEMY_JUMPING,
    ENEMY_CAPTURED
} EnemyState;

// Block types for level tiles
typedef enum {
    BLOCK_EMPTY   = 0,
    BLOCK_SOLID   = 1,
    BLOCK_SEMI    = 2  // One-way platform
} BlockType;

// =============================================================================
// COLOR PALETTE (256 colors)
// =============================================================================

// Player colors
#define COLOR_P1          0x12  // Green
#define COLOR_P2          0x23  // Blue

// UI colors
#define COLOR_WHITE       0xFF
#define COLOR_BLACK       0x00
#define COLOR_RED         0x01
#define COLOR_YELLOW      0x10

// Game colors
#define COLOR_BUBBLE      0x9A  // Light blue
#define COLOR_BUBBLE_DARK 0x67

// Level colors
#define COLOR_TILE_FILL   0x34
#define COLOR_TILE_EDGE   0x12

#endif // CONFIG_H