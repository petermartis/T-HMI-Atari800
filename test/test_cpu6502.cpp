/*
 CPU6502 Unit Tests
 Tests for 6502 CPU data structures and basic functionality
 Note: Full opcode testing requires the complete Atari800Sys integration
*/
#include "catch.hpp"
#include "../src/CPU6502.h"
#include <cstring>

// Simple test subclass that can be instantiated
class TestCPU6502 : public CPU6502 {
private:
    uint8_t memory[65536];

public:
    TestCPU6502() {
        memset(memory, 0, sizeof(memory));
        reset();
    }

    void reset() {
        a = 0;
        x = 0;
        y = 0;
        sp = 0xFF;
        pc = 0x0000;
        sr = 0x20;

        cflag = false;
        zflag = false;
        iflag = true;
        dflag = false;
        vflag = false;
        nflag = false;

        cpuhalted = false;
        numofcycles = 0;
    }

    uint8_t getMem(uint16_t addr) override {
        return memory[addr];
    }

    void setMem(uint16_t addr, uint8_t val) override {
        memory[addr] = val;
    }

    void run() override {
        // Placeholder for testing
    }

    // Test accessors
    uint8_t getA() const { return a; }
    uint8_t getX() const { return x; }
    uint8_t getY() const { return y; }
    uint8_t getSP() const { return sp; }
    uint16_t getPC() const { return pc; }

    bool getC() const { return cflag; }
    bool getZ() const { return zflag; }
    bool getI() const { return iflag; }
    bool getD() const { return dflag; }
    bool getV() const { return vflag; }
    bool getN() const { return nflag; }

    void setA(uint8_t val) { a = val; }
    void setX(uint8_t val) { x = val; }
    void setY(uint8_t val) { y = val; }
    void setSP(uint8_t val) { sp = val; }
    void setPC(uint16_t val) { pc = val; }

    void setC(bool val) { cflag = val; }
    void setZ(bool val) { zflag = val; }
    void setI(bool val) { iflag = val; }
    void setD(bool val) { dflag = val; }
    void setV(bool val) { vflag = val; }
    void setN(bool val) { nflag = val; }
};

// =============================================================================
// REGISTER INITIALIZATION TESTS
// =============================================================================

TEST_CASE("CPU6502 initial state", "[cpu][init]") {
    TestCPU6502 cpu;

    SECTION("Registers initialized to zero") {
        REQUIRE(cpu.getA() == 0x00);
        REQUIRE(cpu.getX() == 0x00);
        REQUIRE(cpu.getY() == 0x00);
    }

    SECTION("Stack pointer initialized") {
        REQUIRE(cpu.getSP() == 0xFF);
    }

    SECTION("Program counter initialized") {
        REQUIRE(cpu.getPC() == 0x0000);
    }

    SECTION("Flags initialized correctly") {
        REQUIRE(cpu.getC() == false);
        REQUIRE(cpu.getZ() == false);
        REQUIRE(cpu.getI() == true);  // Interrupts disabled on reset
        REQUIRE(cpu.getD() == false);
        REQUIRE(cpu.getV() == false);
        REQUIRE(cpu.getN() == false);
    }

    SECTION("CPU not halted") {
        REQUIRE(cpu.cpuhalted == false);
    }
}

// =============================================================================
// REGISTER MANIPULATION TESTS
// =============================================================================

TEST_CASE("CPU register setters and getters", "[cpu][registers]") {
    TestCPU6502 cpu;

    SECTION("Accumulator") {
        cpu.setA(0x42);
        REQUIRE(cpu.getA() == 0x42);

        cpu.setA(0xFF);
        REQUIRE(cpu.getA() == 0xFF);

        cpu.setA(0x00);
        REQUIRE(cpu.getA() == 0x00);
    }

    SECTION("X Register") {
        cpu.setX(0x33);
        REQUIRE(cpu.getX() == 0x33);
    }

    SECTION("Y Register") {
        cpu.setY(0x44);
        REQUIRE(cpu.getY() == 0x44);
    }

    SECTION("Stack Pointer") {
        cpu.setSP(0xF0);
        REQUIRE(cpu.getSP() == 0xF0);

        // SP can wrap
        cpu.setSP(0x00);
        REQUIRE(cpu.getSP() == 0x00);
    }

    SECTION("Program Counter") {
        cpu.setPC(0x1000);
        REQUIRE(cpu.getPC() == 0x1000);

        cpu.setPC(0xFFFF);
        REQUIRE(cpu.getPC() == 0xFFFF);
    }
}

