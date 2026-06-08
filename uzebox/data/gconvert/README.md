# gconvert Configuration Files

This directory contains gconvert XML configuration files for converting PNG images to Uzebox tilesets.

## Prerequisites

Download gconvert from the Uzebox tools:
- https://uzebox.org/wiki/Generating_Tiles_and_Maps_with_gconvert

## Files

### tileset.xml
Converts background tiles (8x8 pixels) for use with `SetTileTable()`.
- Input: `tiles.png` (your tile image)
- Output: `tileset.inc` (C include file)

### sprites.xml
Converts sprite sheets (16x16 pixels = 2x2 tiles) for use with `SetSpritesTileTable()`.
- Input: `spritesheet.png` (sprite sheet with 2x2 tile sprites)
- Output: `sprites.inc` (C include file)

## Usage

1. **Prepare your images:**
   - Create PNG images with 8-bit indexed color (256 colors or less)
   - Include a palette file (`palette.bmp`) that defines your color palette
   - For tiles: one tile per 8x8 pixel cell
   - For sprites: arrange sprites in a grid, each sprite being 16x16 (2x2 tiles)

2. **Create palette:**
   ```
   gconvert -p palette.png -o palette.bmp
   ```

3. **Convert tiles:**
   ```
   gconvert -c tileset.xml
   ```

4. **Convert sprites:**
   ```
   gconvert -c sprites.xml
   ```

5. **Copy output files to src/ directory:**
   ```bash
   cp tileset.inc ../src/
   cp sprites.inc ../src/
   ```

## Image Requirements

- Format: PNG with 8-bit indexed color
- Tile size: 8x8 pixels
- Sprite size: 16x16 pixels (2x2 tiles)
- Color palette: Must match your game's palette

## Example Directory Structure

```
uzebox/
└── data/
    └── gconvert/
        ├── tileset.xml
        ├── sprites.xml
        ├── palette.bmp
        ├── tiles.png      # Your tile image (32 tiles wide max)
        └── spritesheet.png  # Your sprite sheet
```

## Notes

- Mode 3 supports up to 256 tiles in the tileset
- Each tile is 8x8 pixels = 64 bytes
- Total tileset size: 256 × 64 = 16,384 bytes (16KB)
- Sprites are drawn using the sprites[] array, not VRAM tiles