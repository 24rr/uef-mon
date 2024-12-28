#!/bin/bash

# Exit on error
set -e

# Check if OVMF_PATH is set
if [ -z "$OVMF_PATH" ]; then
    echo "Error: OVMF_PATH environment variable is not set"
    echo "Please set OVMF_PATH to point to your OVMF firmware directory"
    exit 1
fi

# Create a dedicated test directory
TEST_DIR="test_env"
mkdir -p "$TEST_DIR"

# Create a copy of OVMF variables file for testing
cp "$OVMF_PATH/OVMF_VARS.fd" "$TEST_DIR/OVMF_VARS_TEST.fd"

# Create a test disk image
dd if=/dev/zero of="$TEST_DIR/test_disk.img" bs=1M count=64
mkfs.fat -F 32 "$TEST_DIR/test_disk.img"

# Mount the disk image and copy test files
mkdir -p "$TEST_DIR/mnt"
sudo mount "$TEST_DIR/test_disk.img" "$TEST_DIR/mnt"

# Create necessary directories
sudo mkdir -p "$TEST_DIR/mnt/EFI/Boot"
sudo mkdir -p "$TEST_DIR/mnt/EFI/UefMon/backup"

# Copy the test application as the bootable application
sudo cp ../build/UefMonTest.efi "$TEST_DIR/mnt/EFI/Boot/bootx64.efi"

# Unmount the disk
sudo umount "$TEST_DIR/mnt"
rmdir "$TEST_DIR/mnt"

echo "Starting QEMU with test environment..."
echo "The test application will run automatically on boot"
echo "Tests will run in an isolated environment and cannot affect your system"

# Run QEMU with test configuration
qemu-system-x86_64 \
    -cpu qemu64 \
    -m 256 \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_PATH/OVMF_CODE.fd" \
    -drive if=pflash,format=raw,file="$TEST_DIR/OVMF_VARS_TEST.fd" \
    -drive format=raw,file="$TEST_DIR/test_disk.img" \
    -net none \
    -nographic

# Clean up test environment
echo "Cleaning up test environment..."
rm -rf "$TEST_DIR"

echo "Test run complete!" 