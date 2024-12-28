#!/bin/bash

# Exit on error
set -e

# Check if EDK2_PATH is set
if [ -z "$EDK2_PATH" ]; then
    echo "Error: EDK2_PATH environment variable is not set"
    echo "Please set EDK2_PATH to point to your EDK II installation"
    exit 1
fi

# Source EDK II build environment
source $EDK2_PATH/edksetup.sh

# Build for X64 architecture
echo "Building main application..."
build -p $EDK2_PATH/MdeModulePkg/MdeModulePkg.dsc -a X64 -t GCC5

# Build test application
echo "Building test application..."
build -p $EDK2_PATH/UnitTestFrameworkPkg/UnitTestFrameworkPkg.dsc -a X64 -t GCC5

# Create build directory and copy artifacts
mkdir -p ../build
cp Build/MdeModule/DEBUG_GCC5/X64/UefMon.efi ../build/
cp Build/UnitTest/DEBUG_GCC5/X64/UefMonTest.efi ../build/

echo "Build complete! Applications are in build/ directory:"
echo "- UefMon.efi (main application)"
echo "- UefMonTest.efi (test application)" 