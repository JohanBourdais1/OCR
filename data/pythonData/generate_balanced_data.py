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

def generate_balanced_dataset(images_per_digit=5000, output_folder="train/"):
    """Generate exactly the same number of images for each digit with variations"""
    
    # Create output folder
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    
    # Font paths - Prefer BOLD fonts for thicker strokes like printed sudoku
    font_paths_bold = [
        # Bold fonts first (priority)
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Bold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoSerif-Bold.ttf",
    ]
    
    font_paths_regular = [
        # Regular fonts (less priority)
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerif.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSerif-Regular.ttf",
    ]
    
    available_fonts_bold = [f for f in font_paths_bold if os.path.exists(f)]
    available_fonts_regular = [f for f in font_paths_regular if os.path.exists(f)]
    
    # 70% bold fonts, 30% regular for variety
    available_fonts = available_fonts_bold * 3 + available_fonts_regular
    
    if not available_fonts:
        print("Warning: No TrueType fonts found, using default font")
        available_fonts = [None]
    
    total_generated = 0
    
    # Generate for each digit (0-9)
    for digit in range(10):
        print(f"\n=== Generating {images_per_digit} images for digit {digit} ===")
        
        for i in range(images_per_digit):
            # Special case: digit 0 = empty cell (all black)
            if digit == 0:
                img = Image.new("L", (28, 28), color=0)  # Completely black
            else:
                # Create larger image then resize for anti-aliasing effect
                scale = 3  # 3x scale for better quality
                base_size = 28 * scale  # 84x84 then resize to 28x28
                
                # LARGER font size for thicker strokes matching printed sudoku
                font_size = random.randint(50, 60)  # Very large for thick strokes like sudoku
                
                # Create image with black background
                img = Image.new("L", (base_size, base_size), color=0)  # Black = 0
                
                # Draw white digit
                draw = ImageDraw.Draw(img)
                
                # Select font (mostly bold)
                selected_font = random.choice(available_fonts)
                
                try:
                    if selected_font:
                        font = ImageFont.truetype(selected_font, font_size)
                    else:
                        font = ImageFont.load_default()
                except:
                    font = ImageFont.load_default()
                
                # Center the digit with small random offset
                text = str(digit)
                bbox = draw.textbbox((0, 0), text, font=font)
                text_width = bbox[2] - bbox[0]
                text_height = bbox[3] - bbox[1]
                
                # Small random offset for position variation
                offset_x = random.randint(-4, 4)
                offset_y = random.randint(-4, 4)
                
                x = (base_size - text_width) // 2 + offset_x
                y = (base_size - text_height) // 2 + offset_y - bbox[1]  # Adjust for baseline
                
                # Ensure text stays within bounds
                x = max(2, min(x, base_size - text_width - 2))
                y = max(2, min(y, base_size - text_height - 2))
                
                # Draw white text (255 = white)
                draw.text((x, y), text, fill=255, font=font)
                
                # Resize to 28x28 with anti-aliasing for smooth edges
                img = img.resize((28, 28), Image.LANCZOS)
                
                # Optional: slight brightness variation (85-100% of max)
                if random.random() < 0.3:
                    import numpy as np
                    arr = np.array(img)
                    factor = random.uniform(0.85, 1.0)
                    arr = (arr * factor).astype(np.uint8)
                    img = Image.fromarray(arr)
            
            # Generate unique ID
            unique_id = str(uuid.uuid4())[:8]
            
            # Save with format: [digit]_randomID.png
            filename = f"{digit}_{unique_id}.png"
            filepath = os.path.join(output_folder, filename)
            img.save(filepath)
            
            total_generated += 1
            
            if (i + 1) % 1000 == 0:
                print(f"  Digit {digit}: {i + 1}/{images_per_digit} images generated")
    
    print(f"\n=== COMPLETE ===")
    print(f"Total images generated: {total_generated}")
    print(f"Images per digit: {images_per_digit}")
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
    
    # Generate 10,000 images per digit
    generate_balanced_dataset(images_per_digit=10000, output_folder="train/")
    
    print("\n✓ Dataset generation complete!")
