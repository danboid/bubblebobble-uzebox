# BubbleBobble Uzebox Port

A port of the BubbleBobble game to the Uzebox retro game console.

## ⚠️ Development Status

**CURRENTLY IN DEVELOPMENT**

The project builds successfully. Major RAM optimizations have been applied to fit within the ATMega644's 4KB RAM constraint.

### What's Working
- ✅ Code compiles without errors
- ✅ Game loop structure implemented
- ✅ Player, enemy, and bubble entities defined
- ✅ Level data structure defined
- ✅ Collision detection implemented
- ✅ Basic game state machine (title, game, game over)
- ✅ RAM usage optimized (~1KB freed by removing level_tiles duplicate)
- ✅ 2-player support (P1 and P2 can start independently)
- ✅ Font support for text rendering
- ✅ UI rendering (score, level, lives display)

### Known Issues
- Video output may not display properly on some emulators
- Real hardware testing recommended for full verification

## Overview

This is a C implementation of a Bubble Bobble-style game designed for the Uzebox, an open-source 8-bit game console based on the Atmel ATMega644 microcontroller.

## Hardware Requirements

- **Uzebox** (or compatible clone)
- **ATMega644p** microcontroller @ 28.636 MHz
- **SNES controller** for input
- TV with composite video input (NTSC)

## Video Mode

This port uses **Mode 3 Extended (no scrolling)**:
- Resolution: 256×224 pixels (32×28 tiles)
- Tile size: 8×8 pixels
- Color depth: 256 colors
- VRAM: 32×28 tiles

This matches the game's original grid size of 32×28, making it a perfect fit!

## Building

### Prerequisites

1. Install the Uzebox toolchain:
   ```bash
   # On Linux/macOS with AVR toolchain
   sudo apt-get install avrdude avr-libc binutils-avr gcc-avr
   
   # Or use the official Uzebox toolchain from https://uzebox.org/wiki/Getting_Started
   ```

2. Get the Uzebox kernel sources:
   ```bash
   cd ..
   git clone https://github.com/Uzebox/uzebox.git
   ```

### Build

The project expects the Uzebox kernel to be at `../../kernel` relative to this directory. 
Your directory structure should look like:

```
uzebox/                    <- this project (demos/bubblebobble-uzebox/uzebox/)
kernel/                    <- Uzebox kernel (from git clone)
demos/
```

Clone the kernel if you haven't:

```bash
cd ../..
git clone https://github.com/Uzebox/uzebox.git
```

Then build:

```bash
cd demos/bubblebobble-uzebox/uzebox
make
```

This will produce:
- `BubbleBobble.hex` - Flash ROM image
- `BubbleBobble.eep` - EEPROM data (if needed)
- `BubbleBobble.lss` - Disassembly listing

### Flashing

```bash
# Using USBasp programmer
make flash

# Or manually with avrdude
avrdude -c usbasp -p m644p -U flash:w:bubblebobble.hex:i
```

## Controls

| Button | Action |
|--------|--------|
| D-Pad | Move left/right |
| A / Up | Jump |
| B | Fire bubble |
| Start | Start game / Pause |

## Game Features

### Implemented (Core Code)
- ✅ Player entity with state machine (walking, jumping, attacking, dead)
- ✅ Bubble shooting mechanism
- ✅ Enemy entities (ZenChan, Maita) with AI
- ✅ Enemy capture in bubbles (pop out transformed)
- ✅ Collision detection system
- ✅ Game state management (title, game, level transition, game over)
- ✅ Level data structure (3 levels defined)
- ✅ Input handling (SNES controller)

### Missing / Incomplete
- ❌ Sprite rendering to screen (using sprites[] array)
- ❌ Sound effects and music
- ❌ Proper tileset with real graphics (using gconvert)

### TODO (Priority Order)
1. **Rendering**
   - [ ] Implement sprite drawing using sprites[] array
   - [ ] Create proper tileset using gconvert

2. **Testing**
   - [ ] Test on real Uzebox hardware
   - [ ] Verify video displays correctly on real TV

3. **Polish**
   - [ ] Add sound effects
   - [ ] Add background music

## Project Structure

```
uzebox/
├── main.c              # Main entry point, Uzebox kernel integration
├── Makefile            # Build configuration
├── README.md           # This file
├── data/
│   └── gconvert/       # gconvert XML configs for asset conversion
└── src/
    ├── config.h        # Game configuration & constants
    ├── types.h         # Game data structures
    ├── level_data.h    # Level maps in Flash (PROGMEM)
    ├── level_data.c    # Level loading functions
    ├── player.c        # Player entity & state machine
    ├── enemy.c         # Enemy entities & AI
    ├── bubble.c        # Bubble projectile system
    ├── collision.c     # Collision detection
    ├── game.c          # Main game loop & state management
    ├── game.h          # Game header with extern declarations
    └── tileset.inc     # Tileset for video mode (use gconvert for production)
```

**Note:** Use `gconvert` to generate proper tiles from PNG files. See the gconvert directory for XML configuration files.

### Converting Sprites

To convert game assets to Uzebox format, use the gconvert XML configuration files in the [data/gconvert/](https://github.com/danboid/bubblebobble-uzebox/tree/main/uzebox/data/gconvert) directory. This includes tilesets, sprites, enemies, and items.

## Memory Usage

Estimated RAM usage (after optimizations):
- Game context: ~400 bytes (reduced from level_tiles removal)
- Player entities: ~52 bytes × 2 = 104 bytes
- Enemy entities: ~52 bytes × 8 = 416 bytes
- Bubble entities: ~52 bytes × 8 = 416 bytes
- Pickup/DeadEnemy entities: ~36 bytes × 8 = 288 bytes
- VRAM: 32×28 = 896 bytes (used for video output)

Total: ~2.5 KB RAM (fits in ATMega644's 4 KB RAM with room for stack)

**Note:** Level tiles are no longer duplicated in RAM. Instead, collision checks read directly from Flash memory (PROGMEM), and VRAM is used only for video display.

## Credits

- Original game concept: Taito (Bubble Bobble)
- This port: Dan MacDonald (danboid) / OpenHands AI
- Uzebox: Belogic Software (https://uzebox.org)

## License

This port inherits the license from the original BubbleBobble project.

## Troubleshooting

### Video Output Issues

If you're seeing a black screen:

1. **Check the kernel**: Make sure you have the correct Uzebox kernel:
   ```bash
   cd ../.. && git clone https://github.com/Uzebox/uzebox.git
   ```

2. **Verify video mode settings**: This port uses Mode 3 with:
   - `VIDEO_MODE=3`
   - `SCROLLING=0`
   - `VRAM_TILES_H=32`, `VRAM_TILES_V=28`
   - `RESOLUTION_EXT=1`

3. **Test with emulator**: The recommended emulator is **cuzebox** (written in C):
   - https://uzebox.org/wiki/CUzebox

4. **Test on real hardware**: Emulators may not perfectly emulate all video modes. Testing on real Uzebox hardware is recommended for final verification.

## References

- [Uzebox Wiki](https://uzebox.org/wiki/)
- [Uzebox Mode 3 Documentation](https://uzebox.org/wiki/Video_Mode_3)
- [Uzebox Forum](https://uzebox.org/forums/)
- [gconvert Tool](https://uzebox.org/wiki/Generating_Tiles_and_Maps_with_gconvert)