#!/bin/bash
set -e

echo "\nroki-lowlvl: Libraries for communication with Roki's hardware"
echo "Author:      Lekhterev V.V. @tndrd, Starkit 2024"

echo "\nInstalling dependencies...\n"

sudo apt install git
sudo apt install cmake
sudo apt install python3 python3-dev python3-distutils

echo "\nUpdating submodules...\n"
git submodule update --init --remote --recursive

echo "\nBuilding project...\n"
mkdir -p build
cd build

cmake ..
cmake --build .

echo "\nInstalling...\n"

cmake --install .

cd ../TestInstall
sh run.sh

cd ..
echo "Installation successful"