# T-HMI-Atari800

Atari 800 XL emulator for the Lilygo T-HMI (ESP32-S3) board.

## Overview

T-HMI-Atari800 is an Atari 800 XL computer emulator designed to run on ESP32-S3 based development boards with LCD displays. It emulates the complete Atari 8-bit hardware including:

- **6502 CPU** - Full instruction set including undocumented opcodes
- **ANTIC** - Display list processor with all 14 graphics modes
- **GTIA** - Graphics/color controller with player/missile graphics
- **POKEY** - 4-channel audio and keyboard/I/O controller
- **PIA** - Parallel I/O adapter for joystick input

## Supported Hardware

| Board | Display | Audio | SD Card |
|-------|---------|-------|---------|
| Lilygo T-HMI | 320x240 TFT | No | Yes |
| Lilygo T-Display S3 | 536x240 AMOLED | No | No |
| Waveshare ESP32-S3-LCD-2.8 | 320x240 TFT | Yes (I2S) | Yes |

## Features

- 64KB RAM (Atari 800 XL configuration)
- PAL timing (50Hz display, 312 scanlines)
- Hardware-accelerated display output
- 4-channel POKEY audio synthesis
- Web-based keyboard input over WiFi
- BLE keyboard support (with companion Android app)
- Joystick support via analog GPIO
- SD card support for loading .XEX/.ATR files

## Building

### Prerequisites

1. Clone the repository:
```bash
git clone https://github.com/petermartis/T-HMI-Atari800.git
cd T-HMI-Atari800
```

2. Install Arduino CLI:
```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

3. Install ESP32 Arduino core:
```bash
make install-core
```

### Compile

```bash
# For Lilygo T-HMI with web keyboard
make BOARD=T_HMI KEYBOARD=WEB_KEYBOARD

# For Waveshare with BLE keyboard
make BOARD=WAVESHARE KEYBOARD=BLE_KEYBOARD
```

### Upload

```bash
make upload PORT=/dev/ttyACM0
```

## Testing

The project includes a comprehensive test suite covering CPU emulation, memory system, and display processor.

### Running Tests

```bash
# Run all tests
make test

# Run specific test categories
make test-cpu      # CPU 6502 emulation tests
make test-memory   # Memory banking and address decoding
make test-antic    # ANTIC display processor tests

# Run with verbose output
make test-verbose
```

### Test Coverage

- **229 assertions** across **29 test cases**
- CPU register and flag operations
- Memory mapping and banking
- ANTIC display modes and configuration
- Stack, zero page, and addressing modes

Tests run automatically on every push via GitHub Actions CI/CD.

For more details, see [`test/README.md`](test/README.md).

## ROM Files

The emulator includes a minimal OS ROM for testing. For full compatibility with Atari software, you should use proper ROM files:

### Recommended: Altirra OS/BASIC (Free)

The Altirra OS and BASIC are high-quality replacements that are freely redistributable with emulators. Download from:
- https://www.virtualdub.org/altirra.html

### Original ROMs

Alternatively, you can use original Atari ROMs:
- **OS ROM**: atarixl.rom (16KB, Rev 2 recommended)
- **BASIC ROM**: atbasic.rom (8KB, Rev C recommended)

Place ROM files in the `src/roms/` directory and rebuild.

## Input Methods

### Web Keyboard (Default)

1. On first boot, the device starts as a WiFi access point
2. Connect to the "T-HMI-Atari800" network
3. Navigate to http://192.168.4.1
4. Enter your home WiFi credentials
5. On subsequent boots, access the keyboard at the device's IP address

### BLE Keyboard

1. Install the companion Android app from the `android/` directory
2. Enable Bluetooth on your phone
3. Pair with the device
4. Use the on-screen keyboard in the app

### Physical Joystick

Connect an analog joystick shield:
- X-axis: GPIO15 (ADC)
- Y-axis: GPIO16 (ADC)
- Fire button: GPIO18
- Secondary button: GPIO17

## Console Keys

The Atari console keys are mapped to:
- **START**: Joystick fire + up
- **SELECT**: Joystick fire + down
- **OPTION**: Joystick fire + left
- **RESET**: Joystick fire + right (long press)

## Technical Details

### Memory Map

```
$0000-$3FFF  RAM (16KB)
$4000-$7FFF  RAM (16KB)
$8000-$9FFF  RAM/Cartridge
$A000-$BFFF  BASIC ROM / RAM
$C000-$CFFF  OS ROM
$D000-$D0FF  GTIA registers
$D200-$D2FF  POKEY registers
$D300-$D3FF  PIA registers
$D400-$D4FF  ANTIC registers
$D800-$FFFF  OS ROM
```

### Display

- Resolution: 320x192 pixels (standard playfield)
- Color depth: 256 colors (GTIA palette)
- Refresh rate: 50Hz (PAL)
- LCD output: RGB565

### Audio

- 4 audio channels
- Frequency range: ~30Hz to ~30kHz
- Distortion modes: Pure tone, 4-bit poly, 5-bit poly, 9-bit poly, 17-bit poly
- Sample rate: 44.1kHz

## Credits

- Based on the T-HMI-C64 emulator architecture by retroelec
- Atari 800 hardware documentation from the Atari community
- Altirra emulator and documentation by Avery Lee

## License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

See [LICENSE](LICENSE) for details.

## References

- [Atari 800 Technical Reference](https://www.atariarchives.org/dev/)
- [ANTIC, GTIA, POKEY documentation](https://www.atariarchives.org/mapping/)
- [Altirra Hardware Reference Manual](https://www.virtualdub.org/altirra.html)
- [T-HMI-C64 Project](https://github.com/retroelec/T-HMI-C64)
