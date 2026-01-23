# T-HMI-Atari800 Test Suite

Comprehensive testing infrastructure for the Atari 800 XL emulator.

## Overview

This test suite provides automated testing for the T-HMI-Atari800 emulator, covering:
- CPU 6502 emulation
- Memory system and banking
- ANTIC display processor
- System integration

## Quick Start

### Running Tests

```bash
# Run all tests
make test

# Run specific test categories
make test-cpu      # CPU emulation tests
make test-memory   # Memory system tests
make test-antic    # ANTIC display tests

# Run with verbose output
make test-verbose

# Clean test build
make test-clean
```

## Test Coverage

### Current Status

```
Total Test Cases: 29
Total Assertions: 229
Test Coverage:    Basic (foundational tests implemented)
```

### Test Breakdown

#### 1. CPU6502 Tests (`test_cpu6502.cpp`)
- **Initial State Tests**: Verify CPU initializes correctly
- **Register Tests**: Test all CPU registers (A, X, Y, SP, PC)
- **Flag Tests**: Validate all status flags (C, Z, I, D, V, N)
- **Memory Interface**: Test read/write operations
- **Stack Operations**: Stack pointer and stack memory
- **Reset Behavior**: CPU reset functionality
- **Addressing Modes**: Calculate effective addresses

**Coverage**: 13 test cases, 89 assertions

#### 2. Memory System Tests (`test_memory.cpp`)
- **Memory Map**: Validate address ranges
- **Address Decoder**: Test memory region categorization
- **Banking Simulation**: OS ROM and BASIC ROM banking
- **Stack Area**: Stack page (page 1) behavior
- **Zero Page**: Zero page addressing and wraparound
- **Page Crossing**: Page boundary detection
- **Endianness**: Little-endian multi-byte values

**Coverage**: 10 test cases, 79 assertions

#### 3. ANTIC Tests (`test_antic.cpp`)
- **Register Addresses**: ANTIC register mapping
- **Display Modes**: All 16 ANTIC display modes
- **Display List**: Instruction bit fields and flags
- **Screen Dimensions**: Resolution constants
- **DMACTL Register**: DMA control configurations
- **NMI Sources**: Interrupt handling
- **Character Set**: Character ROM configuration
- **Scrolling**: Horizontal and vertical scroll
- **Player/Missile**: P/M graphics configuration

**Coverage**: 13 test cases, 61 assertions

## Test Framework

### Catch2

We use [Catch2 v2.13.10](https://github.com/catchorg/Catch2) as our testing framework.

**Why Catch2?**
- Header-only (no build dependencies)
- Excellent test output and reporting
- BDD-style test organization
- Works on all platforms (Linux, macOS, Windows, ESP32 build environment)

### Test Structure

```cpp
TEST_CASE("Test description", "[tag][subtag]") {
    // Setup
    TestObject obj;

    SECTION("Specific behavior") {
        // Test code
        REQUIRE(condition == expected);
    }
}
```

## Adding New Tests

### Step 1: Create Test File

Create a new file in `test/`:

```cpp
#include "catch.hpp"
#include "../src/YourComponent.h"

TEST_CASE("Component behavior", "[component]") {
    SECTION("Specific test") {
        REQUIRE(true);
    }
}
```

### Step 2: Add to Makefile

Edit `Makefile` to include your test file:

```makefile
TEST_SOURCES = $(TEST_DIR)/test_main.cpp \
               $(TEST_DIR)/test_cpu6502.cpp \
               $(TEST_DIR)/test_memory.cpp \
               $(TEST_DIR)/test_antic.cpp \
               $(TEST_DIR)/test_your_component.cpp
```

### Step 3: Run Tests

```bash
make test-clean
make test
```

## CI/CD Integration

Tests run automatically on every push via GitHub Actions:

- ✅ All unit tests
- ✅ ESP32 compilation (all boards)
- ✅ Code formatting checks

See `.github/workflows/tests.yml` for configuration.

## Future Test Enhancements

### High Priority

1. **Full CPU Opcode Tests**
   - Test all 256 opcodes (including undocumented)
   - Cycle counting verification
   - Integration with Atari800Sys

2. **ANTIC Integration Tests**
   - Display list execution
   - Screen rendering pipeline
   - DLI and VBI timing

3. **GTIA Tests**
   - Color palette
   - Player/missile graphics
   - Collision detection

4. **POKEY Tests**
   - Audio synthesis
   - Keyboard scanning
   - Serial I/O

### Medium Priority

5. **Integration Tests**
   - Boot sequence
   - Load and run Atari programs
   - Regression tests with golden references

6. **Performance Tests**
   - Cycle accuracy benchmarks
   - Frame rate measurement
   - Memory usage profiling

### Low Priority

7. **Hardware Abstraction Tests**
   - Mock display driver
   - Mock keyboard driver
   - Factory pattern tests

## Test Coverage Goals

| Component | Current | Target |
|-----------|---------|--------|
| CPU6502   | Basic   | 90%+   |
| Memory    | Basic   | 85%+   |
| ANTIC     | Basic   | 80%+   |
| GTIA      | None    | 75%+   |
| POKEY     | None    | 75%+   |
| PIA       | None    | 70%+   |
| Integration| None   | 60%+   |

## Troubleshooting

### Tests Fail to Compile

```bash
# Clean build and try again
make test-clean
make test

# Check g++ version (need C++17 support)
g++ --version  # Should be 7.0 or higher
```

### Catch2 Missing

```bash
# Re-download Catch2
curl -L -o test/catch.hpp https://raw.githubusercontent.com/catchorg/Catch2/v2.13.10/single_include/catch2/catch.hpp
```

### Specific Test Failing

```bash
# Run with verbose output
make test-verbose

# Run only specific tests by tag
./test/build/test_runner "[cpu][init]"
```

## Resources

- [Catch2 Documentation](https://github.com/catchorg/Catch2/blob/v2.x/docs/Readme.md)
- [6502 Test Suite](https://github.com/Klaus2m5/6502_65C02_functional_tests)
- [Atari 800 Technical Reference](http://www.atariarchives.org/APX/showinfo.php?cat=20049)

## Contributing

When adding features:
1. Write tests first (TDD approach)
2. Ensure all existing tests pass
3. Add new tests for new functionality
4. Update this README if adding new test categories

## License

Test code follows the same GPL v3 license as the main project.
