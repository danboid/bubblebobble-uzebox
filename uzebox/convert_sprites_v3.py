#!/usr/bin/env python3
"""
PNG to Uzebox Sprite Converter v3

Creates properly indexed sprite data for the Uzebox.
Each sprite gets a unique index, and we generate both:
1. The actual tile data (stored in flash)
2. A lookup table for sprite animation frames

"""

import os
from PIL import Image

# Color palette for Uzebox (simplified grayscale + common colors)
PALETTE = {
    # Transparent
    (0, 0, 0, 0): 0x00,
    
    # Black
    (0, 0, 0): 0x00,
    
    # Grays
    (32, 32, 32): 0x11,
    (64, 64, 64): 0x22,
    (96, 96, 96): 0x33,
    (128, 128, 128): 0x44,
    (160, 160, 160): 0x55,
    (192, 192, 192): 0x66,
    (224, 224, 224): 0x77,
    
    # White
    (255, 255, 255): 0xFF,
    
    # Red
    (255, 64, 64): 0x01,
    (200, 50, 50): 0x02,
    
    # Yellow
    (255, 255, 0): 0x10,
    (255, 200, 0): 0x20,
    
    # Green (Player 1)
    (92, 230, 52): 0x12,
    (70, 200, 40): 0x13,
    (50, 170, 30): 0x14,
    
    # Blue (Player 2)
    (52, 168, 230): 0x23,
    (40, 140, 200): 0x24,
    (30, 110, 180): 0x25,
    
    # Bubble colors
    (154, 221, 250): 0x9A,  # Light blue
    (103, 193, 231): 0x67,  # Medium blue
    (200, 230, 250): 0x8A,  # Highlight
    (80, 160, 220): 0x57,   # Shadow
    
    # Orange
    (255, 165, 0): 0x30,
    
    # Pink
    (255, 192, 203): 0x40,
}

def rgb_to_index(r, g, b, a=255):
    """Convert RGBA to palette index."""
    if a < 128:
        return 0x00
    
    # Check exact RGBA match first
    key = (r, g, b, a)
    if key in PALETTE:
        return PALETTE[key]
    
    # Check RGB match
    key = (r, g, b)
    if key in PALETTE:
        return PALETTE[key]
    
    # Fallback to grayscale
    gray = (r + g + b) // 3
    return gray


def extract_sprite_tiles(img, x, y, width, height):
    """Extract 8x8 tiles from a region of the image."""
    tiles = []
    
    for ty in range(0, height, 8):
        for tx in range(0, width, 8):
            tile_data = []
            for py in range(8):
                row = []
                for px in range(8):
                    px_abs = x + tx + px
                    py_abs = y + ty + py
                    
                    if px_abs < img.width and py_abs < img.height:
                        pixel = img.getpixel((px_abs, py_abs))
                        if len(pixel) == 4:
                            r, g, b, a = pixel
                        else:
                            r, g, b = pixel
                            a = 255
                        row.append(rgb_to_index(r, g, b, a))
                    else:
                        row.append(0x00)
                tile_data.append(row)
            tiles.append(tile_data)
    
    return tiles


def generate_tile_c(tiles, name):
    """Generate C array for tiles."""
    lines = []
    lines.append(f"// {name} - {len(tiles)} tiles ({len(tiles) * 64} bytes)")
    lines.append(f"const char {name}[] PROGMEM = {{")
    
    for i, tile in enumerate(tiles):
        if i % 4 == 0:
            lines.append(f"    // Tile {i}")
        for row in tile:
            hex_str = ", ".join(f"0x{b:02X}" for b in row)
            lines.append(f"    {hex_str},")
        if i < len(tiles) - 1:
            lines.append("")
    
    lines.append("};")
    lines.append("")
    
    return "\n".join(lines)


