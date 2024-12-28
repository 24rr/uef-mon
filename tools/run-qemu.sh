#!/bin/bash

# Exit on error
set -e

# Check if OVMF_PATH is set
if [ -z "$OVMF_PATH" ]; then
    echo "Error: OVMF_PATH environment variable is not set"
    echo "Please set OVMF_PATH to point to your OVMF firmware directory"
    exit 1
fi

# Create a test disk image if it doesn't exist
if [ ! -f test_disk.img ]; then
    dd if=/dev/zero of=test_disk.img bs=1M count=64
    mkfs.fat -F 32 test_disk.img
fi

# Copy the UEFI application to the disk image
mkdir -p mnt
sudo mount test_disk.img mnt
sudo cp ../build/UefMon.efi mnt/
sudo umount mnt
rmdir mnt

# Run QEMU with OVMF
qemu-system-x86_64 \
    -cpu qemu64 \
    -m 256 \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_PATH/OVMF_CODE.fd" \
    -drive if=pflash,format=raw,file="$OVMF_PATH/OVMF_VARS.fd" \
    -drive format=raw,file=test_disk.img \
    -net none 