#!/usr/bin/env python3
"""
Diagnose recognition issues by comparing preprocessing pipeline
"""
from PIL import Image
import os
import sys

print("=== RECOGNITION DIAGNOSIS ===\n")

# Check some grid images
print("Grid images (108-109x108 px):")
grid_samples = ["grid/tile_00.png", "grid/tile_13.png", "grid/tile_44.png"]
for grid_file in grid_samples:
    if os.path.exists(grid_file):
        img = Image.open(grid_file).convert('L')
        pixels = list(img.getdata())
        w, h = img.size
        
        # Get center region stats
        center_start = (h//2 - 10) * w + (w//2 - 10)
        print(f"  {grid_file}: size={w}x{h}, corner={pixels[0]}, center~{pixels[center_start]}")

print("\nTraining images (28x28 px):")
train_samples = ["../data/pythonData/train/1_*.png", "../data/pythonData/train/4_*.png"]
for pattern in train_samples:
    files = []
    if "*" in pattern:
        import glob
        files = glob.glob(pattern)[:1]
    else:
        files = [pattern] if os.path.exists(pattern) else []
    
    for train_file in files:
        img = Image.open(train_file)
        pixels = list(img.getdata())
        print(f"  {os.path.basename(train_file)}: size=28x28, pixels: min={min(pixels)}, max={max(pixels)}")

print("\n=== CROP ANALYSIS ===")
print("Current crop: 10 pixels from all sides of ~108px image")
print("Resulting size: ~88x88, then resized to 28x28")
print("Scale factor: 28/88 ≈ 0.318x")
print("\nPossible issue: Aggressive crop + downsampling may lose digit details")
print("Recommendation: Try reducing crop to 5-8 pixels or adaptive crop")

print("\n=== THRESHOLD ANALYSIS ===")
print("Current threshold: 128")
print("Pixels < 128 → 0 (black), >= 128 → 255 (white)")
print("\nVerify that grid image centers have good contrast after inversion")

print("\n=== NEXT STEPS ===")
print("1. Check debug images in debug/*.pgm to verify preprocessing")
print("2. Try reducing crop padding (5-8 px instead of 10)")
print("3. Verify network training accuracy on training set")
print("4. Check for specific digit confusion patterns")
