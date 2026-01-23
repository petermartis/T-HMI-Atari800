/*
 Test Mock for CPU6502 - provides a simple memory interface for testing
*/
#ifndef MOCKCPU6502_H
#define MOCKCPU6502_H

#include "../../src/CPU6502.h"
#include <cstring>

class MockCPU6502 : public CPU6502 {
private:
    uint8_t memory[65536]; // 64K memory

public:
    MockCPU6502() {
        memset(memory, 0, sizeof(memory));
        reset();
    }

    void reset() {
        // Initialize CPU state
        a = 0;
        x = 0;
        y = 0;
        sp = 0xFF;
        pc = 0x0000;
        sr = 0x20; // Bit 5 is always 1

        cflag = false;
        zflag = false;
        iflag = true;  // Interrupts disabled on reset
        dflag = false;
        vflag = false;
        nflag = false;

        cpuhalted = false;
        numofcycles = 0;
    }

    // Memory interface
    uint8_t getMem(uint16_t addr) override {
        return memory[addr];
    }

    void setMem(uint16_t addr, uint8_t val) override {
        memory[addr] = val;
    }

    // Helper to write a program into memory
    void loadProgram(uint16_t addr, const uint8_t* data, size_t len) {
        memcpy(&memory[addr], data, len);
        pc = addr;
    }

    // Execute a single instruction by calling the internal execute method
    void executeInstruction() {
        // We can't access cmdarr6502 directly as it's private
        // Instead, we'll call run() which should execute one instruction
        run();
    }

    // Run until halt or max cycles
    void run() override {
        // Execute one instruction
        // Since we can't access the private cmdarr6502, we need a workaround
        // The actual implementation would be in Atari800Sys
        // For now, we'll note that full CPU tests need the actual Atari800Sys
        // This mock is for basic testing only
        if (!cpuhalted) {
            // Placeholder - real implementation would fetch and execute opcode
            numofcycles++;
        }
    }

    // Accessors for testing
    uint8_t getA() const { return a; }
    uint8_t getX() const { return x; }
    uint8_t getY() const { return y; }
    uint8_t getSP() const { return sp; }
    uint16_t getPC() const { return pc; }
    uint8_t getSR() const { return sr; }

    bool getC() const { return cflag; }
    bool getZ() const { return zflag; }
    bool getI() const { return iflag; }
    bool getD() const { return dflag; }
    bool getV() const { return vflag; }
    bool getN() const { return nflag; }

    // Setters for testing setup
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

#endif // MOCKCPU6502_H
