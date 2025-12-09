#!/usr/bin/env python3
from PIL import Image
import sys

# Analyser une image de grille
grid_img = Image.open('grid/tile_73.png')  # Devrait être un 4
print(f"Grid image: {grid_img.size}, mode: {grid_img.mode}")
pixels = list(grid_img.getdata())
print(f"Grid Min: {min(min(p) if isinstance(p, tuple) else p for p in pixels)}")
print(f"Grid Max: {max(max(p) if isinstance(p, tuple) else p for p in pixels)}")
print(f"Grid sample (center): {pixels[54*108 + 54]}")  # Centre approximatif
print(f"Grid sample (corner): {pixels[0]}")

# Analyser une image d'entraînement
train_img = Image.open('../data/mnist_png/train/0_1.png')
print(f"\nTrain image: {train_img.size}, mode: {train_img.mode}")
train_pixels = list(train_img.getdata())
print(f"Train Min: {min(train_pixels)}")
print(f"Train Max: {max(train_pixels)}")
print(f"Train sample (center): {train_pixels[14*28 + 14]}")
print(f"Train sample (corner): {train_pixels[0]}")

# Convertir grid image en grayscale et voir
grid_gray = grid_img.convert('L')
grid_gray_pixels = list(grid_gray.getdata())
print(f"\nGrid as grayscale:")
print(f"  Min: {min(grid_gray_pixels)}, Max: {max(grid_gray_pixels)}")
print(f"  Center: {grid_gray_pixels[54*108 + 54]}")
print(f"  Corner: {grid_gray_pixels[0]}")

# Inverser les couleurs
grid_inverted = [255 - p for p in grid_gray_pixels]
print(f"\nGrid inverted:")
print(f"  Min: {min(grid_inverted)}, Max: {max(grid_inverted)}")
print(f"  Center: {grid_inverted[54*108 + 54]}")
print(f"  Corner: {grid_inverted[0]}")

print("\n=== CONCLUSION ===")
print(f"Training images: Background={train_pixels[0]}, Digit=~{max(train_pixels)}")
print(f"Grid images (original): Background={grid_gray_pixels[0]}, Digit=~{min(grid_gray_pixels)}")
print(f"Grid images (inverted): Background={grid_inverted[0]}, Digit=~{max(grid_inverted)}")
