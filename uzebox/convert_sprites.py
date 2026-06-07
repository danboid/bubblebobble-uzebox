#!/usr/bin/env python3
"""
PNG to Uzebox Sprite Converter

Converts PNG images to C byte arrays suitable for the Uzebox Mode 3 video mode.
Each pixel becomes one byte (palette index for 256-color mode).

Usage:
    python3 convert_sprites.py [input.png] [output.h] [tile_index]
    
Example:
    python3 convert_sprites.py BubbleCharacter.png sprites.h 0
"""

import sys
import os
from PIL import Image

def load_png_and_convert_to_tiles(filepath, tile_width=8, tile_height=8):
    """Load a PNG and extract 8x8 tiles as byte arrays."""
    try:
        img = Image.open(filepath)
        img = img.convert('RGBA')
        
        width, height = img.size
        tiles = []
        
        # Extract tiles row by row, left to right, top to bottom
        for y in range(0, height, tile_height):
            for x in range(0, width, tile_width):
                # Extract tile
                tile_data = []
                for ty in range(tile_height):
                    for tx in range(tile_width):
                        if y + ty < height and x + tx < width:
                            pixel = img.getpixel((x + tx, y + ty))
                            # Convert RGBA to palette index (simplified - just use R for demo)
                            # In a real converter, you'd use a color palette
                            r, g, b, a = pixel
                            if a < 128:
                                # Transparent - use index 0
                                tile_data.append(0)
                            else:
                                # Map RGB to Uzebox palette index (simplified mapping)
                                color_index = rgb_to_uzebox_color(r, g, b)
                                tile_data.append(color_index)
                        else:
                            tile_data.append(0)
                tiles.append(tile_data)
        
        return tiles, width // tile_width, height // tile_height
    except Exception as e:
        print(f"Error loading {filepath}: {e}")
        return None, 0, 0

def rgb_to_uzebox_color(r, g, b):
    """
    Map RGB to a Uzebox-compatible palette index.
    This is a simplified mapping - a proper implementation would use
    the actual Uzebox palette.
    """
    # Simplify to grayscale first
    gray = (r + g + b) // 3
    
    # Map to 16-level grayscale (simplified)
    if gray < 16:
        return 0x00
    elif gray < 32:
        return 0x01
    elif gray < 48:
        return 0x02
    elif gray < 64:
        return 0x03
    elif gray < 80:
        return 0x04
    elif gray < 96:
        return 0x05
    elif gray < 112:
        return 0x06
    elif gray < 128:
        return 0x07
    elif gray < 144:
        return 0x08
    elif gray < 160:
        return 0x09
    elif gray < 176:
        return 0x0A
    elif gray < 192:
        return 0x0B
    elif gray < 208:
        return 0x0C
    elif gray < 224:
        return 0x0D
    elif gray < 240:
        return 0x0E
    else:
        return 0x0F

def generate_c_array(tiles, name, tiles_across, tiles_down):
    """Generate C code for the sprite tiles."""
    lines = []
    lines.append(f"// {name}")
    lines.append(f"// {tiles_across} x {tiles_down} tiles ({tiles_across * tiles_down} total)")
    lines.append(f"// Each tile is 8x8 pixels")
    lines.append("")
    
    total_tiles = len(tiles)
    lines.append(f"// Total: {total_tiles} tiles")
    lines.append("")
    
    # Calculate unique palette entries
    all_bytes = [byte for tile in tiles for byte in tile]
    unique_bytes = sorted(set(all_bytes))
    
    lines.append(f"// Unique colors used: {len(unique_bytes)}")
    lines.append(f"// Color indices: {[hex(b) for b in unique_bytes]}")
    lines.append("")
    
    # Generate compressed tile data (RLE for repeated tiles)
    # For now, just output all tiles
    lines.append(f"const char {name}[] PROGMEM = {{")
    
    # Output as rows of hex bytes
    for i, tile in enumerate(tiles):
        # Add comment with tile position
        tile_x = i % tiles_across
        tile_y = i // tiles_across
        
        if i % 4 == 0:
            lines.append(f"    // Tile {i} ({tile_x},{tile_y})")
        
        # Output tile data as hex bytes (8 bytes per row = 1 pixel row)
        for row in range(8):
            row_start = row * 8
            row_data = tile[row_start:row_start + 8]
            hex_str = ", ".join(f"0x{b:02X}" for b in row_data)
            lines.append(f"    {hex_str},")
        
        if i < total_tiles - 1:
            lines.append("")
    
    lines.append("};")
    lines.append("")
    lines.append(f"// {total_tiles * 64} bytes total")
    
    return "\n".join(lines)

def main():
    if len(sys.argv) < 3:
        print("Usage: python3 convert_sprites.py <input.png> <output.h> [name]")
        print("")
        print("Example:")
        print("  python3 convert_sprites.py BubbleCharacter.png bubble_sprites.h BubbleSprite")
        return 1
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    name = sys.argv[3] if len(sys.argv) > 3 else os.path.splitext(os.path.basename(input_file))[0]
    
    if not os.path.exists(input_file):
        print(f"Error: {input_file} not found")
        return 1
    
    print(f"Converting {input_file}...")
    
    tiles, tiles_across, tiles_down = load_png_and_convert_to_tiles(input_file)
    
    if tiles is None:
        print("Failed to load image")
        return 1
    
    print(f"Image size: {tiles_across * 8} x {tiles_down * 8} pixels")
    print(f"Tiles: {tiles_across} x {tiles_down} = {len(tiles)} tiles")
    print(f"Total bytes: {len(tiles) * 64}")
    
    # Generate C code
    c_code = generate_c_array(tiles, name, tiles_across, tiles_down)
    
    # Write output
    with open(output_file, 'w') as f:
        f.write(f"/*\n")
        f.write(f" * Auto-generated sprite data from {os.path.basename(input_file)}\n")
        f.write(f" * Generated by convert_sprites.py\n")
        f.write(f" */\n")
        f.write(f"\n")
        f.write(c_code)
    
    print(f"Written to {output_file}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())