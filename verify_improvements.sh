#!/bin/bash
# 🚀 Verification Script for CNN Improvements
# Run this to verify everything is set up correctly

set -e

echo "════════════════════════════════════════════════════════"
echo "  🧠 CNN Improvements - Verification Checklist"
echo "════════════════════════════════════════════════════════"
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} Found: $1"
        return 0
    else
        echo -e "${RED}✗${NC} Missing: $1"
        return 1
    fi
}

check_dir() {
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} Found: $1"
        return 0
    else
        echo -e "${RED}✗${NC} Missing: $1"
        return 1
    fi
}

echo ""
echo "1️⃣  Checking Source Code Files..."
echo "─────────────────────────────────"

ALL_OK=true
check_file "src/network/digitreconizer/network.h" || ALL_OK=false
check_file "src/network/digitreconizer/network.c" || ALL_OK=false
check_file "src/main.c" || ALL_OK=false

echo ""
echo "2️⃣  Checking Documentation Files..."
echo "────────────────────────────────"

check_file "IMPROVEMENTS.md" || ALL_OK=false
check_file "TRAINING_GUIDE.md" || ALL_OK=false
check_file "EXECUTIVE_SUMMARY.md" || ALL_OK=false

echo ""
echo "3️⃣  Checking Python Utilities..."
echo "──────────────────────────────"

check_file "data/pythonData/augment_data.py" || ALL_OK=false
check_file "data/pythonData/analyze_dataset.py" || ALL_OK=false
check_file "data/pythonData/generate_balanced_data.py" || ALL_OK=false

echo ""
echo "4️⃣  Checking Data Directories..."
echo "──────────────────────────────"

check_dir "data/pythonData/train" || echo -e "${YELLOW}⚠${NC}  Need to generate training data"
check_dir "data/pythonData/test" || echo -e "${YELLOW}⚠${NC}  Need to generate test data"

echo ""
echo "5️⃣  Verifying Key Features in Code..."
echo "────────────────────────────────────"

# Check for batch norm
if grep -q "batch_norm_forward" src/network/digitreconizer/network.c; then
    echo -e "${GREEN}✓${NC} Batch Normalization implemented"
else
    echo -e "${RED}✗${NC} Batch Normalization NOT found"
    ALL_OK=false
fi

# Check for dropout
if grep -q "apply_dropout" src/network/digitreconizer/network.c; then
    echo -e "${GREEN}✓${NC} Dropout implemented"
else
    echo -e "${RED}✗${NC} Dropout NOT found"
    ALL_OK=false
fi

# Check for learning rate scheduler
if grep -q "get_learning_rate" src/network/digitreconizer/network.c; then
    echo -e "${GREEN}✓${NC} Learning rate scheduler implemented"
else
    echo -e "${RED}✗${NC} Learning rate scheduler NOT found"
    ALL_OK=false
fi

# Check for mini-batch
if grep -q "BATCH_SIZE" src/network/digitreconizer/network.h; then
    echo -e "${GREEN}✓${NC} Mini-batch training configured"
else
    echo -e "${RED}✗${NC} Mini-batch training NOT found"
    ALL_OK=false
fi

# Check architecture size
if grep -q "NB_FILTER_1 32" src/network/digitreconizer/network.h; then
    echo -e "${GREEN}✓${NC} Increased architecture (32 filters Conv1)"
else
    echo -e "${YELLOW}⚠${NC}  Architecture might not be updated"
fi

if grep -q "HIDDEN_SIZE 512" src/network/digitreconizer/network.h; then
    echo -e "${GREEN}✓${NC} Increased hidden layer (512 neurons)"
else
    echo -e "${YELLOW}⚠${NC}  Hidden layer might not be updated"
fi

echo ""
echo "6️⃣  Attempting Compilation..."
echo "───────────────────────────"

if [ -f "src/Makefile" ]; then
    cd src
    if make --version > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} Make is available"
        
        # Try to compile
        if make clean > /dev/null 2>&1; then
            echo -e "${GREEN}✓${NC} Make clean succeeded"
        else
            echo -e "${YELLOW}⚠${NC}  Make clean had warnings"
        fi
        
        if make > /dev/null 2>&1; then
            echo -e "${GREEN}✓${NC} Compilation successful!"
            
            if [ -f "main" ]; then
                echo -e "${GREEN}✓${NC} Binary 'main' created"
            else
                echo -e "${RED}✗${NC} Binary 'main' not found"
                ALL_OK=false
            fi
        else
            echo -e "${RED}✗${NC} Compilation FAILED"
            echo ""
            echo "Trying again with more detail:"
            make 2>&1 | head -20
            ALL_OK=false
        fi
    else
        echo -e "${RED}✗${NC} Make not available"
        ALL_OK=false
    fi
    cd ..
else
    echo -e "${RED}✗${NC} Makefile not found in src/"
    ALL_OK=false
fi

echo ""
echo "════════════════════════════════════════════════════════"

if [ "$ALL_OK" = true ]; then
    echo -e "${GREEN}✅ All checks passed! Ready to train.${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. cd src"
    echo "  2. ./main --save     # Train the model"
    echo ""
    echo "Or run the full pipeline:"
    echo "  1. Generate data: python3 data/pythonData/generate_balanced_data.py"
    echo "  2. Augment data:  python3 data/pythonData/augment_data.py"
    echo "  3. Train model:   cd src && ./main --save"
    echo "  4. Test model:    ./main --load"
    echo "  5. Use UI:        ./main --ui"
else
    echo -e "${RED}❌ Some checks failed. Please review above.${NC}"
    echo ""
    echo "Common issues:"
    echo "  - Code not updated: Make sure IMPROVEMENTS.md changes are applied"
    echo "  - Missing libraries: Install SDL2, GTK3, etc."
    echo "  - Compilation errors: Check network.h and network.c syntax"
    exit 1
fi

echo ""
echo "════════════════════════════════════════════════════════"
echo "Generated: $(date)"
echo "════════════════════════════════════════════════════════"
