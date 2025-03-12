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

sudo cmake --install .

if [ -z ${ROKI_PARAMS_SET+x} ]; then 
  echo "Roki parameters not yet set, setting with:"
  cat paramscmd.txt
  cat paramscmd.txt >> ~/.bashrc
else 
  echo "Roki parameters already installed, skipping .bashrc modification"
fi

cd ../TestInstall
sh run.sh

cd ..
echo "Installation successful"