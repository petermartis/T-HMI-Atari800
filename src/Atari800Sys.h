/*
 Copyright (C) 2024-2025 retroelec <retroelec42@gmail.com>

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 3 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 for more details.

 For the complete text of the GNU General Public License see
 http://www.gnu.org/licenses/.
*/
#ifndef ATARI800SYS_H
#define ATARI800SYS_H

#include "ANTIC.h"
#include "CPU6502.h"
#include "GTIA.h"
#include "PIA.h"
#include "POKEY.h"
#include "keyboard/KeyboardDriver.h"
#include "joystick/JoystickDriver.h"
#include <atomic>
#include <cstdint>

// Atari 800 XL/XE Memory Map
// $0000-$3FFF: RAM (16KB base)
// $4000-$7FFF: RAM (additional 16KB)
// $8000-$9FFF: Cartridge ROM / RAM
// $A000-$BFFF: BASIC ROM / Cartridge / RAM
// $C000-$CFFF: Self-test ROM / RAM (XL/XE)
// $D000-$D0FF: GTIA registers (mirrored)
// $D100-$D1FF: (reserved)
// $D200-$D2FF: POKEY registers (mirrored)
// $D300-$D3FF: PIA registers (mirrored)
// $D400-$D4FF: ANTIC registers (mirrored)
// $D500-$D7FF: (reserved/cart control)
// $D800-$FFFF: OS ROM / floating point / character set

// Memory size options
constexpr uint32_t MEM_16K = 16 * 1024;
constexpr uint32_t MEM_48K = 48 * 1024;
constexpr uint32_t MEM_64K = 64 * 1024;  // XL/XE standard
constexpr uint32_t MEM_128K = 128 * 1024; // 130XE extended

// ROM sizes
constexpr uint16_t OS_ROM_SIZE = 16 * 1024;    // $C000-$FFFF
constexpr uint16_t BASIC_ROM_SIZE = 8 * 1024;  // $A000-$BFFF
constexpr uint16_t CHARSET_SIZE = 1024;         // Character set

/**
 * @brief Atari 800 System - combines CPU and all chips
 *
 * This class implements the main Atari 800 XL/XE computer system,
 * including memory mapping, I/O routing, and interrupt handling.
 */
class Atari800Sys : public CPU6502 {
private:
  // Memory
  uint8_t *ram;                    // Main RAM (64KB)
  const uint8_t *osRom;            // OS ROM (16KB)
  const uint8_t *basicRom;         // BASIC ROM (8KB)
  const uint8_t *charRom;          // Character ROM (built into OS)

  // Banking state (XL/XE)
  bool osRomEnabled;               // OS ROM visible ($C000-$FFFF)
  bool basicRomEnabled;            // BASIC ROM visible ($A000-$BFFF)
  bool selfTestEnabled;            // Self-test ROM visible ($5000-$57FF)

  // Input devices
  JoystickDriver *joystick;

  // Internal state
  bool nmiActive;                  // NMI being processed
  uint8_t lastIRQ;                 // Last IRQ source

  // Cycle counting
  int32_t cyclesThisScanline;
  int32_t cyclesPerScanline;

  // Debug
  inline void logDebugInfo() __attribute__((always_inline));

public:
  // Hardware chips
  ANTIC antic;
  GTIA gtia;
  POKEY pokey;
  PIA pia;

  // Keyboard
  KeyboardDriver *keyboard;

  // Profiling
  std::atomic<uint32_t> numofcyclespersecond;
  std::atomic<bool> perf;

  Atari800Sys();
  ~Atari800Sys();

  // Initialization
  void init(uint8_t *ram, const uint8_t *osRom, const uint8_t *basicRom);
  void reset();

  // CPU6502 interface implementations
  uint8_t getMem(uint16_t addr) override;
  void setMem(uint16_t addr, uint8_t val) override;
  void run() override;

  // Optional BRK handler override
  void cmd6502brk() override;

  // Memory access helpers
  uint8_t readIO(uint16_t addr);
  void writeIO(uint16_t addr, uint8_t val);

  // Input setup
  void setJoystick(JoystickDriver *joy) { joystick = joy; }
  void setKeyboard(KeyboardDriver *kb) { keyboard = kb; }

  // Banking control (XL/XE)
  void updateBanking();

  // Interrupt helpers
  void checkInterrupts();
  bool handleNMI();
  bool handleIRQ();

  // State access for external commands
  void setPC(uint16_t newPC) { pc = newPC; }
  uint16_t getPC() const { return pc; }
  uint8_t *getRam() { return ram; }

  // Keyboard scanning
  void scanKeyboard();
};

#endif // ATARI800SYS_H
