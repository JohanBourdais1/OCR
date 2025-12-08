''' script that switch white in black and black in white in a directory and sub directories '''
import os
from PIL import Image

def switch_color_in_image(image_path, output_path):
    """Switch black and white colors in a single image."""
    with Image.open(image_path) as img:
        # Convert image to 'L' mode (grayscale) if not already
        if img.mode != 'L':
            img = img.convert('L')
        
        # Invert colors: 255 - pixel value
        inverted_img = img.point(lambda p: 255 - p)
        
        # Save the modified image
        inverted_img.save(output_path)
def switch_color_in_directory(input_dir, output_dir):
    """Recursively switch colors in all images in the input directory and save to output directory."""
    for root, _, files in os.walk(input_dir):
        # Create corresponding output directory
        relative_path = os.path.relpath(root, input_dir)
        output_root = os.path.join(output_dir, relative_path)
        os.makedirs(output_root, exist_ok=True)
        
        for file in files:
            if file.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.tiff')):
                input_path = os.path.join(root, file)
                output_path = os.path.join(output_root, file)
                switch_color_in_image(input_path, output_path)
# Example usage:
switch_color_in_directory('data/', 'data/')