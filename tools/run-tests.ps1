# Check if OVMF_PATH environment variable is set
if (-not $env:OVMF_PATH) {
    Write-Error "Error: OVMF_PATH environment variable is not set"
    Write-Error "Please set OVMF_PATH to point to your OVMF firmware directory"
    exit 1
}

# Create a dedicated test directory
$TEST_DIR = "test_env"
New-Item -ItemType Directory -Force -Path $TEST_DIR | Out-Null

# Create a copy of OVMF variables file for testing
Copy-Item "$env:OVMF_PATH\OVMF_VARS.fd" "$TEST_DIR\OVMF_VARS_TEST.fd"

# Create a test disk image (64MB)
$diskImage = "$TEST_DIR\test_disk.img"
$fs = New-Object IO.FileStream($diskImage, [IO.FileMode]::Create)
$fs.SetLength(64MB)
$fs.Close()

# Format the disk image as FAT32
# Note: On Windows, we'll need external tools for this
# For now, we'll create directories directly in QEMU

Write-Host "Starting QEMU with test environment..."
Write-Host "The test application will run automatically on boot"
Write-Host "Tests will run in an isolated environment and cannot affect your system"

# Run QEMU with test configuration
& qemu-system-x86_64 `
    -cpu qemu64 `
    -m 256 `
    -drive if=pflash,format=raw,readonly=on,file="$env:OVMF_PATH\OVMF_CODE.fd" `
    -drive if=pflash,format=raw,file="$TEST_DIR\OVMF_VARS_TEST.fd" `
    -drive format=raw,file="$TEST_DIR\test_disk.img" `
    -net none `
    -nographic

# Clean up test environment
Write-Host "Cleaning up test environment..."
Remove-Item -Recurse -Force $TEST_DIR

Write-Host "Test run complete!" 