def main():
    # Sprite definitions - position in source image (in pixels)
    sprites = {
        # Player 1 (Bubble) - 16x16, first row of BubbleCharacter.png
        'P1_IDLE':   {'file': '../Assets/BubbleCharacter.png', 'x': 0, 'y': 0, 'w': 16, 'h': 16},
        'P1_WALK0':  {'file': '../Assets/BubbleCharacter.png', 'x': 16, 'y': 0, 'w': 16, 'h': 16},
        'P1_WALK1':  {'file': '../Assets/BubbleCharacter.png', 'x': 32, 'y': 0, 'w': 16, 'h': 16},
        'P1_JUMP':   {'file': '../Assets/BubbleCharacter.png', 'x': 48, 'y': 0, 'w': 16, 'h': 16},
        'P1_ATTACK': {'file': '../Assets/BubbleCharacter.png', 'x': 64, 'y': 0, 'w': 16, 'h': 16},
        
        # Player 2 (Bobble) - 16x16, first row of BobbleCharacter.png
        'P2_IDLE':   {'file': '../Assets/BobbleCharacter.png', 'x': 0, 'y': 0, 'w': 16, 'h': 16},
        'P2_WALK0':  {'file': '../Assets/BobbleCharacter.png', 'x': 16, 'y': 0, 'w': 16, 'h': 16},
        'P2_WALK1':  {'file': '../Assets/BobbleCharacter.png', 'x': 32, 'y': 0, 'w': 16, 'h': 16},
        'P2_JUMP':   {'file': '../Assets/BobbleCharacter.png', 'x': 48, 'y': 0, 'w': 16, 'h': 16},
        
        # Bubbles - 16x16, various rows of BubbleLarge.png
        'BUBBLE_SPAWN': {'file': '../Assets/BubbleLarge.png', 'x': 0, 'y': 0, 'w': 16, 'h': 16},
        'BUBBLE_IDLE':  {'file': '../Assets/BubbleLarge.png', 'x': 16, 'y': 0, 'w': 16, 'h': 16},
        'BUBBLE_POP':   {'file': '../Assets/BubbleLarge.png', 'x': 32, 'y': 0, 'w': 16, 'h': 16},
        
        # ZenChan - 16x16, first row of Enemys.png
        'ZENCHAN_NORMAL':  {'file': '../Assets/Enemys.png', 'x': 0, 'y': 0, 'w': 16, 'h': 16},
        'ZENCHAN_WALK':    {'file': '../Assets/Enemys.png', 'x': 16, 'y': 0, 'w': 16, 'h': 16},
        'ZENCHAN_CHARGE':  {'file': '../Assets/Enemys.png', 'x': 32, 'y': 0, 'w': 16, 'h': 16},
        'ZENCHAN_DEAD':    {'file': '../Assets/Enemys.png', 'x': 48, 'y': 0, 'w': 16, 'h': 16},
        
        # Maita - 16x16, second row of Enemys.png
        'MAITA_NORMAL': {'file': '../Assets/Enemys.png', 'x': 0, 'y': 16, 'w': 16, 'h': 16},
        'MAITA_WALK':   {'file': '../Assets/Enemys.png', 'x': 16, 'y': 16, 'w': 16, 'h': 16},
        
        # Items - 8x8
        'WATERMELON': {'file': '../Assets/Items.png', 'x': 19*8, 'y': 0, 'w': 8, 'h': 8},
        'FRIES':      {'file': '../Assets/Items.png', 'x': 25*8, 'y': 0, 'w': 8, 'h': 8},
        
        # Level tiles - 8x8
        'TILE_SOLID': {'file': '../Assets/LevelTiles.png', 'x': 0, 'y': 0, 'w': 8, 'h': 8},
        'TILE_SEMI':  {'file': '../Assets/LevelTiles.png', 'x': 0, 'y': 8, 'w': 8, 'h': 8},
    }
    
    # Load all images
    images = {}
    for name, info in sprites.items():
        filepath = os.path.join(os.path.dirname(__file__), info['file'])
        if info['file'] not in images:
            try:
                images[info['file']] = Image.open(filepath).convert('RGBA')
                print(f"Loaded {info['file']}")
            except Exception as e:
                print(f"Error loading {filepath}: {e}")
    
    # Extract all tiles and assign indices
    all_tiles = []
    sprite_indices = {}
    tile_index = 0
    
    c_data_lines = []
    c_data_lines.append("/*")
    c_data_lines.append(" * BubbleBobble Uzebox - Sprite Tile Data")
    c_data_lines.append(" * Auto-generated from PNG assets")
    c_data_lines.append(" *")
    c_data_lines.append(" * Each tile is 8x8 pixels, 1 byte per pixel (palette index)")
    c_data_lines.append(" */")
    c_data_lines.append("")
    c_data_lines.append("#ifndef SPRITE_DATA_H")
    c_data_lines.append("#define SPRITE_DATA_H")
    c_data_lines.append("")
    
    for name, info in sprites.items():
        if info['file'] not in images:
            continue
        
        img = images[info['file']]
        tiles = extract_sprite_tiles(img, info['x'], info['y'], info['w'], info['h'])
        
        sprite_indices[name] = {
            'start': tile_index,
            'count': len(tiles),
        }
        
        # Generate C data for these tiles
        c_data_lines.append(f"// {name} (index {tile_index}, {len(tiles)} tiles)")
        c_data_lines.append(f"const char sprite_{name}[] PROGMEM = {{")
        
        for i, tile in enumerate(tiles):
            for row in tile:
                hex_str = ", ".join(f"0x{b:02X}" for b in row)
                c_data_lines.append(f"    {hex_str},")
            if i < len(tiles) - 1:
                c_data_lines.append("")
        
        c_data_lines.append("};")
        c_data_lines.append("")
        
        tile_index += len(tiles)
        all_tiles.extend(tiles)
    
    # Generate sprite base indices as defines
    c_data_lines.append("// ============================================")
    c_data_lines.append("// SPRITE BASE INDICES")
    c_data_lines.append("// Use these as base indices for animation frames")
    c_data_lines.append("// ============================================")
    c_data_lines.append("")
    
    for name, idx in sprite_indices.items():
        c_data_lines.append(f"#define SPRITE_{name.upper()} {idx['start']}")
        c_data_lines.append(f"#define SPRITE_{name.upper()}_TILES {idx['count']}")
    
    c_data_lines.append("")
    
    # Generate tile data array (all tiles in one array for easy loading)
    c_data_lines.append("// ============================================")
    c_data_lines.append("// ALL TILES (for VRAM)")
    c_data_lines.append("// ============================================")
    c_data_lines.append("")
    c_data_lines.append(f"// Total: {len(all_tiles)} tiles, {len(all_tiles) * 64} bytes")
    c_data_lines.append("")
    c_data_lines.append("const char all_sprites[] PROGMEM = {")
    
    for i, tile in enumerate(all_tiles):
        if i % 8 == 0:
            c_data_lines.append(f"    // Tile {i}")
        for row in tile:
            hex_str = ", ".join(f"0x{b:02X}" for b in row)
            c_data_lines.append(f"    {hex_str},")
    
    c_data_lines.append("};")
    c_data_lines.append("")
    c_data_lines.append("#endif // SPRITE_DATA_H")
    
    # Write sprite data file
    output_path = os.path.join(os.path.dirname(__file__), 'data', 'sprite_data.h')
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    with open(output_path, 'w') as f:
        f.write("\n".join(c_data_lines))
    
    print(f"\nWritten sprite data to {output_path}")
    print(f"Total tiles: {len(all_tiles)}")
    print(f"Memory usage: {len(all_tiles) * 64} bytes")
    
    # Print summary of indices
    print("\nSprite indices:")
    for name, idx in sprite_indices.items():
        print(f"  {name}: {idx['start']} ({idx['count']} tiles)")


if __name__ == "__main__":
    main()