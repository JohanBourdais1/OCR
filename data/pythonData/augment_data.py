#!/usr/bin/env python3
"""
Enhanced Data Augmentation for Sudoku Digit Recognition
Applique des transformations avancées pour améliorer la robustesse
"""

from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageOps
import numpy as np
import random
import math
import os

def add_elastic_deformation(image, alpha=30, sigma=3):
    """Apply elastic deformation to image (like handwriting variations)"""
    image_array = np.array(image)
    shape = image_array.shape
    
    # Generate random displacement fields
    dx = np.random.randn(*shape) * sigma
    dy = np.random.randn(*shape) * sigma
    
    # Smooth the displacement
    from scipy.ndimage import gaussian_filter
    dx = gaussian_filter(dx, sigma=sigma) * alpha
    dy = gaussian_filter(dy, sigma=sigma) * alpha
    
    # Apply displacement
    x, y = np.meshgrid(np.arange(shape[1]), np.arange(shape[0]))
    x_displaced = (x + dx).astype(np.float32)
    y_displaced = (y + dy).astype(np.float32)
    
    # Clip to valid range
    x_displaced = np.clip(x_displaced, 0, shape[1]-1)
    y_displaced = np.clip(y_displaced, 0, shape[0]-1)
    
    # Interpolate
    from scipy.ndimage import map_coordinates
    coords = np.array([y_displaced, x_displaced])
    deformed = map_coordinates(image_array, coords, order=1, mode='constant', cval=0)
    
    return Image.fromarray(deformed.astype(np.uint8))

def add_rotation(image, angle=None):
    """Apply random rotation (-15 to +15 degrees)"""
    if angle is None:
        angle = random.uniform(-15, 15)
    return image.rotate(angle, fillcolor=0, resample=Image.BICUBIC)

def add_perspective(image):
    """Apply slight perspective/affine transformation"""
    width, height = image.size
    
    # Small random shifts at corners for perspective
    shift = random.uniform(-2, 2)
    coeffs = [
        1 + random.uniform(-0.05, 0.05), random.uniform(-0.05, 0.05), random.uniform(-2, 2),
        random.uniform(-0.05, 0.05), 1 + random.uniform(-0.05, 0.05), random.uniform(-2, 2)
    ]
    
    return image.transform(image.size, Image.AFFINE, coeffs, Image.BICUBIC)

def add_gaussian_noise(image, sigma=10):
    """Add Gaussian noise"""
    image_array = np.array(image, dtype=np.float32)
    noise = np.random.normal(0, sigma, image_array.shape)
    noisy = np.clip(image_array + noise, 0, 255)
    return Image.fromarray(noisy.astype(np.uint8))

def adjust_contrast(image, factor=None):
    """Adjust image contrast"""
    if factor is None:
        factor = random.uniform(0.7, 1.3)
    
    enhancer = ImageOps.autocontrast(image)
    # Manual contrast adjustment
    image_array = np.array(image, dtype=np.float32)
    mean = np.mean(image_array)
    image_array = (image_array - mean) * factor + mean
    image_array = np.clip(image_array, 0, 255)
    return Image.fromarray(image_array.astype(np.uint8))

def adjust_brightness(image, factor=None):
    """Adjust image brightness"""
    if factor is None:
        factor = random.uniform(0.8, 1.2)
    
    image_array = np.array(image, dtype=np.float32)
    image_array = image_array * factor
    image_array = np.clip(image_array, 0, 255)
    return Image.fromarray(image_array.astype(np.uint8))

def add_stroke_variation(image, thickness_var=0.1):
    """Simulate stroke thickness variation (emulate different pen pressures)"""
    # This is done at generation time, not post-processing
    # But we can simulate by blurring/unblurring slightly
    image_array = np.array(image, dtype=np.float32)
    
    # Slight morphological operations
    if random.random() > 0.5:
        # Dilate slightly
        from scipy.ndimage import binary_dilation
        binary = image_array > 128
        dilated = binary_dilation(binary, iterations=1).astype(np.float32) * 255
        image_array = dilated
    
    return Image.fromarray(image_array.astype(np.uint8))

def augment_image(image, augmentation_level=3):
    """
    Apply multiple augmentations to an image
    
    augmentation_level:
        1: Light (rotation only)
        2: Medium (rotation + noise)
        3: Heavy (rotation + noise + perspective + contrast + brightness)
    """
    
    if augmentation_level >= 1 and random.random() > 0.5:
        image = add_rotation(image)
    
    if augmentation_level >= 2 and random.random() > 0.7:
        image = add_gaussian_noise(image, sigma=random.uniform(5, 15))
    
    if augmentation_level >= 3:
        if random.random() > 0.7:
            image = add_perspective(image)
        if random.random() > 0.6:
            image = adjust_contrast(image)
        if random.random() > 0.6:
            image = adjust_brightness(image)
    
    return image

def generate_augmented_data(base_images_dir, output_dir, augmentations_per_image=5):
    """
    Generate augmented versions of existing images
    
    Usage: Call this on your existing training dataset to expand it
    """
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    augmented_count = 0
    
    for digit_dir in os.listdir(base_images_dir):
        digit_path = os.path.join(base_images_dir, digit_dir)
        
        if not os.path.isdir(digit_path):
            continue
        
        output_digit_dir = os.path.join(output_dir, digit_dir)
        if not os.path.exists(output_digit_dir):
            os.makedirs(output_digit_dir)
        
        print(f"Processing digit {digit_dir}...")
        
        for image_file in os.listdir(digit_path):
            if not image_file.endswith(('.png', '.jpg', '.bmp')):
                continue
            
            image_path = os.path.join(digit_path, image_file)
            image = Image.open(image_path).convert('L')
            
            # Save original
            image.save(os.path.join(output_digit_dir, image_file))
            
            # Generate augmentations
            for aug_idx in range(augmentations_per_image):
                augmented = augment_image(image.copy(), augmentation_level=3)
                
                # Save with augmentation index
                base_name = os.path.splitext(image_file)[0]
                output_file = f"{base_name}_aug{aug_idx}.png"
                augmented.save(os.path.join(output_digit_dir, output_file))
                
                augmented_count += 1
                
                if augmented_count % 100 == 0:
                    print(f"  Generated {augmented_count} augmented images...")
    
    print(f"✓ Total augmented images generated: {augmented_count}")
    print(f"  Approximate expansion: {1 + augmentations_per_image}x")

if __name__ == "__main__":
    # Example usage - expand existing training data
    base_dir = "train/"
    output_dir = "train_augmented/"
    
    if os.path.exists(base_dir):
        generate_augmented_data(base_dir, output_dir, augmentations_per_image=4)
        print(f"\n✓ Augmented data saved to {output_dir}")
        print("You can use this as your new training directory")
    else:
        print(f"Error: {base_dir} not found")
        print("First generate training data with generate_balanced_data.py")
