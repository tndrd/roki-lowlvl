#!/bin/bash
set -e

echo "\nVerifying installation...\n"

mkdir -p build/
rm -rf build/*

cd build

echo "Finding CXX library..."
cmake ..

echo "Building against CXX library..."
cmake --build .
./TestInstall

cd ..

echo "Testing python module..."
python3 TestInstall.py

echo "Testing mbctl..."
mbctl help > /dev/null

echo "Testing mbdiag..."
mbdiag dummy > /dev/null

echo "\nInstallation verified successfully\n"