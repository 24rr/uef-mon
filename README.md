# UEF-Mon (UEFI Variable Recovery Tool)

A lightweight UEFI runtime utility for monitoring, backing up, and recovering critical UEFI variables. This tool helps protect your system's firmware configuration by providing automated backup and recovery capabilities for UEFI variables.

## Features

- **Variable Monitoring**: Real-time monitoring of UEFI variable changes
- **Automated Backup**: Automatic backup of variables when changes are detected
- **Safe Recovery**: Restore UEFI variables from backups with validation checks
- **Interactive UI**: Simple menu-driven interface for all operations
- **Safety First**: Built-in validation and error checking to prevent corruption

## Use Cases

- Backup critical boot configuration before firmware updates
- Monitor secure boot variables for unauthorized changes
- Create restore points for UEFI configuration
- Recover from failed firmware updates or configuration changes

## Building

### Prerequisites
- EDK II (Tianocore)
- QEMU (for testing)
- OVMF (Open Virtual Machine Firmware)
- GCC or MSVC compiler

### Quick Start
```bash
# Clone the repository
git clone https://github.com/24rr/uef-mon.git
cd uef-mon

# Build the project
./tools/build.sh  # For Linux/macOS
./tools/build.bat # For Windows
```

### Testing
The project includes a comprehensive test suite that can be run safely without affecting your system's UEFI variables:

```bash
cd tests
./build.bat  # For Windows
make test    # For Linux/macOS
```

## Safety Features

- Read-only operations by default
- Validation before any write operations
- Backup verification before restoration
- Isolated test environment using QEMU/OVMF

## Project Structure
```
.
├── src/              # Source code
├── include/          # Header files
├── tests/            # Test suite
├── tools/            # Build and test scripts
└── edk2/            # EDK II configuration
```

## License

[MIT License](LICENSE)

## Contributing

Contributions are welcome! Please read our contributing guidelines before submitting pull requests.

## Disclaimer

This tool interacts with UEFI firmware variables. While it includes safety measures, use caution when modifying UEFI variables as incorrect modifications can affect system boot capability. Always test in a safe environment first. 