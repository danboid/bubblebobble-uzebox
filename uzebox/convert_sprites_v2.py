#!/usr/bin/env python3
"""
PNG to Uzebox Sprite Converter v2

Creates optimized sprite data for the Uzebox with:
- Proper 8x8 tile extraction
- Palette index mapping
- Sprite atlas with tile references
- Memory-efficient output

Usage:
    python3 convert_sprites_v2.py
    
This converts all assets in ../Assets/ to sprites.h
"""

import os
from PIL import Image
import json

# Uzebox color palette (16 levels of gray as a starting point)
# In a real converter, you'd use the full 256-color palette
GRAY_PALETTE = [
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
]

# Color mapping for Bubble Bobble specific colors
COLOR_MAP = {
    # (R, G, B) -> palette index
    # Greens (player 1)
    (92, 230, 52): 0x12,   # Player 1 green
    (52, 168, 230): 0x23,  # Player 2 blue
    # Bubbles
    (154, 221, 250): 0x9A, # Light blue bubble
    (103, 193, 231): 0x67, # Darker bubble
    # UI
    (255, 255, 255): 0xFF, # White
    (0, 0, 0): 0x00,       # Black
    (255, 0, 0): 0x01,     # Red
    (255, 255, 0): 0x10,   # Yellow
    # Level tiles
    (52, 52, 52): 0x34,    # Tile fill
    (18, 18, 18): 0x12,    # Tile edge
}

def rgb_to_palette_index(r, g, b, a=255):
    """Convert RGBA to palette index."""
    if a < 128:
        return 0x00  # Transparent
    
    # Check exact color match first
    key = (r, g, b)
    if key in COLOR_MAP:
        return COLOR_MAP[key]
    
    # Fall back to grayscale
    gray = (r + g + b) // 3
    
    # Map to nearest palette entry
    best_idx = 0
    best_dist = 256 * 256 * 3
    for idx, pal_color in enumerate(GRAY_PALETTE):
        dist = abs(gray - pal_color)
        if dist < best_dist:
            best_dist = dist
            best_idx = idx
    
    return best_idx