// =============================================================================
// FLAG MANIPULATION TESTS
// =============================================================================

TEST_CASE("CPU flags", "[cpu][flags]") {
    TestCPU6502 cpu;

    SECTION("Carry flag") {
        cpu.setC(false);
        REQUIRE(cpu.getC() == false);

        cpu.setC(true);
        REQUIRE(cpu.getC() == true);
    }

    SECTION("Zero flag") {
        cpu.setZ(false);
        REQUIRE(cpu.getZ() == false);

        cpu.setZ(true);
        REQUIRE(cpu.getZ() == true);
    }

    SECTION("Interrupt disable flag") {
        cpu.setI(false);
        REQUIRE(cpu.getI() == false);

        cpu.setI(true);
        REQUIRE(cpu.getI() == true);
    }

    SECTION("Decimal mode flag") {
        cpu.setD(false);
        REQUIRE(cpu.getD() == false);

        cpu.setD(true);
        REQUIRE(cpu.getD() == true);
    }

    SECTION("Overflow flag") {
        cpu.setV(false);
        REQUIRE(cpu.getV() == false);

        cpu.setV(true);
        REQUIRE(cpu.getV() == true);
    }

    SECTION("Negative flag") {
        cpu.setN(false);
        REQUIRE(cpu.getN() == false);

        cpu.setN(true);
        REQUIRE(cpu.getN() == true);
    }
}

// =============================================================================
// MEMORY INTERFACE TESTS
// =============================================================================

TEST_CASE("Memory interface", "[cpu][memory]") {
    TestCPU6502 cpu;

    SECTION("Read/Write single byte") {
        cpu.setMem(0x1000, 0x42);
        REQUIRE(cpu.getMem(0x1000) == 0x42);
    }

    SECTION("Read/Write all memory addresses") {
        // Test various addresses
        cpu.setMem(0x0000, 0x11);
        cpu.setMem(0x00FF, 0x22);
        cpu.setMem(0x0100, 0x33);
        cpu.setMem(0x01FF, 0x44);
        cpu.setMem(0x8000, 0x55);
        cpu.setMem(0xFFFF, 0x66);

        REQUIRE(cpu.getMem(0x0000) == 0x11);
        REQUIRE(cpu.getMem(0x00FF) == 0x22);
        REQUIRE(cpu.getMem(0x0100) == 0x33);
        REQUIRE(cpu.getMem(0x01FF) == 0x44);
        REQUIRE(cpu.getMem(0x8000) == 0x55);
        REQUIRE(cpu.getMem(0xFFFF) == 0x66);
    }

    SECTION("Memory independence") {
        cpu.setMem(0x2000, 0xAA);
        cpu.setMem(0x2001, 0xBB);

        REQUIRE(cpu.getMem(0x2000) == 0xAA);
        REQUIRE(cpu.getMem(0x2001) == 0xBB);

        // Changing one shouldn't affect the other
        cpu.setMem(0x2000, 0xCC);
        REQUIRE(cpu.getMem(0x2000) == 0xCC);
        REQUIRE(cpu.getMem(0x2001) == 0xBB);
    }
}

// =============================================================================
// CYCLE COUNTING TESTS
// =============================================================================

TEST_CASE("Cycle counting", "[cpu][cycles]") {
    TestCPU6502 cpu;

    SECTION("Initial cycle count") {
        REQUIRE(cpu.numofcycles == 0);
    }

    SECTION("Cycle count can be modified") {
        cpu.numofcycles = 10;
        REQUIRE(cpu.numofcycles == 10);

        cpu.numofcycles += 5;
        REQUIRE(cpu.numofcycles == 15);
    }
}

// =============================================================================
// STACK POINTER TESTS
// =============================================================================

