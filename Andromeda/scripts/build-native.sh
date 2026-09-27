#!/bin/bash
set -e
echo "=== Compiling Andromeda Native UCI Binary ==="
mkdir -p ../build/native
cd ../src
make -j$(nproc)
mv andromeda ../build/native/
echo "Compiled native binary to ../build/native/andromeda"