def extract_tiles_from_spritesheet(img, tile_w=8, tile_h=8, sprite_w=16, sprite_h=16):
    """Extract animation frames from a sprite sheet."""
    frames = []
    sprites_per_row = img.width // sprite_w
    sprites_per_col = img.height // sprite_h
    frames_per_sprite = sprites_per_row  # Assuming sprites are arranged in animation frames
    
    for sy in range(sprites_per_col):
        for sx in range(sprites_per_row):
            tile_row = (sy * sprite_h) // tile_h
            tile_col = (sx * sprite_w) // tile_w
            
            for ty in range(sprite_h // tile_h):
                for tx in range(sprite_w // tile_w):
                    tile_x = sx * sprite_w + tx * tile_w
                    tile_y = sy * sprite_h + ty * tile_h
                    
                    tile_data = []
                    for py in range(tile_h):
                        row = []
                        for px in range(tile_w):
                            if tile_x + px < img.width and tile_y + py < img.height:
                                pixel = img.getpixel((tile_x + px, tile_y + py))
                                if len(pixel) == 4:
                                    r, g, b, a = pixel
                                else:
                                    r, g, b = pixel
                                    a = 255
                                row.append(rgb_to_palette_index(r, g, b, a))
                            else:
                                row.append(0)
                        tile_data.append(row)
                    frames.append(tile_data)
    
    return frames

def extract_tile_atlas(img, tile_w=8, tile_h=8):
    """Extract all 8x8 tiles from an image."""
    tiles = []
    width, height = img.size
    
    for y in range(0, height, tile_h):
        for x in range(0, width, tile_w):
            tile_data = []
            for py in range(tile_h):
                row = []
                for px in range(tile_w):
                    if x + px < width and y + py < height:
                        pixel = img.getpixel((x + px, y + py))
                        if len(pixel) == 4:
                            r, g, b, a = pixel
                        else:
                            r, g, b = pixel
                            a = 255
                        row.append(rgb_to_palette_index(r, g, b, a))
                    else:
                        row.append(0)
                tile_data.append(row)
            tiles.append(tile_data)
    
    return tiles

def generate_tile_data_c(tiles, name):
    """Generate C array for tile data."""
    lines = []
    lines.append(f"// Tile data for {name}")
    lines.append(f"// {len(tiles)} tiles, {len(tiles) * 64} bytes")
    lines.append("")
    lines.append(f"const char {name}_tiles[] PROGMEM = {{")
    
    for i, tile in enumerate(tiles):
        if i % 16 == 0:
            lines.append(f"    // Tile {i}")
        
        # Output as hex rows
        for row in tile:
            hex_str = ", ".join(f"0{b:02X}" for b in row)
            lines.append(f"    {hex_str},")
        
        if i < len(tiles) - 1:
            lines.append("")
    
    lines.append("};")
    lines.append("")
    
    return "\n".join(lines)

def generate_sprite_defines(sprite_data):
    """Generate #define statements for sprite tile indices."""
    lines = []
    lines.append("// Sprite tile indices")
    lines.append("// Format: #define SPRITE_NAME_TILE (base_index + frame * frames_per_row + row)")
    lines.append("")
    
    for sprite_name, info in sprite_data.items():
        lines.append(f"// {sprite_name}")
        lines.append(f"//   Size: {info['tile_w']}x{info['tile_h']} pixels ({info['tile_w']//8}x{info['tile_h']//8} tiles)")
        lines.append(f"//   Animation frames: {info['frames']}")
        lines.append("")
    
    return "\n".join(lines)

# Asset definitions with tile positions
ASSETS = {
    'Player1': {
        'file': '../Assets/BubbleCharacter.png',
        'tile_x': 0, 'tile_y': 0,
        'tile_w': 16, 'tile_h': 16,
        'frames': 4,
    },
    'Player2': {
        'file': '../Assets/BobbleCharacter.png', 
        'tile_x': 0, 'tile_y': 0,
        'tile_w': 16, 'tile_h': 16,
        'frames': 4,
    },
    'Bubble': {
        'file': '../Assets/BubbleLarge.png',
        'tile_x': 0, 'tile_y': 0,
        'tile_w': 16, 'tile_h': 16,
        'frames': 4,
    },
    'AttackBubble': {
        'file': '../Assets/AttackBubble.png',
        'tile_x': 0, 'tile_y': 0,
        'tile_w': 16, 'tile_h': 16,
        'frames': 4,
    },
    'ZenChan': {
        'file': '../Assets/Enemys.png',
        'tile_x': 0, 'tile_y': 0,
        'tile_w': 16, 'tile_h': 16,
        'frames': 4,
    },
    'Maita': {
        'file': '../Assets/Enemys.png',
        'tile_x': 8 * 8, 'tile_y': 0,
        'tile_w': 16, 'tile_h': 16,
        'frames': 4,
    },
    'Watermelon': {
        'file': '../Assets/Items.png',
        'tile_x': 19 * 8, 'tile_y': 0,
        'tile_w': 8, 'tile_h': 8,
        'frames': 1,
    },
    'Fries': {
        'file': '../Assets/Items.png',
        'tile_x': 25 * 8, 'tile_y': 0,
        'tile_w': 8, 'tile_h': 8,
        'frames': 1,
    },
    'TileSolid': {
        'file': '../Assets/LevelTiles.png',
        'tile_x': 0, 'tile_y': 0,
        'tile_w': 8, 'tile_h': 8,
        'frames': 1,
    },
    'TileSemi': {
        'file': '../Assets/LevelTiles.png',
        'tile_x': 0, 'tile_y': 8,
        'tile_w': 8, 'tile_h': 8,
        'frames': 1,
    },
}

def main():
    all_tiles = []
    sprite_info = {}
    tile_index = 0
    
    output_lines = []
    output_lines.append("/*")
    output_lines.append(" * BubbleBobble Uzebox - Sprite Data")
    output_lines.append(" * Auto-generated from PNG assets")
    output_lines.append(" *")
    output_lines.append(" * Tile format: 8x8 pixels, 1 byte per pixel (palette index)")
    output_lines.append(" * Memory: Each tile = 64 bytes")
    output_lines.append(" */")
    output_lines.append("")
    output_lines.append("#ifndef SPRITES_H")
    output_lines.append("#define SPRITES_H")
    output_lines.append("")
    
    # Process each asset
    for name, info in ASSETS.items():
        filepath = os.path.join(os.path.dirname(__file__), info['file'])
        
        if not os.path.exists(filepath):
            print(f"Warning: {filepath} not found, skipping {name}")
            continue
        
        print(f"Processing {name} from {info['file']}...")
        
        img = Image.open(filepath).convert('RGBA')
        
        # Extract tile(s) from the sprite sheet
        x, y = info['tile_x'], info['tile_y']
        w, h = info['tile_w'], info['tile_h']
        
        sprite_tiles = []
        for fy in range(h // 8):
            for fx in range(w // 8):
                tile_x = x + fx * 8
                tile_y = y + fy * 8
                
                tile_data = []
                for py in range(8):
                    row = []
                    for px in range(8):
                        if tile_x + px < img.width and tile_y + py < img.height:
                            pixel = img.getpixel((tile_x + px, tile_y + py))
                            if len(pixel) == 4:
                                r, g, b, a = pixel
                            else:
                                r, g, b = pixel
                                a = 255
                            row.append(rgb_to_palette_index(r, g, b, a))
                        else:
                            row.append(0)
                    tile_data.append(row)
                sprite_tiles.append(tile_data)
        
        # Store sprite info
        sprite_info[name] = {
            'tile_index': tile_index,
            'tile_count': len(sprite_tiles),
            'tile_w': w,
            'tile_h': h,
            'frames': info['frames']
        }
        
        # Add tiles to global tile array
        sprite_info[name]['start_index'] = len(all_tiles)
        all_tiles.extend(sprite_tiles)
        
        # Generate #defines
        output_lines.append(f"// {name}: {w}x{h} pixels, {len(sprite_tiles)} tiles")
        
        tile_index += len(sprite_tiles)
    
    # Generate global sprite data
    output_lines.append("")
    output_lines.append("// ============================================")
    output_lines.append("// GLOBAL SPRITE DATA")
    output_lines.append("// ============================================")
    output_lines.append("")
    output_lines.append(f"// Total tiles: {len(all_tiles)}")
    output_lines.append(f"// Total bytes: {len(all_tiles) * 64}")
    output_lines.append("")
    
    # Tile data array
    output_lines.append("const char sprite_tiles[] PROGMEM = {")
    
    for i, tile in enumerate(all_tiles):
        if i % 16 == 0:
            output_lines.append(f"    // Tile {i}")
        
        for row in tile:
            hex_str = ", ".join(f"0{b:02X}" for b in row)
            output_lines.append(f"    {hex_str},")
        
        if i < len(all_tiles) - 1:
            output_lines.append("")
    
    output_lines.append("};")
    output_lines.append("")
    
    # Generate sprite index defines
    output_lines.append("// ============================================")
    output_lines.append("// SPRITE INDICES")
    output_lines.append("// ============================================")
    output_lines.append("")
    
    tile_offset = 0
    for name, info in ASSETS.items():
        if name in sprite_info:
            si = sprite_info[name]
            base = si['start_index']
            output_lines.append(f"// {name}")
            output_lines.append(f"#define TILE_{name.upper()}_BASE {base}")
            output_lines.append(f"#define TILE_{name.upper()}_WIDTH {si['tile_w']//8}")
            output_lines.append(f"#define TILE_{name.upper()}_HEIGHT {si['tile_h']//8}")
            output_lines.append("")
    
    # Tile table for Mode 3 VRAM lookup
    output_lines.append("// ============================================")
    output_lines.append("// TILE LOOKUP TABLE")
    output_lines.append("// Maps tile index in VRAM to sprite tile")
    output_lines.append("// ============================================")
    output_lines.append("")
    output_lines.append("// For Mode 3, each VRAM entry is an index into this table")
    output_lines.append("// The Uzebox kernel will look up sprite_tiles[table[index]]")
    output_lines.append("")
    
    output_lines.append("#endif // SPRITES_H")
    
    # Write output
    output_file = os.path.join(os.path.dirname(__file__), 'data', 'sprites.h')
    os.makedirs(os.path.dirname(output_file), exist_ok=True)
    
    with open(output_file, 'w') as f:
        f.write("\n".join(output_lines))
    
    print(f"\nWritten to {output_file}")
    print(f"Total tiles: {len(all_tiles)}")
    print(f"Memory usage: {len(all_tiles) * 64} bytes")
    
    # Also create a compact version with just essential tiles
    create_compact_sprites(all_tiles, sprite_info)

def create_compact_sprites(all_tiles, sprite_info):
    """Create a compact sprite data file with only essential tiles."""
    
    # We need:
    # - Player idle (1 frame)
    # - Player walk (2 frames)
    # - Player jump (1 frame)
    # - Player attack (2 frames)
    # - Bubble (2 frames)
    # - ZenChan normal (2 frames)
    # - ZenChan charge (1 frame)
    # - ZenChan dead (1 frame)
    # - Maita (2 frames)
    # - Watermelon (1 frame)
    # - Fries (1 frame)
    # - Tile solid (1 frame)
    # - Tile semi (1 frame)
    
    essential_indices = []
    
    # Map sprite names to essential tile indices
    essential_map = {
        'P1_IDLE': sprite_info.get('Player1', {}).get('start_index', 0),
        'P1_WALK0': sprite_info.get('Player1', {}).get('start_index', 0) + 1,
        'P1_WALK1': sprite_info.get('Player1', {}).get('start_index', 0) + 2,
        'P1_JUMP': sprite_info.get('Player1', {}).get('start_index', 0) + 3,
        'P1_ATTACK0': sprite_info.get('Player1', {}).get('start_index', 0) + 4,
        'P1_ATTACK1': sprite_info.get('Player1', {}).get('start_index', 0) + 5,
        
        'P2_IDLE': sprite_info.get('Player2', {}).get('start_index', 0),
        'P2_WALK0': sprite_info.get('Player2', {}).get('start_index', 0) + 1,
        'P2_WALK1': sprite_info.get('Player2', {}).get('start_index', 0) + 2,
        'P2_JUMP': sprite_info.get('Player2', {}).get('start_index', 0) + 3,
        
        'BUBBLE': sprite_info.get('Bubble', {}).get('start_index', 0),
        'BUBBLE_POP': sprite_info.get('Bubble', {}).get('start_index', 0) + 4,
        
        'ZENCHAN_1': sprite_info.get('ZenChan', {}).get('start_index', 0),
        'ZENCHAN_2': sprite_info.get('ZenChan', {}).get('start_index', 0) + 1,
        'ZENCHAN_CHARGE': sprite_info.get('ZenChan', {}).get('start_index', 0) + 8,
        
        'MAITA_1': sprite_info.get('Maita', {}).get('start_index', 0),
        'MAITA_2': sprite_info.get('Maita', {}).get('start_index', 0) + 1,
        
        'WATERMELON': sprite_info.get('Watermelon', {}).get('start_index', 0),
        'FRIES': sprite_info.get('Fries', {}).get('start_index', 0),
        
        'TILE_SOLID': sprite_info.get('TileSolid', {}).get('start_index', 0),
        'TILE_SEMI': sprite_info.get('TileSemi', {}).get('start_index', 0),
    }
    
    # Create compact data file
    output_lines = []
    output_lines.append("/*")
    output_lines.append(" * BubbleBobble Uzebox - Compact Sprite Data")
    output_lines.append(" * Essential tiles only for game sprites")
    output_lines.append(" */")
    output_lines.append("")
    output_lines.append("#ifndef SPRITES_COMPACT_H")
    output_lines.append("#define SPRITES_COMPACT_H")
    output_lines.append("")
    
    # Define indices
    output_lines.append("// Essential sprite tile indices")
    output_lines.append("")
    for name, idx in essential_map.items():
        output_lines.append(f"#define {name} {idx}")
    
    output_lines.append("")
    output_lines.append("#endif // SPRITES_COMPACT_H")
    
    output_file = os.path.join(os.path.dirname(__file__), 'data', 'sprites_compact.h')
    with open(output_file, 'w') as f:
        f.write("\n".join(output_lines))
    
    print(f"Compact defines written to {output_file}")

if __name__ == "__main__":
    main()