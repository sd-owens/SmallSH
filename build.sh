#!/bin/bash
echo "Compiling smallsh"
gcc -std=c99 -g smallsh.c -o smallsh
echo "Compiling cd"
gcc -std=c99 -g cd.c -o cd
echo "Compiling complete"
./smallsh