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
#ifndef PIA_H
#define PIA_H

#include <cstdint>

// PIA register addresses (offset from base $D300)
constexpr uint8_t PORTA = 0x00;    // Port A data (joystick directions)
constexpr uint8_t PORTB = 0x01;    // Port B data (memory control, etc.)
constexpr uint8_t PACTL = 0x02;    // Port A control
constexpr uint8_t PBCTL = 0x03;    // Port B control

// PORTA bits (active-low joystick inputs on 800/XL/XE)
// For Atari 800 style ports:
constexpr uint8_t JOY1_UP = 0x01;     // Joystick 1 up
constexpr uint8_t JOY1_DOWN = 0x02;   // Joystick 1 down
constexpr uint8_t JOY1_LEFT = 0x04;   // Joystick 1 left
constexpr uint8_t JOY1_RIGHT = 0x08;  // Joystick 1 right
constexpr uint8_t JOY2_UP = 0x10;     // Joystick 2 up
constexpr uint8_t JOY2_DOWN = 0x20;   // Joystick 2 down
constexpr uint8_t JOY2_LEFT = 0x40;   // Joystick 2 left
constexpr uint8_t JOY2_RIGHT = 0x80;  // Joystick 2 right

// PORTB bits (memory/bank control on XL/XE)
constexpr uint8_t PORTB_OS_ROM = 0x01;     // OS ROM enable (0=enabled)
constexpr uint8_t PORTB_BASIC = 0x02;      // BASIC ROM enable (0=enabled)
constexpr uint8_t PORTB_LED = 0x04;        // Keyboard LED control
constexpr uint8_t PORTB_SELFTEST = 0x80;   // Self-test ROM enable (0=enabled)

// PACTL/PBCTL bits
constexpr uint8_t PIA_IRQ1 = 0x80;         // IRQ1 status
constexpr uint8_t PIA_IRQ2 = 0x40;         // IRQ2 status
constexpr uint8_t PIA_DDR = 0x04;          // 0=DDR selected, 1=Data register
constexpr uint8_t PIA_C2_OUTPUT = 0x08;    // CA2/CB2 direction (0=input, 1=output)
constexpr uint8_t PIA_C2_CTRL = 0x30;      // CA2/CB2 control bits
constexpr uint8_t PIA_C1_CTRL = 0x03;      // CA1/CB1 control bits

/**
 * @brief PIA - Peripheral Interface Adapter (6520/6522)
 *
 * The PIA chip handles:
 * - Joystick direction inputs (PORTA)
 * - Memory banking on XL/XE machines (PORTB)
 * - Peripheral interrupts
 * - SIO motor control
 *
 * The Atari 800 has a 6520 PIA at $D300-$D3FF.
 */
class PIA {
private:
  // Port A (joysticks)
  uint8_t porta;        // Data register
  uint8_t ddra;         // Data direction register
  uint8_t pactl;        // Control register

  // Port B (memory/peripherals)
  uint8_t portb;        // Data register
  uint8_t ddrb;         // Data direction register
  uint8_t pbctl;        // Control register

  // Joystick state (directly written by emulator)
  uint8_t joy1;         // Joystick 1 direction bits (active-high internally)
  uint8_t joy2;         // Joystick 2 direction bits (active-high internally)

public:
  PIA();
  void reset();

  // Register access
  uint8_t read(uint8_t addr);
  void write(uint8_t addr, uint8_t val);

  // Joystick interface (called by emulator)
  void setJoystick1(bool up, bool down, bool left, bool right);
  void setJoystick2(bool up, bool down, bool left, bool right);

  // Memory control (for XL/XE banking)
  uint8_t getPortB() const { return portb; }
  bool isOSROMEnabled() const { return (portb & PORTB_OS_ROM) == 0; }
  bool isBASICEnabled() const { return (portb & PORTB_BASIC) == 0; }
  bool isSelfTestEnabled() const { return (portb & PORTB_SELFTEST) == 0; }
};

#endif // PIA_H
