# BubbleBobble Uzebox Port

A port of the BubbleBobble game to the Uzebox retro game console.

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

### Implemented
- ✅ Player movement (walk, jump)
- ✅ Bubble shooting
- ✅ Enemy AI (ZenChan, Maita)
- ✅ Enemy capture in bubbles
- ✅ Fruit drops from defeated enemies
- ✅ Score system
- ✅ Lives system
- ✅ Multiple levels
- ✅ Level transitions

### TODO
- [ ] Sprite animation frames
- [ ] Sound effects
- [ ] 2-player support
- [ ] Title screen menu
- [ ] Game over / Score screen
- [ ] Sprite data from original PNGs

## Project Structure

```
uzebox/
├── main.c              # Main entry point, Uzebox kernel integration
├── Makefile            # Build configuration
├── convert_sprites.py  # PNG to C sprite converter (basic)
├── convert_sprites_v2.py # PNG to C sprite converter (with sprite sheets)
├── convert_sprites_v3.py # PNG to C sprite converter (optimized)
├── README.md           # This file
├── data/
│   ├── sprites.h          # Full sprite data (76 tiles, 4864 bytes)
│   ├── sprite_data.h       # Complete sprite tile data with defines
│   └── sprites_compact.h  # Compact sprite indices
└── src/
    ├── config.h         # Game configuration & constants
    ├── types.h         # Game data structures
    ├── level_data.h    # Level maps (converted from Levels.png)
    ├── player.c         # Player entity & state machine
    ├── enemy.c         # Enemy entities & AI
    ├── bubble.c        # Bubble projectile system
    ├── collision.c      # Collision detection
    └── game.c          # Main game loop & state management
```

## Sprite Data

Sprites have been auto-converted from the original PNG assets:

| Sprite | Source File | Tiles | Memory |
|--------|-------------|-------|--------|
| Player 1 | BubbleCharacter.png | 4 frames × 4 tiles | 1024 bytes |
| Player 2 | BobbleCharacter.png | 4 frames × 4 tiles | 1024 bytes |
| Bubbles | BubbleLarge.png | 3 frames × 4 tiles | 768 bytes |
| ZenChan | Enemys.png | 4 frames × 4 tiles | 1024 bytes |
| Maita | Enemys.png | 2 frames × 4 tiles | 512 bytes |
| Watermelon | Items.png | 1 tile | 64 bytes |
| Fries | Items.png | 1 tile | 64 bytes |
| Solid Tile | LevelTiles.png | 1 tile | 64 bytes |
| Semi Tile | LevelTiles.png | 1 tile | 64 bytes |

**Total: 76 tiles, 4,864 bytes**

### Converting Sprites

To regenerate sprite data after modifying PNGs:

```bash
python3 convert_sprites_v3.py
```

This will convert all sprites from `../Assets/` and output to `data/sprites.h`.

## Memory Usage

Estimated RAM usage:
- Game context: ~512 bytes
- Player entities: ~128 bytes × 2
- Enemy entities: ~128 bytes × 8
- Bubble entities: ~128 bytes × 8
- Level tiles: 32×28 = 896 bytes

Total: ~2.5 KB RAM (fits in ATMega644's 4 KB RAM)

## Credits

- Original game concept: Taito (Bubble Bobble)
- This port: OpenHands AI
- Uzebox: Belogic Software (https://uzebox.org)

## License

This port inherits the license from the original BubbleBobble project.

## References

- [Uzebox Wiki](https://uzebox.org/wiki/)
- [Uzebox Mode 3 Documentation](https://uzebox.org/wiki/Video_Mode_3)
- [Uzebox Forum](https://uzebox.org/forums/)