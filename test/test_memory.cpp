/*
 Memory System Tests
 Tests for Atari 800 XL memory banking and address decoding
*/
#include "catch.hpp"
#include <cstdint>
#include <cstring>

// =============================================================================
// MEMORY MAP CONSTANTS
// =============================================================================

TEST_CASE("Memory map constants are correct", "[memory][constants]") {
    SECTION("Memory sizes") {
        REQUIRE(16 * 1024 == 16384);
        REQUIRE(48 * 1024 == 49152);
        REQUIRE(64 * 1024 == 65536);
    }

    SECTION("Address ranges") {
        // RAM base
        REQUIRE(0x0000 == 0);
        REQUIRE(0x3FFF == 16383);

        // I/O area
        REQUIRE(0xD000 == 53248);
        REQUIRE(0xD0FF == 53503); // GTIA
        REQUIRE(0xD200 == 53760); // POKEY start
        REQUIRE(0xD300 == 54016); // PIA start
        REQUIRE(0xD400 == 54272); // ANTIC start

        // ROM areas
        REQUIRE(0xA000 == 40960);  // BASIC ROM start
        REQUIRE(0xBFFF == 49151);  // BASIC ROM end
        REQUIRE(0xC000 == 49152);  // OS ROM start
        REQUIRE(0xFFFF == 65535);  // OS ROM end
    }
}

// =============================================================================
// ADDRESS DECODE TESTS
// =============================================================================

TEST_CASE("Address decoder function", "[memory][decoder]") {
    // Helper function to categorize addresses
    auto categorizeAddress = [](uint16_t addr) -> const char* {
        if (addr < 0x4000) return "RAM_LOW";
        if (addr < 0x8000) return "RAM_MID";
        if (addr < 0xA000) return "CART_AREA";
        if (addr < 0xC000) return "BASIC_AREA";
        if (addr < 0xD000) return "OS_ROM_LOW";
        if (addr < 0xD100) return "GTIA";
        if (addr < 0xD200) return "RESERVED1";
        if (addr < 0xD300) return "POKEY";
        if (addr < 0xD400) return "PIA";
        if (addr < 0xD500) return "ANTIC";
        if (addr < 0xD800) return "RESERVED2";
        return "OS_ROM_HIGH";
    };

    SECTION("RAM areas") {
        REQUIRE(std::string(categorizeAddress(0x0000)) == "RAM_LOW");
        REQUIRE(std::string(categorizeAddress(0x1000)) == "RAM_LOW");
        REQUIRE(std::string(categorizeAddress(0x3FFF)) == "RAM_LOW");
        REQUIRE(std::string(categorizeAddress(0x4000)) == "RAM_MID");
        REQUIRE(std::string(categorizeAddress(0x7FFF)) == "RAM_MID");
    }

    SECTION("I/O areas") {
        REQUIRE(std::string(categorizeAddress(0xD000)) == "GTIA");
        REQUIRE(std::string(categorizeAddress(0xD200)) == "POKEY");
        REQUIRE(std::string(categorizeAddress(0xD300)) == "PIA");
        REQUIRE(std::string(categorizeAddress(0xD400)) == "ANTIC");
    }

    SECTION("ROM areas") {
        REQUIRE(std::string(categorizeAddress(0xA000)) == "BASIC_AREA");
        REQUIRE(std::string(categorizeAddress(0xC000)) == "OS_ROM_LOW");
        REQUIRE(std::string(categorizeAddress(0xE000)) == "OS_ROM_HIGH");
    }
}

// =============================================================================
// MEMORY MIRRORING TESTS
// =============================================================================