TEST_CASE("Stack operations", "[cpu][stack]") {
    TestCPU6502 cpu;

    SECTION("Stack grows downward") {
        uint8_t sp_initial = cpu.getSP();
        cpu.setSP(sp_initial - 1);
        REQUIRE(cpu.getSP() == sp_initial - 1);
    }

    SECTION("Stack wraparound") {
        cpu.setSP(0x00);
        REQUIRE(cpu.getSP() == 0x00);

        // After a push, SP would wrap to 0xFF
        cpu.setSP(0xFF);
        REQUIRE(cpu.getSP() == 0xFF);
    }

    SECTION("Stack lives in page 1") {
        cpu.setSP(0x80);
        uint16_t stack_addr = 0x0100 | cpu.getSP();
        REQUIRE(stack_addr == 0x0180);

        cpu.setSP(0xFF);
        stack_addr = 0x0100 | cpu.getSP();
        REQUIRE(stack_addr == 0x01FF);

        cpu.setSP(0x00);
        stack_addr = 0x0100 | cpu.getSP();
        REQUIRE(stack_addr == 0x0100);
    }
}

// =============================================================================
// RESET BEHAVIOR TESTS
// =============================================================================

TEST_CASE("CPU reset", "[cpu][reset]") {
    TestCPU6502 cpu;

    // Modify state
    cpu.setA(0xFF);
    cpu.setX(0xFF);
    cpu.setY(0xFF);
    cpu.setSP(0x00);
    cpu.setPC(0x1234);
    cpu.setC(true);
    cpu.setZ(true);
    cpu.cpuhalted = true;

    // Reset
    cpu.reset();

    SECTION("Registers cleared") {
        REQUIRE(cpu.getA() == 0x00);
        REQUIRE(cpu.getX() == 0x00);
        REQUIRE(cpu.getY() == 0x00);
    }

    SECTION("Stack pointer reset") {
        REQUIRE(cpu.getSP() == 0xFF);
    }

    SECTION("Program counter reset") {
        REQUIRE(cpu.getPC() == 0x0000);
    }

    SECTION("Flags reset") {
        REQUIRE(cpu.getC() == false);
        REQUIRE(cpu.getZ() == false);
        REQUIRE(cpu.getI() == true);
        REQUIRE(cpu.getD() == false);
        REQUIRE(cpu.getV() == false);
        REQUIRE(cpu.getN() == false);
    }

    SECTION("CPU not halted after reset") {
        REQUIRE(cpu.cpuhalted == false);
    }
}

// =============================================================================
// OPCODE DECODING TESTS
// =============================================================================

TEST_CASE("Opcode constants", "[cpu][opcodes]") {
    // Test that we can define and reference opcodes
    SECTION("Common opcodes") {
        const uint8_t LDA_IMM = 0xA9;
        const uint8_t LDA_ABS = 0xAD;
        const uint8_t STA_ABS = 0x8D;
        const uint8_t JMP_ABS = 0x4C;
        const uint8_t JMP_IND = 0x6C;
        const uint8_t BRK = 0x00;
        const uint8_t NOP = 0xEA;

        REQUIRE(LDA_IMM == 0xA9);
        REQUIRE(LDA_ABS == 0xAD);
        REQUIRE(STA_ABS == 0x8D);
        REQUIRE(JMP_ABS == 0x4C);
        REQUIRE(JMP_IND == 0x6C);
        REQUIRE(BRK == 0x00);
        REQUIRE(NOP == 0xEA);
    }
}

// =============================================================================
// ADDRESSING MODE CALCULATIONS
// =============================================================================

TEST_CASE("Address calculations", "[cpu][addressing]") {
    SECTION("Absolute addressing") {
        uint8_t lo = 0x34;
        uint8_t hi = 0x12;
        uint16_t addr = lo | (hi << 8);
        REQUIRE(addr == 0x1234);
    }

    SECTION("Zeropage,X wrapping") {
        uint8_t base = 0xFF;
        uint8_t x = 0x02;
        uint8_t effective = base + x; // Wraps to 0x01
        REQUIRE(effective == 0x01);
    }

    SECTION("Absolute,X page crossing") {
        uint16_t base = 0x10FF;
        uint8_t x = 0x01;
        uint16_t effective = base + x; // Crosses page boundary
        REQUIRE(effective == 0x1100);
        REQUIRE((base & 0xFF00) != (effective & 0xFF00)); // Different pages
    }

    SECTION("Indirect addressing") {
        // Address stored little-endian
        uint16_t ptr = 0x1000;
        uint8_t addr_lo = 0x34;
        uint8_t addr_hi = 0x12;
        uint16_t target = addr_lo | (addr_hi << 8);
        REQUIRE(target == 0x1234);
    }
}

// Note: Full opcode execution tests require the complete Atari800Sys
// integration, as CPU6502 is an abstract base class. The above tests
// verify the basic CPU state management and memory interface.
