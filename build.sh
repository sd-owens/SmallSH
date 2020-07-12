#!/bin/bash
echo "Compiling smallsh..."
gcc -std=c99 -g smallsh.c -o smallsh
echo "Compile complete"
./smallsh