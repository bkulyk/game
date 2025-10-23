from PIL import Image
import math
import sys
import argparse

# Configuration
TRANSPARENT_COLOR = (255, 0, 255)  # Magenta/pink as transparency
MAX_COLORS_PER_SPRITE = 16         # SMS/Genesis sprite color limit

GENESIS_PALETTE_RGB = [
  (int(round(r * 255.0 / 7.0)), int(round(g * 255.0 / 7.0)), int(round(b * 255.0 / 7.0)))
  for r in range(8) for g in range(8) for b in range(8)
]

def find_nearest_genesis_color(rgb):
    """Find the closest Genesis color (from GENESIS_PALETTE_RGB) to the given RGB"""
    r, g, b = rgb
    min_dist = float('inf')
    best_color = (0, 0, 0)

    for color in GENESIS_PALETTE_RGB:
        # use squared distance for speed
        dist = (r - color[0])**2 + (g - color[1])**2 + (b - color[2])**2
        if dist < min_dist:
            min_dist = dist
            best_color = color

    return best_color

def convert_to_indexed_sms(input_path, output_path):
    """Convert image to indexed PNG using Genesis palette snapping but include only colors actually used"""
    try:
        img = Image.open(input_path).convert("RGBA")
    except FileNotFoundError:
        print(f"Error: Input file '{input_path}' not found")
        sys.exit(1)

    width, height = img.size

    # First pass: map each pixel to a palette index (0 reserved for transparent).
    color_map = {}        # Genesis color tuple -> palette index (1..)
    next_index = 1
    pixel_indices = [0] * (width * height)

    for y in range(height):
        for x in range(width):
            idx = y * width + x
            r, g, b, a = img.getpixel((x, y))

            if a < 128:  # Transparent
                pixel_indices[idx] = 0
            else:
                snapped_color = find_nearest_genesis_color((r, g, b))
                if snapped_color not in color_map:
                    if next_index > 255:
                        # Fallback: map to nearest already-mapped color if we run out of palette indices
                        min_dist = float('inf')
                        best_idx = 1
                        for mapped_color, mapped_idx in color_map.items():
                            dist = ((snapped_color[0] - mapped_color[0])**2 +
                                    (snapped_color[1] - mapped_color[1])**2 +
                                    (snapped_color[2] - mapped_color[2])**2)
                            if dist < min_dist:
                                min_dist = dist
                                best_idx = mapped_idx
                        pixel_indices[idx] = best_idx
                        continue
                    color_map[snapped_color] = next_index
                    next_index += 1

                pixel_indices[idx] = color_map[snapped_color]

    # Build palette: transparent first, then only the used Genesis colors in insertion order
    palette = list(TRANSPARENT_COLOR)
    for color in color_map.keys():
        palette.extend(color)

    # Pad palette to 256 colors (768 values)
    while len(palette) < 256 * 3:
        palette.extend([0, 0, 0])

    # Create palette image and set data
    palette_img = Image.new("P", (width, height))
    palette_img.putpalette(palette)
    palette_img.putdata(pixel_indices)

    # Verify color count (exclude transparent)
    color_count = len(color_map)
    if color_count > MAX_COLORS_PER_SPRITE - 1:
        print(f"Warning: Uses {color_count} colors (sprite limit is {MAX_COLORS_PER_SPRITE - 1})")

    # Save as optimized indexed PNG
    palette_img.save(output_path, optimize=True)
    print(f"Successfully converted {input_path} to {output_path}")
    print(f"Used {color_count} colors + transparency")

def main():
    parser = argparse.ArgumentParser(
        description='Convert images to Sega Master System compatible indexed PNGs',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""Example:
  python color-snapper.py sprite.png sprite_sms.png
""")
    parser.add_argument('input', help='Input image file path')
    parser.add_argument('output', help='Output image file path')

    args = parser.parse_args()

    convert_to_indexed_sms(args.input, args.output)

if __name__ == "__main__":
    main()
