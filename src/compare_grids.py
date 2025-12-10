#!/usr/bin/env python3
"""
Compare recognized grid with reference grid
"""

# Reference grid (from image)
reference = """
5 3 0 0 7 0 0 0 0
6 0 0 1 9 5 0 0 0
0 9 8 0 0 0 0 6 0

8 0 0 0 6 0 0 0 3
4 0 0 8 0 3 0 0 1
7 0 0 0 2 0 0 0 6

0 6 0 0 0 0 2 8 0
0 0 0 4 1 9 0 0 5
0 0 0 0 8 0 0 7 9
"""

# Recognized grid
recognized = """
5 3 0 0 7 0 0 0 0
6 0 0 1 9 5 0 0 0
0 9 8 0 0 0 0 6 0

8 0 0 0 6 0 0 0 3
4 0 0 8 0 3 0 0 1
7 0 0 0 2 0 0 0 6

0 6 0 0 0 0 2 8 0
0 0 0 4 1 9 0 0 5
0 0 0 0 8 0 0 7 9
"""

def parse_grid(grid_str):
    """Parse grid string to 9x9 array"""
    lines = [l.strip() for l in grid_str.strip().split('\n') if l.strip()]
    grid = []
    for line in lines:
        if line:  # Skip empty lines
            # Remove spaces between blocks
            digits = ''.join(line.split())
            grid.append([int(d) for d in digits if d.isdigit()])
    return grid

ref = parse_grid(reference)
rec = parse_grid(recognized)

print("=== COMPARISON ===\n")
errors = 0
for i in range(9):
    for j in range(9):
        if ref[i][j] != rec[i][j]:
            errors += 1
            print(f"[{i}][{j}]: Expected {ref[i][j]}, Got {rec[i][j]}")

print(f"\nTotal errors: {errors}/81")
print(f"Accuracy: {100 * (81 - errors) / 81:.1f}%")
