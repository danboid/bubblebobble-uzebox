#include <uzebox.h>
/*
 * BubbleBobble Uzebox - Main Game Module
 * 
 * Game state management, main loop, and initialization
 */

#include "types.h"
#include "config.h"
#include "level_data.h"
#include "game.h"

// =============================================================================
// GLOBAL GAME CONTEXT
// =============================================================================

GameContext game;

// =============================================================================
// TEXT PRINTING HELPERS
// =============================================================================

// Digit strings in PROGMEM
static const char digits_pstr[] PROGMEM = "0123456789";

// Print a 16-bit unsigned integer at position (5 digits)
void PrintUInt16(uint8_t x, uint8_t y, uint16_t value) {
    uint16_t place = 10000;
    uint8_t started = 0;
    
    while (place > 0) {
        uint8_t digit = value / place;
        value %= place;
        
        if (started || digit > 0 || place == 1) {
            // Print single digit using char pointer trick
            char c = pgm_read_byte(&digits_pstr[digit]);
            char str[2] = {c, 0};
            Print(x++, y, str);
            started = 1;
        } else {
            // Print leading space
            Print(x++, y, PSTR(" "));
        }
        place /= 10;
    }
}

// Print an 8-bit unsigned integer at position (2 digits)
void PrintUInt8(uint8_t x, uint8_t y, uint8_t value) {
    uint8_t tens = value / 10;
    uint8_t ones = value % 10;
    
    char str[2];
    str[0] = pgm_read_byte(&digits_pstr[tens]);
    str[1] = 0;
    Print(x++, y, str);
    
    str[0] = pgm_read_byte(&digits_pstr[ones]);
    Print(x++, y, str);
}

// Print a single digit
void PrintDigit(uint8_t x, uint8_t y, uint8_t digit) {
    char str[2];
    str[0] = pgm_read_byte(&digits_pstr[digit]);
    str[1] = 0;
    Print(x, y, str);
}

// =============================================================================
// GAME INITIALIZATION
// =============================================================================

void game_init(uint8_t player_count) {
    game.state = STATE_TITLE;
    game.level_index = 0;
    game.player_count = player_count;
    game.frame_counter = 0;
    game.transition_timer = 0;
    game.camera_y = 0;
    
    // Clear all entities
    for (uint8_t i = 0; i < 2; i++) {
        game.players[i].base.flags = 0;
        game.players[i].lives = STARTING_LIVES;
        game.players[i].score = 0;
    }
    
    for (uint8_t i = 0; i < 8; i++) {
        game.enemies[i].base.flags = 0;
        game.bubbles[i].base.flags = 0;
    }
    
    for (uint8_t i = 0; i < 4; i++) {
        game.pickups[i].active = 0;
        game.dead_enemies[i].base.flags = 0;
    }
    
    game.enemy_count = 0;
    game.bubble_count = 0;
    game.pickup_count = 0;
    game.dead_enemy_count = 0;
}

// =============================================================================
// GAME STATE TRANSITIONS
// =============================================================================

void game_start(void) {
    game.state = STATE_GAME;
    game.level_index = 0;
    
    // Initialize players
    for (uint8_t i = 0; i < game.player_count; i++) {
        player_init(&game.players[i], i);
    }
    
    // Load first level
    load_level(&game, game.level_index);
    spawn_enemies_from_level(&game);
}

void game_next_level(void) {
    game.level_index++;
    
    if (game.level_index >= 4) {
        // All levels complete - show score screen
        game.state = STATE_SCORE_SCREEN;
        return;
    }
    
    // Start transition
    game.state = STATE_LEVEL_TRANSITION;
    game.transition_timer = 240;  // 4 seconds
    
    // Clear remaining bubbles and pickups
    for (uint8_t i = 0; i < 8; i++) {
        game.bubbles[i].base.flags = 0;
    }
    for (uint8_t i = 0; i < 4; i++) {
        game.pickups[i].active = 0;
    }
    game.bubble_count = 0;
    game.pickup_count = 0;
    
    // Move players to top for transition animation
    for (uint8_t i = 0; i < game.player_count; i++) {
        game.players[i].base.y = -50;  // Off screen
        game.players[i].state = PLAYER_BUBBLE;
    }
}

void game_handle_level_complete(void) {
    // Check if all enemies defeated
    uint8_t enemies_remaining = 0;
    for (uint8_t i = 0; i < game.enemy_count; i++) {
        if (ENTITY_IS_ACTIVE(&game.enemies[i].base) && !game.enemies[i].captured) {
            enemies_remaining++;
        }
    }
    
    if (enemies_remaining == 0) {
        game_next_level();
    }
}

