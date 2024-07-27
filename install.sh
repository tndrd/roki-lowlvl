#!/bin/bash
set -e

echo "roki-lowlvl: Libraries for communication with Roki's hardware"
echo "Author: Lekhterev V.V. @tndrd, Starkit 2024"

echo "Installing dependencies..."

sudo apt install git
sudo apt install cmake
sudo apt install python3 python3-dev python3-distutils

echo "Updating submodules..."
git submodule update --init --remote --recursive

echo "Building project..."
mkdir -p build
cd build

cmake ..
cmake --build .

echo "Installing..."

cmake --install .

cd TestInstall
sh run.sh

cd ..
echo "Installation successful"