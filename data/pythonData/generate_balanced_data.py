#!/usr/bin/env python3
"""
Generate balanced training dataset: 5000 images per digit (0-9)
Total: 50,000 images
White digits on black background, 28x28 grayscale
With variations: different fonts, sizes, positions, brightness
OPTIMIZED for printed sudoku digits (larger, thicker strokes)
"""

from PIL import Image, ImageDraw, ImageFont, ImageFilter
import os
import random
import uuid

def generate_balanced_dataset(images_per_digit=20000, output_folder="train/"):
    """Generate exactly the same number of images for each digit with variations
       Equally distributed across all available fonts"""
    
    # Create output folder
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    
    # Font paths - Regular and Bold only (no italic/oblique, no thin/light)
    font_paths = [
        # DejaVu Bold
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
        # DejaVu Regular
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        # Ubuntu Bold
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf",
        # Ubuntu Regular/Medium
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-M.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
        # Noto
        "/usr/share/fonts/truetype/noto/NotoSansMono-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoMono-Regular.ttf",
        # Liberation Bold
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Bold.ttf",
        # Liberation Regular
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf",
        # FreeFonts Bold
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf",
        # FreeFonts Regular
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerif.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
    ]
    
    available_fonts = [f for f in font_paths if os.path.exists(f)]
    
    if not available_fonts:
        print("Error: No TrueType fonts found!")
        return
    
    num_fonts = len(available_fonts)
    images_per_font = images_per_digit // num_fonts
    remainder = images_per_digit % num_fonts
    
    print(f"Found {num_fonts} fonts")
    print(f"Images per digit: {images_per_digit}")
    print(f"Images per font per digit: {images_per_font}")
    print(f"Remainder (distributed to first fonts): {remainder}")
    print(f"\nFonts used:")
    for i, f in enumerate(available_fonts):
        print(f"  {i+1}. {os.path.basename(f)}")
    
    total_generated = 0
    
    # Generate for each digit (0-9)
    for digit in range(10):
        print(f"\n=== Generating images for digit {digit} ===")
        
        digit_count = 0
        
        # Special case: digit 0 = empty cell (all black)
        # Only generate 1000 images (they're all identical anyway)
        if digit == 0:
            num_empty = 10000
            for i in range(num_empty):
                img = Image.new("L", (28, 28), color=0)  # Completely black
                unique_id = str(uuid.uuid4())[:8]
                filename = f"{digit}_{unique_id}.png"
                filepath = os.path.join(output_folder, filename)
                img.save(filepath)
                digit_count += 1
                total_generated += 1
            print(f"  Digit {digit} (empty): {digit_count} images generated")
        else:
            # For each font, generate equal number of images
            for font_idx, font_path in enumerate(available_fonts):
                # Add 1 extra image to first 'remainder' fonts
                count_for_this_font = images_per_font + (1 if font_idx < remainder else 0)
                
                try:
                    font_name = os.path.basename(font_path)
                    
                    # Define font sizes to distribute evenly
                    font_sizes = list(range(50, 76))  # 50 to 75 (26 sizes)
                    
                    for i in range(count_for_this_font):
                        # Create larger image then resize
                        scale = 4  # Increased scale for better quality
                        base_size = 28 * scale  # 112x112
                        
                        # Distribute font sizes evenly across all images for this font
                        font_size = font_sizes[i % len(font_sizes)]
                        
                        img = Image.new("L", (base_size, base_size), color=0)
                        draw = ImageDraw.Draw(img)
                        
                        font = ImageFont.truetype(font_path, font_size)
                        
                        text = str(digit)
                        bbox = draw.textbbox((0, 0), text, font=font)
                        text_width = bbox[2] - bbox[0]
                        text_height = bbox[3] - bbox[1]
                        
                        # Random offset for position variation
                        offset_x = random.randint(-6, 6)
                        offset_y = random.randint(-6, 6)
                        
                        x = (base_size - text_width) // 2 + offset_x
                        y = (base_size - text_height) // 2 + offset_y - bbox[1]
                        
                        x = max(2, min(x, base_size - text_width - 2))
                        y = max(2, min(y, base_size - text_height - 2))
                        
                        # Draw with slight thickening (5 positions: center + cross)
                        for dx, dy in [(0, 0), (-1, 0), (1, 0), (0, -1), (0, 1)]:
                            draw.text((x + dx, y + dy), text, fill=255, font=font)
                        
                        # Resize with LANCZOS for smoother result
                        img = img.resize((28, 28), Image.LANCZOS)
                        
                        # Threshold (80 = balanced thickness)
                        import numpy as np
                        arr = np.array(img)
                        arr = np.where(arr > 80, 255, 0).astype(np.uint8)
                        img = Image.fromarray(arr)
                        
                        unique_id = str(uuid.uuid4())[:8]
                        filename = f"{digit}_{unique_id}.png"
                        filepath = os.path.join(output_folder, filename)
                        img.save(filepath)
                        
                        digit_count += 1
                        total_generated += 1
                    
                except Exception as e:
                    print(f"  Error with font {font_name}: {e}")
                    continue
            
            print(f"  Digit {digit}: {digit_count} images generated")
    
    print(f"\n=== COMPLETE ===")
    print(f"Total images generated: {total_generated}")
    print(f"Output folder: {output_folder}")
    
    # Verify distribution
    print("\n=== Verification ===")
    for digit in range(10):
        count = len([f for f in os.listdir(output_folder) if f.startswith(f"{digit}_")])
        print(f"Digit {digit}: {count} images")


if __name__ == "__main__":
    print("Generating balanced training dataset...")
    print("Format: 28x28 grayscale PNG")
    print("Background: Black (0)")
    print("Digits: White (255)")
    print()
    
    # Generate 20,000 images per digit for better 6/9 distinction
    generate_balanced_dataset(images_per_digit=10000, output_folder="train/")
    
    print("\n✓ Dataset generation complete!")