void game_player_died(void) {
    // Check if game over
    uint8_t players_alive = 0;
    for (uint8_t i = 0; i < game.player_count; i++) {
        if (game.players[i].lives > 0) {
            players_alive++;
        }
    }
    
    if (players_alive == 0) {
        game.state = STATE_GAME_OVER;
    }
}

// =============================================================================
// INPUT HANDLING
// =============================================================================

void game_read_input(void) {
    // Store previous buttons
    game.prev_buttons[0] = game.buttons[0];
    game.prev_buttons[1] = game.buttons[1];
    
    // Read controller
    // Note: Uses Uzebox controller API
    // game.buttons[0] = read_controller(0);
    // game.buttons[1] = read_controller(1);
}

uint8_t is_button_pressed(uint8_t player, uint8_t button) {
    return (game.buttons[player] & button) && !(game.prev_buttons[player] & button);
}

uint8_t is_button_held(uint8_t player, uint8_t button) {
    return (game.buttons[player] & button);
}

// =============================================================================
// TITLE SCREEN
// =============================================================================

void update_title_screen(void) {
    // Start 1-player game on button press
    if (is_button_pressed(0, BTN_START) || is_button_pressed(0, BTN_A)) {
        game.player_count = 1;
        game_start();
    }
    // Start 2-player game on button press (P2)
    if (is_button_pressed(1, BTN_START) || is_button_pressed(1, BTN_A)) {
        game.player_count = 2;
        game_start();
    }
}

void render_title_screen(void) {
    // Show title
    Print(10, 10, PSTR("BUBBLEBOBBLE"));
    
    // 1 player option
    Print(12, 14, PSTR("P1 START"));
    Print(12, 16, PSTR("PRESS A"));
    
    // 2 player option  
    Print(12, 20, PSTR("P2 START"));
    Print(12, 22, PSTR("PRESS A"));
}

// =============================================================================
// GAME UPDATE
// =============================================================================

void update_game(void) {
    // Update players
    for (uint8_t i = 0; i < game.player_count; i++) {
        Player* player = &game.players[i];
        
        // Handle input
        player_handle_input(player, game.buttons[i], game.prev_buttons[i]);
        
        // Physics update
        player_physics_update(player, &game);
        
        // Update death/respawn
        player_update_death(player);
        player_update_invulnerability(player);
        
        // Fire bubbles
        if (player->state == PLAYER_ATTACKING && player->base.frame == 2) {
            player_fire_bubble(player);
        }
    }
    
    // Update enemies
    for (uint8_t i = 0; i < game.enemy_count; i++) {
        Enemy* enemy = &game.enemies[i];
        
        if (ENTITY_IS_ACTIVE(&enemy->base) && !enemy->captured) {
            enemy_update_ai(enemy, &game.players[0]);
            enemy_physics_update(enemy, &game);
        }
    }
    
    // Update bubbles
    update_bubbles();
    
    // Run collision checks
    run_collision_checks();
    
    // Check level completion
    game_handle_level_complete();
}

// =============================================================================
// LEVEL TRANSITION UPDATE
// =============================================================================

void update_level_transition(void) {
    game.transition_timer--;
    
    // Animate camera moving up
    if (game.transition_timer > 120) {
        // Moving up
        game.camera_y -= 2;
    } else if (game.transition_timer == 120) {
        // Load next level
        load_level(&game, game.level_index);
        spawn_enemies_from_level(&game);
        game.camera_y = GRID_SIZE_Y * TILE_HEIGHT;
    } else {
        // Moving to position
        game.camera_y += 2;
        if (game.camera_y > 0) game.camera_y = 0;
    }
    
    // Update player bubbles floating down
    for (uint8_t i = 0; i < game.player_count; i++) {
        Player* player = &game.players[i];
        if (game.transition_timer < 120) {
            player->base.y += 1;
            if (player->base.y > 0) {
                player->base.y = 0;
                player->state = PLAYER_WALKING;
            }
        }
    }
    
    if (game.transition_timer == 0) {
        game.state = STATE_GAME;
    }
}

// =============================================================================
// GAME OVER UPDATE
// =============================================================================

void update_game_over(void) {
    if (is_button_pressed(0, BTN_START)) {
        game_init(game.player_count);
        game.state = STATE_TITLE;
    }
}

void render_game_over(void) {
    // draw_text(10, 12, "GAME OVER");
    // draw_text(8, 15, "PRESS START");
}

