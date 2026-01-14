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
#include "PIA.h"

PIA::PIA() { reset(); }

void PIA::reset() {
  porta = 0xFF;  // All inputs high (no joystick pressed)
  ddra = 0x00;   // All inputs
  pactl = 0x00;

  // PORTB defaults for XL/XE (all ROM enabled)
  portb = 0xFF;
  ddrb = 0x00;
  pbctl = 0x00;

  joy1 = 0;  // No joystick input
  joy2 = 0;
}

uint8_t PIA::read(uint8_t addr) {
  addr &= 0x03;

  switch (addr) {
  case PORTA:
    if (pactl & PIA_DDR) {
      // Read data register
      // Combine joystick input with port settings
      // Joystick inputs are active-low
      uint8_t joy1Bits = joy1 ? ((~joy1) & 0x0F) : 0x0F;
      uint8_t joy2Bits = joy2 ? ((~joy2) & 0x0F) << 4 : 0xF0;
      // Inputs come from joysticks, outputs from porta
      return (joy1Bits & ~ddra) | (joy2Bits & ~ddra) | (porta & ddra);
    } else {
      // Read DDR
      return ddra;
    }

  case PORTB:
    if (pbctl & PIA_DDR) {
      // Read data register
      return portb;
    } else {
      // Read DDR
      return ddrb;
    }

  case PACTL:
    return pactl;

  case PBCTL:
    return pbctl;

  default:
    return 0xFF;
  }
}

void PIA::write(uint8_t addr, uint8_t val) {
  addr &= 0x03;

  switch (addr) {
  case PORTA:
    if (pactl & PIA_DDR) {
      // Write data register (only output bits affected)
      porta = val;
    } else {
      // Write DDR
      ddra = val;
    }
    break;

  case PORTB:
    if (pbctl & PIA_DDR) {
      // Write data register
      // Only bits set as outputs in ddrb can be written
      portb = (val & ddrb) | (portb & ~ddrb);
    } else {
      // Write DDR
      ddrb = val;
    }
    break;

  case PACTL:
    pactl = val;
    break;

  case PBCTL:
    pbctl = val;
    break;
  }
}

void PIA::setJoystick1(bool up, bool down, bool left, bool right) {
  joy1 = 0;
  if (up)    joy1 |= 0x01;
  if (down)  joy1 |= 0x02;
  if (left)  joy1 |= 0x04;
  if (right) joy1 |= 0x08;
}

void PIA::setJoystick2(bool up, bool down, bool left, bool right) {
  joy2 = 0;
  if (up)    joy2 |= 0x01;
  if (down)  joy2 |= 0x02;
  if (left)  joy2 |= 0x04;
  if (right) joy2 |= 0x08;
}