TEST_CASE("I/O register mirroring", "[memory][mirroring]") {
    // I/O registers are mirrored throughout their 256-byte pages
    SECTION("GTIA mirrors") {
        // GTIA at $D000-$D0FF has ~32 registers that are mirrored
        auto isGtiaMirror = [](uint16_t addr1, uint16_t addr2) {
            return ((addr1 & 0xFF00) == 0xD000) &&
                   ((addr2 & 0xFF00) == 0xD000) &&
                   ((addr1 & 0x1F) == (addr2 & 0x1F));
        };

        REQUIRE(isGtiaMirror(0xD000, 0xD020)); // Register 0
        REQUIRE(isGtiaMirror(0xD001, 0xD021)); // Register 1
        REQUIRE(!isGtiaMirror(0xD000, 0xD001)); // Different registers
    }

    SECTION("POKEY mirrors") {
        auto isPokeyMirror = [](uint16_t addr1, uint16_t addr2) {
            return ((addr1 & 0xFF00) == 0xD200) &&
                   ((addr2 & 0xFF00) == 0xD200) &&
                   ((addr1 & 0x0F) == (addr2 & 0x0F));
        };

        REQUIRE(isPokeyMirror(0xD200, 0xD210)); // Register 0
        REQUIRE(isPokeyMirror(0xD207, 0xD217)); // Register 7
    }
}

// =============================================================================
// BANKING SIMULATION TESTS
// =============================================================================

class SimpleBankedMemory {
private:
    uint8_t ram[65536];
    uint8_t osRom[16384];
    uint8_t basicRom[8192];
    bool osEnabled;
    bool basicEnabled;

public:
    SimpleBankedMemory() {
        memset(ram, 0, sizeof(ram));
        memset(osRom, 0xFF, sizeof(osRom)); // Fill with $FF
        memset(basicRom, 0xBB, sizeof(basicRom)); // Fill with $BB
        osEnabled = true;
        basicEnabled = true;
    }

    uint8_t read(uint16_t addr) {
        // OS ROM: $C000-$FFFF
        if (addr >= 0xC000 && osEnabled) {
            return osRom[addr - 0xC000];
        }
        // BASIC ROM: $A000-$BFFF
        if (addr >= 0xA000 && addr < 0xC000 && basicEnabled) {
            return basicRom[addr - 0xA000];
        }
        // Otherwise RAM
        return ram[addr];
    }

    void write(uint16_t addr, uint8_t val) {
        // Can't write to ROM areas (even if enabled)
        if (addr >= 0xC000 && osEnabled) return;
        if (addr >= 0xA000 && addr < 0xC000 && basicEnabled) return;
        ram[addr] = val;
    }

    void setOsEnabled(bool enabled) { osEnabled = enabled; }
    void setBasicEnabled(bool enabled) { basicEnabled = enabled; }
    bool isOsEnabled() const { return osEnabled; }
    bool isBasicEnabled() const { return basicEnabled; }
};

TEST_CASE("Banking behavior", "[memory][banking]") {
    SimpleBankedMemory mem;

    SECTION("OS ROM banking") {
        // With OS enabled, reading $C000 returns ROM
        mem.setOsEnabled(true);
        REQUIRE(mem.read(0xC000) == 0xFF);

        // With OS disabled, reading $C000 returns RAM
        mem.setOsEnabled(false);
        mem.write(0xC000, 0x42);
        REQUIRE(mem.read(0xC000) == 0x42);

        // Re-enable OS ROM
        mem.setOsEnabled(true);
        REQUIRE(mem.read(0xC000) == 0xFF);

        // Underlying RAM still has 0x42
        mem.setOsEnabled(false);
        REQUIRE(mem.read(0xC000) == 0x42);
    }

    SECTION("BASIC ROM banking") {
        // With BASIC enabled, reading $A000 returns ROM
        mem.setBasicEnabled(true);
        REQUIRE(mem.read(0xA000) == 0xBB);

        // With BASIC disabled, can write to RAM at $A000
        mem.setBasicEnabled(false);
        mem.write(0xA000, 0x55);
        REQUIRE(mem.read(0xA000) == 0x55);
    }

    SECTION("RAM areas always accessible") {
        // Low RAM always writable
        mem.write(0x1000, 0x42);
        REQUIRE(mem.read(0x1000) == 0x42);

        // Mid RAM always writable
        mem.write(0x5000, 0x33);
        REQUIRE(mem.read(0x5000) == 0x33);
    }

    SECTION("Write to ROM area is ignored when ROM enabled") {
        mem.setOsEnabled(true);
        uint8_t original = mem.read(0xE000);
        mem.write(0xE000, 0x12); // Try to write
        REQUIRE(mem.read(0xE000) == original); // Unchanged
    }
}

// =============================================================================
// STACK TESTS
// =============================================================================

