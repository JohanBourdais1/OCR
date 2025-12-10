#!/bin/bash
# Script to run UI with ASAN leak detection disabled (GTK has too many false positives)
ASAN_OPTIONS=detect_leaks=0:halt_on_error=0 ./main --ui