// =============================================================================
// SCORE SCREEN UPDATE
// =============================================================================

void update_score_screen(void) {
    if (is_button_pressed(0, BTN_START)) {
        game_init(game.player_count);
        game.state = STATE_TITLE;
    }
}

void render_score_screen(void) {
    // Render final scores
    // draw_text(8, 8, "CONGRATULATIONS!");
    // draw_number(8, 12, "P1 SCORE:", game.players[0].score);
    // if (game.player_count > 1) {
    //     draw_number(8, 14, "P2 SCORE:", game.players[1].score);
    // }
}

// =============================================================================
// LEVEL RENDERING
// =============================================================================

void render_level(void) {
    // Render tiles using SetTile (tile coordinates, not pixels)
    for (uint8_t y = 0; y < 28; y++) {
        for (uint8_t x = 0; x < 32; x++) {
            BlockType tile = get_tile(game.level_index, x, y);

            if (tile == BLOCK_SOLID) {
                SetTile(x, y, 1);  // Solid tile
            } else if (tile == BLOCK_SEMI) {
                SetTile(x, y, 2);  // Semi-solid tile
            } else {
                SetTile(x, y, 0);  // Empty/air tile
            }
        }
    }
}

// =============================================================================
// UI RENDERING
// =============================================================================

void render_ui(void) {
    if (game.player_count == 2) {
        // 2-player mode: show P1 info (top left), P2 info (top right)
        
        // P1 Score
        Print(1, 0, PSTR("P1"));
        PrintUInt16(4, 0, game.players[0].score);
        
        // P1 Lives
        Print(1, 1, PSTR("x"));
        PrintUInt8(3, 1, game.players[0].lives);
        
        // Level in center
        Print(14, 0, PSTR("LV"));
        PrintUInt8(17, 0, game.level_index + 1);
        
        // P2 Score
        Print(22, 0, PSTR("P2"));
        PrintUInt16(25, 0, game.players[1].score);
        
        // P2 Lives
        Print(22, 1, PSTR("x"));
        PrintUInt8(24, 1, game.players[1].lives);
    } else {
        // 1-player mode: show more info
        
        // Score display
        Print(1, 0, PSTR("SCORE"));
        PrintUInt16(7, 0, game.players[0].score);
        
        // Level indicator
        Print(14, 0, PSTR("LV"));
        PrintUInt8(17, 0, game.level_index + 1);
        
        // Lives
        Print(28, 0, PSTR("x"));
        PrintUInt8(30, 0, game.players[0].lives);
    }
}

// =============================================================================
// MAIN RENDER
// =============================================================================

void render(void) {
    switch (game.state) {
        case STATE_TITLE:
            render_title_screen();
            break;
        case STATE_GAME:
            render_level();
            // Render entities
            for (uint8_t i = 0; i < game.bubble_count; i++) {
                bubble_render(&game.bubbles[i]);
            }
            for (uint8_t i = 0; i < game.enemy_count; i++) {
                enemy_render(&game.enemies[i]);
            }
            for (uint8_t i = 0; i < game.player_count; i++) {
                player_render(&game.players[i]);
            }
            render_ui();
            break;
        case STATE_LEVEL_TRANSITION:
            render_level();
            render_ui();
            break;
        case STATE_GAME_OVER:
            render_game_over();
            break;
        case STATE_SCORE_SCREEN:
            render_score_screen();
            break;
    }
}

// =============================================================================
// MAIN UPDATE
// =============================================================================

void game_update(void) {
    switch (game.state) {
        case STATE_TITLE:
            update_title_screen();
            break;
        case STATE_GAME:
            update_game();
            break;
        case STATE_LEVEL_TRANSITION:
            update_level_transition();
            break;
        case STATE_GAME_OVER:
            update_game_over();
            break;
        case STATE_SCORE_SCREEN:
            update_score_screen();
            break;
    }
}

// =============================================================================
// MAIN GAME LOOP (called from Uzebox kernel)
// =============================================================================

void GameLoop(void) {
    game.frame_counter++;

    // Test: Draw a simple pattern every frame
    static uint8_t frame = 0;
    frame++;
    
    // Fill entire screen with tile 0 on black, tile 1 on white checkerboard
    for (uint8_t y = 0; y < 28; y++) {
        for (uint8_t x = 0; x < 32; x++) {
            vram[(y) * VRAM_TILES_H + (x)] = ((x + y + (frame >> 4)) % 2) ? 1 : 0;
        }
    }

    // Read input
    game_read_input();

    // Update game state
    game_update();

    // Render
    render();
}
