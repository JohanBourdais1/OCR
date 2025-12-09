from PIL import Image
from PIL import ImageFont
from PIL import ImageDraw

import random
import os
import uuid


def generate_digit_images(num_images=5000, output_folder="train/"):
    """Generates images containing a single digit with white text on black background"""
    
    # Create output folder if it doesn't exist
    os.makedirs(output_folder, exist_ok=True)
    
    random.seed(42)
    
    generated_count = 0
    
    for _ in range(num_images):
        # Random digit (0-9)
        digit = random.randint(0, 9)
        
        # Random size for the digit
        size = random.randint(100, 200)
        
        # Random position
        x = random.randint(5, 80)
        y = random.randint(5, 80)
        
        # Generate image with black background
        img = Image.new("L", (256, 256), color=0)  # Black background
        
        # Draw white digit using default font
        draw = ImageDraw.Draw(img)
        try:
            # Try to use a TrueType font if available
            font = ImageFont.load_default()
        except:
            # Fallback to default font
            font = ImageFont.load_default()
        
        # For better quality, use a larger default font size
        # We'll need to scale the digit size manually
        draw.text((x, y), str(digit), fill=255, font=font)
        
        # Resize to 28x28
        img = img.resize((28, 28), Image.BILINEAR)
        
        # Generate unique ID
        unique_id = str(uuid.uuid4())[:8]
        
        # Save with format: [digit]_randomID.png
        filename = f"{digit}_{unique_id}.png"
        filepath = os.path.join(output_folder, filename)
        img.save(filepath)
        
        generated_count += 1
        
        if generated_count % 500 == 0:
            print(f"Generated {generated_count} images...")
    
    print(f"Successfully generated {generated_count} images in {output_folder}")


if __name__ == "__main__":
    # Generate 5000 images (modify the number as needed)
    generate_digit_images(num_images=5000, output_folder="../mnist_png/train")