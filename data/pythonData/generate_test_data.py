#!/usr/bin/env python3
"""
Generate test dataset organized by digit and font
Structure: test/{digit}/{font_name}/image.png
"""

from PIL import Image, ImageDraw, ImageFont
import os
import random
import numpy as np

def generate_test_dataset(images_per_font=50, output_folder="test/"):
    """Generate test images organized by digit and font"""
    
    # Remove existing test folder and recreate
    if os.path.exists(output_folder):
        import shutil
        shutil.rmtree(output_folder)
    
    # Font paths - Bold fonts for thicker strokes
    font_paths = [
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-M.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansMono-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerif.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
    ]
    
    available_fonts = [f for f in font_paths if os.path.exists(f)]
    
    if not available_fonts:
        print("Error: No TrueType fonts found!")
        return
    
    print(f"Found {len(available_fonts)} fonts")
    print(f"Images per font per digit: {images_per_font}")
    print(f"Structure: {output_folder}{{digit}}/{{font_name}}/")
    print()
    
    total_generated = 0
    
    # Generate for each digit (0-9)
    for digit in range(10):
        print(f"=== Generating images for digit {digit} ===")
        
        # Create digit folder
        digit_folder = os.path.join(output_folder, str(digit))
        os.makedirs(digit_folder, exist_ok=True)
        
        # Special case: digit 0 = empty cell (all black)
        if digit == 0:
            empty_folder = os.path.join(digit_folder, "empty")
            os.makedirs(empty_folder, exist_ok=True)
            
            for i in range(images_per_font):
                img = Image.new("L", (28, 28), color=0)
                filepath = os.path.join(empty_folder, f"0_{i:04d}.png")
                img.save(filepath)
                total_generated += 1
            
            print(f"  empty: {images_per_font} images")
            continue
        
        # For each font
        for font_path in available_fonts:
            font_name = os.path.basename(font_path).replace('.ttf', '')
            font_folder = os.path.join(digit_folder, font_name)
            os.makedirs(font_folder, exist_ok=True)
            
            try:
                for i in range(images_per_font):
                    # Create larger image then resize
                    scale = 4
                    base_size = 28 * scale  # 112x112
                    
                    # Vary font size
                    font_size = random.randint(70, 85)
                    
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
                    
                    # Resize with LANCZOS
                    img = img.resize((28, 28), Image.LANCZOS)
                    
                    # Threshold (80 = balanced thickness)
                    arr = np.array(img)
                    arr = np.where(arr > 80, 255, 0).astype(np.uint8)
                    img = Image.fromarray(arr)
                    
                    filepath = os.path.join(font_folder, f"{digit}_{i:04d}.png")
                    img.save(filepath)
                    total_generated += 1
                
            except Exception as e:
                print(f"  Error with font {font_name}: {e}")
                continue
        
        # Count fonts for this digit
        font_count = len([d for d in os.listdir(digit_folder) if os.path.isdir(os.path.join(digit_folder, d))])
        print(f"  {font_count} fonts, {images_per_font} images each")
    
    print(f"\n=== COMPLETE ===")
    print(f"Total images generated: {total_generated}")
    print(f"Output folder: {output_folder}")
    
    # Show structure
    print(f"\n=== Structure ===")
    for digit in range(10):
        digit_folder = os.path.join(output_folder, str(digit))
        if os.path.exists(digit_folder):
            fonts = sorted(os.listdir(digit_folder))
            print(f"  {digit}/: {len(fonts)} folders")
            if digit == 1:  # Show example for digit 1
                for font in fonts[:3]:
                    font_path = os.path.join(digit_folder, font)
                    count = len(os.listdir(font_path))
                    print(f"      {font}/: {count} images")
                if len(fonts) > 3:
                    print(f"      ... and {len(fonts) - 3} more")


if __name__ == "__main__":
    print("Generating test dataset...")
    print("Format: 28x28 grayscale PNG")
    print("Background: Black (0)")
    print("Digits: White (255)")
    print()
    
    # Generate 50 images per font per digit
    generate_test_dataset(images_per_font=50, output_folder="test/")
    
    print("\n✓ Test dataset generation complete!")
