#!/bin/bash
set -e

echo "Verifying installation..."

mkdir -p build/
rm -rf build/*

cd build

echo "Finding CXX library..."
cmake ..

echo "Building against CXX library..."
cmake --build .
./TestInstall

echo "Testing python module..."
python3 TestInstall.py

cd ..

echo "Testing mbctl..."
mbctl > /dev/null

echo "Testing mbwtf..."
mbwtf dontrun > /dev/null

echo "Installation verified successfully"