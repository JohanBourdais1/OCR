#!/usr/bin/env python3
"""
Generate balanced training dataset: 5000 images per digit (0-9)
Total: 50,000 images
White digits on black background, 28x28 grayscale
With variations: different fonts, sizes, positions, brightness
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
    
    # Font paths - ALL fonts (bold + regular/light) for maximum variety
    font_paths = [
        # DejaVu family
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Oblique.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
        # Liberation family
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Bold.ttf",
        # FreeFont family
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerif.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf",
        # Ubuntu family
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-L.ttf",
        # Noto family
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Light.ttf",
        "/usr/share/fonts/truetype/noto/NotoSerif-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSerif-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoMono-Regular.ttf",
    ]
    
    available_fonts = [f for f in font_paths if os.path.exists(f)]
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
                # Random variations for each image
                # VERY small size to match thin printed sudoku digits (target: 50-75 non-zero pixels)
                base_size = 28  # Same as final size - no scaling
                font_size = random.randint(14, 18)   # Very small font for thin strokes
                
                # Create image directly at 28x28 with black background
                img = Image.new("L", (base_size, base_size), color=0)  # Black = 0
                
                # Draw white digit
                draw = ImageDraw.Draw(img)
                
                # Prefer non-bold fonts for thinner strokes
                selected_font = random.choice(available_fonts)
                
                try:
                    if selected_font:
                        font = ImageFont.truetype(selected_font, font_size)
                    else:
                        font = ImageFont.load_default()
                except:
                    font = ImageFont.load_default()
                
                # Center the digit with LARGER random offset for position variation
                text = str(digit)
                bbox = draw.textbbox((0, 0), text, font=font)
                text_width = bbox[2] - bbox[0]
                text_height = bbox[3] - bbox[1]
                
                # Add small random offset for position variation
                offset_x = random.randint(-3, 3)
                offset_y = random.randint(-3, 3)
                
                # Also add slight rotation simulation by varying position more
                x = (base_size - text_width) // 2 + offset_x
                y = (base_size - text_height) // 2 + offset_y
                
                # Ensure text stays within bounds
                x = max(0, min(x, base_size - text_width))
                y = max(0, min(y, base_size - text_height))
                
                # Draw white text (255 = white)
                draw.text((x, y), text, fill=255, font=font)
                
                # No resize needed - already at 28x28
            
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
