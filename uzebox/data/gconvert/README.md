# gconvert Configuration Files

This directory contains gconvert XML configuration files for converting PNG images from the `../../Assets/` directory to Uzebox tilesets.

## Prerequisites

Download gconvert from the Uzebox tools:
- https://uzebox.org/wiki/Generating_Tiles_and_Maps_with_gconvert

## Files

### tileset.xml
Converts background tiles from `LevelTiles.png` (8x8 pixels).
- Source: `../../Assets/LevelTiles.png` (40x200 pixels = 5x25 tiles)
- Output: `tileset.inc` (C include file for `SetTileTable()`)

### sprites.xml
Converts player character sprites (16x16 pixels = 2x2 tiles).
- Sources: `../../Assets/BubbleCharacter.png` (96x64), `../../Assets/BobbleCharacter.png` (96x64)
- Output: `sprites.inc` (C include file for `SetSpritesTileTable()`)

### enemies.xml
Converts enemy sprites (16x16 pixels).
- Source: `../../Assets/Enemys.png` (256x192 = 8x6 sprites)
- Output: `enemies.inc` (C include file for `SetSpritesTileTable()`)

### items.xml
Converts item and bubble sprites (16x16 pixels).
- Sources: `../../Assets/Items.png` (576x64), `../../Assets/Bubble.png` (320x32)
- Output: `items.inc` (C include file for `SetSpritesTileTable()`)

## Usage

1. **Create palette file:**
   ```bash
   gconvert -p ../../Assets/YourImage.png -o palette.bmp
   ```

2. **Convert tiles:**
   ```bash
   gconvert -c tileset.xml
   ```

3. **Convert sprites:**
   ```bash
   gconvert -c sprites.xml
   gconvert -c enemies.xml
   gconvert -c items.xml
   ```

4. **Copy output files to src/ directory:**
   ```bash
   cp tileset.inc ../src/
   cp sprites.inc ../src/
   ```

## Asset Dimensions

| File | Dimensions | Tiles (8x8) | Sprites (16x16) |
|------|-----------|-------------|-----------------|
| LevelTiles.png | 40x200 | 5x25 = 125 tiles | N/A |
| BubbleCharacter.png | 96x64 | 12x8 | 6x4 = 24 |
| BobbleCharacter.png | 96x64 | 12x8 | 6x4 = 24 |
| Enemys.png | 256x192 | 32x24 | 16x12 = 192 |
| Items.png | 576x64 | 72x8 | 36x4 = 144 |
| Bubble.png | 320x32 | 40x4 | 20x2 = 40 |
| Levels.png | 32x84 | 4x10 | 2x5 = 10 |

## Notes

- Mode 3 supports up to 256 tiles in the tileset
- Each tile is 8x8 pixels = 64 bytes
- Total tileset size: 256 × 64 = 16,384 bytes (16KB)
- Sprites are drawn using the sprites[] array, not VRAM tiles
- All source images should use 8-bit indexed color (256 colors or less)