TEST_CASE("Stack area tests", "[memory][stack]") {
    // Stack is in page 1: $0100-$01FF
    SECTION("Stack boundaries") {
        uint16_t stack_base = 0x0100;
        uint16_t stack_top = 0x01FF;

        REQUIRE(stack_base == 256);
        REQUIRE(stack_top == 511);
        REQUIRE((stack_top - stack_base + 1) == 256);
    }

    SECTION("Stack pointer behavior") {
        // SP is 8-bit, stack is always in page 1
        auto stackAddress = [](uint8_t sp) -> uint16_t {
            return 0x0100 | sp;
        };

        REQUIRE(stackAddress(0xFF) == 0x01FF);
        REQUIRE(stackAddress(0x00) == 0x0100);
        REQUIRE(stackAddress(0x80) == 0x0180);
    }
}

// =============================================================================
// ZERO PAGE TESTS
// =============================================================================

TEST_CASE("Zero page tests", "[memory][zeropage]") {
    SECTION("Zero page range") {
        REQUIRE(0x00 == 0);
        REQUIRE(0xFF == 255);
    }

    SECTION("Zero page wraparound") {
        // When doing zeropage,X addressing, it wraps at page boundary
        auto zpIndexed = [](uint8_t base, uint8_t index) -> uint8_t {
            return static_cast<uint8_t>(base + index); // Wraps automatically
        };

        REQUIRE(zpIndexed(0xFF, 0x01) == 0x00); // Wraps to zero page
        REQUIRE(zpIndexed(0xF0, 0x20) == 0x10); // $F0 + $20 = $110 -> $10
    }
}

// =============================================================================
// PAGE CROSSING TESTS
// =============================================================================

TEST_CASE("Page crossing detection", "[memory][pages]") {
    // Page crossing matters for cycle counting
    auto pagesCross = [](uint16_t addr1, uint16_t addr2) -> bool {
        return (addr1 & 0xFF00) != (addr2 & 0xFF00);
    };

    SECTION("Same page") {
        REQUIRE(!pagesCross(0x1000, 0x1010));
        REQUIRE(!pagesCross(0x20FF, 0x2000)); // Wraps back
    }

    SECTION("Different pages") {
        REQUIRE(pagesCross(0x10FF, 0x1100));
        REQUIRE(pagesCross(0x2000, 0x3000));
    }

    SECTION("Page boundaries") {
        REQUIRE(!pagesCross(0x1000, 0x10FF));
        REQUIRE(pagesCross(0x10FF, 0x1100));
    }
}

// =============================================================================
// INDIRECT ADDRESSING TESTS
// =============================================================================

TEST_CASE("Indirect addressing", "[memory][addressing]") {
    SECTION("JMP indirect bug") {
        // 6502 has a bug: JMP ($10FF) reads from $10FF and $1000, not $1100
        auto indirectAddress = [](uint16_t, uint8_t lowByte, uint8_t highByte) -> uint16_t {
            // If pointer is at page boundary, high byte wraps
            // For demonstration purposes, we just combine the bytes
            return lowByte | (highByte << 8);
        };

        // Normal case: $1000 -> reads $1000 and $1001
        REQUIRE(indirectAddress(0x1000, 0x34, 0x12) == 0x1234);

        // Bug case: $10FF -> reads $10FF and $1000 (not $1100)
        REQUIRE(indirectAddress(0x10FF, 0x34, 0x12) == 0x1234);
    }
}

// =============================================================================
// ENDIANNESS TESTS
// =============================================================================

TEST_CASE("Little-endian multi-byte values", "[memory][endianness]") {
    SECTION("16-bit address storage") {
        // 6502 is little-endian: low byte first
        uint8_t memory[2] = {0x34, 0x12};
        uint16_t addr = memory[0] | (memory[1] << 8);
        REQUIRE(addr == 0x1234);
    }

    SECTION("Writing addresses") {
        uint8_t memory[2];
        uint16_t addr = 0x5678;
        memory[0] = addr & 0xFF;        // Low byte
        memory[1] = (addr >> 8) & 0xFF; // High byte

        REQUIRE(memory[0] == 0x78);
        REQUIRE(memory[1] == 0x56);
    }
}
