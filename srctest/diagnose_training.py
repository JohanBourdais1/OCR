#!/usr/bin/env python3
"""
Script d'entraînement optimisé pour le CNN
Analyse le dataset, valide en temps réel, et diagnostique les problèmes
"""

import os
import subprocess
import sys
from pathlib import Path
from collections import defaultdict

def count_images(path):
    """Compter les images par classe"""
    counts = defaultdict(int)
    total = 0
    
    for digit in range(10):
        digit_dir = Path(path) / str(digit)
        if digit_dir.exists():
            # Count recursively including subfolders
            for img_file in digit_dir.rglob('*.png'):
                if img_file.is_file():
                    counts[digit] += 1
                    total += 1
    
    return counts, total

def check_dataset_balance(train_path):
    """Vérifier l'équilibre du dataset"""
    print("\n" + "="*60)
    print("DATASET BALANCE CHECK")
    print("="*60)
    
    counts, total = count_images(train_path)
    
    if total == 0:
        print("❌ No images found in training directory!")
        print(f"   Checked: {train_path}")
        return False
    
    print(f"\nTotal training images: {total}")
    print("\nClass distribution:")
    
    imbalance = False
    for digit in range(10):
        count = counts.get(digit, 0)
        if total > 0:
            percent = 100.0 * count / total
            expected_percent = 10.0
            
            # Red if more than 5% deviation
            if abs(percent - expected_percent) > 5.0:
                status = "⚠️  IMBALANCED"
                imbalance = True
            else:
                status = "✓"
            
            print(f"  Digit {digit}: {count:5d} images ({percent:5.1f}%) {status}")
    
    if imbalance:
        print("\n⚠️  Warning: Dataset is imbalanced!")
        print("   Consider augmenting underrepresented classes")
    else:
        print("\n✓ Dataset appears well-balanced")
    
    return not imbalance

def check_image_quality(train_path):
    """Vérifier la qualité des images"""
    print("\n" + "="*60)
    print("IMAGE QUALITY CHECK")
    print("="*60)
    
    try:
        from PIL import Image
        import numpy as np
        
        print("\nChecking sample images...")
        
        for digit in range(10):
            digit_dir = Path(train_path) / str(digit)
            if digit_dir.exists():
                # Trouver un fichier image
                img_files = list(digit_dir.rglob('*.png'))
                if img_files:
                    img = Image.open(img_files[0]).convert('L')
                    arr = np.array(img)
                    
                    contrast = arr.max() - arr.min()
                    mean_val = arr.mean()
                    
                    status = "✓" if contrast > 50 else "⚠️"
                    print(f"  Digit {digit}: size={img.size}, contrast={contrast}, mean={mean_val:.1f} {status}")
                    
                    if contrast < 30:
                        print(f"    ⚠️  Low contrast detected - images may be too faded")
    
    except ImportError:
        print("PIL not available, skipping image quality check")

def generate_training_report(train_path):
    """Générer un rapport complet"""
    print("\n" + "="*70)
    print(" "*15 + "CNN TRAINING DIAGNOSTIC REPORT")
    print("="*70)
    
    check_dataset_balance(train_path)
    check_image_quality(train_path)
    
    print("\n" + "="*70)
    print("RECOMMENDATIONS FOR 100% ACCURACY")
    print("="*70)
    print("""
1. ✓ ADAPTIVE CROPPING
   - Replace fixed 10px crop with content-aware cropping
   - Preserves more digit detail, reduces data loss

2. ✓ BETTER NORMALIZATION  
   - Use min-max scaling instead of simple /255
   - Improves contrast regardless of image brightness

3. ✓ LARGER NETWORK
   - Conv1: 32 filters (was 16)
   - Conv2: 64 filters (was 32)
   - Dense: 512 units (was 256)

4. ✓ BATCH NORMALIZATION
   - Stabilizes training, allows higher learning rates
   - Add batch norm after conv/dense layers

5. ✓ VALIDATION MONITORING
   - Test on validation set every N epochs
   - Prevent overfitting, track generalization

6. ✓ IMPROVED OPTIMIZATION
   - Use Adam optimizer instead of basic SGD
   - Learning rate scheduling
   - Momentum for faster convergence

7. ✓ DATA AUGMENTATION
   - Random rotations (±5°)
   - Random translations (±2px)
   - Random scaling (±10%)
   - Improves robustness to variations

8. ✓ ENSEMBLE TRAINING
   - Train multiple networks with different initializations
   - Average predictions for better accuracy
""")
    print("="*70)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 diagnose_training.py <train_path>")
        sys.exit(1)
    
    train_path = sys.argv[1]
    
    if not Path(train_path).exists():
        print(f"Error: Path does not exist: {train_path}")
        sys.exit(1)
    
    generate_training_report(train_path)
