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
#include "GTIA.h"
#include <cstring>

GTIA::GTIA() { reset(); }

void GTIA::reset() {
  memset(hposp, 0, sizeof(hposp));
  memset(hposm, 0, sizeof(hposm));
  memset(sizep, 0, sizeof(sizep));
  sizem = 0;
  memset(grafp, 0, sizeof(grafp));
  grafm = 0;

  // Default colors (typical Atari startup colors)
  colpm[0] = 0x38;  // Player 0: Red
  colpm[1] = 0x58;  // Player 1: Orange
  colpm[2] = 0x88;  // Player 2: Blue
  colpm[3] = 0xC8;  // Player 3: Green

  colpf[0] = 0x28;  // Playfield 0
  colpf[1] = 0x48;  // Playfield 1
  colpf[2] = 0x94;  // Playfield 2 (default text background)
  colpf[3] = 0x46;  // Playfield 3

  colbk = 0x00;     // Background: Black

  prior = 0;
  vdelay = 0;
  gractl = 0;

  memset(m2pf, 0, sizeof(m2pf));
  memset(p2pf, 0, sizeof(p2pf));
  memset(m2pl, 0, sizeof(m2pl));
  memset(p2pl, 0, sizeof(p2pl));

  // Triggers not pressed (active-low)
  for (int i = 0; i < 4; i++) {
    trig[i] = 1;
  }

  // Console switches not pressed (active-low)
  consol = 0x07;

  isPAL = true;  // Default to PAL
}

uint8_t GTIA::read(uint8_t addr) {
  addr &= 0x1F;

  switch (addr) {
  // Missile to playfield collisions
  case M0PF:
    return m2pf[0];
  case M1PF:
    return m2pf[1];
  case M2PF:
    return m2pf[2];
  case M3PF:
    return m2pf[3];

  // Player to playfield collisions
  case P0PF:
    return p2pf[0];
  case P1PF:
    return p2pf[1];
  case P2PF:
    return p2pf[2];
  case P3PF:
    return p2pf[3];

  // Missile to player collisions
  case M0PL:
    return m2pl[0];
  case M1PL:
    return m2pl[1];
  case M2PL:
    return m2pl[2];
  case M3PL:
    return m2pl[3];

  // Player to player collisions
  case P0PL:
    return p2pl[0];
  case P1PL:
    return p2pl[1];
  case P2PL:
    return p2pl[2];
  case P3PL:
    return p2pl[3];

  // Triggers (active-low)
  case TRIG0:
    return trig[0];
  case TRIG1:
    return trig[1];
  case TRIG2:
    return trig[2];
  case TRIG3:
    return trig[3];

  // PAL/NTSC flag
  case PAL:
    // Bits 1-3 indicate PAL (0x0E) or NTSC (0x0F)
    // Actually: PAL returns 0x01, NTSC returns 0x0F in bits 1-3
    return isPAL ? 0x01 : 0x0F;

  // Console switches
  case CONSOL_R:
    // Bits 0-2: START, SELECT, OPTION (active-low)
    // Bits 3-7: Speaker control (active-low output, but reads as input)
    return consol | 0xF8;

  default:
    return 0xFF;
  }
}

void GTIA::write(uint8_t addr, uint8_t val) {
  addr &= 0x1F;

  switch (addr) {
  // Player horizontal positions
  case HPOSP0:
    hposp[0] = val;
    break;
  case HPOSP1:
    hposp[1] = val;
    break;
  case HPOSP2:
    hposp[2] = val;
    break;
  case HPOSP3:
    hposp[3] = val;
    break;

  // Missile horizontal positions
  case HPOSM0:
    hposm[0] = val;
    break;
  case HPOSM1:
    hposm[1] = val;
    break;
  case HPOSM2:
    hposm[2] = val;
    break;
  case HPOSM3:
    hposm[3] = val;
    break;

  // Player sizes
  case SIZEP0:
    sizep[0] = val & 0x03;
    break;
  case SIZEP1:
    sizep[1] = val & 0x03;
    break;
  case SIZEP2:
    sizep[2] = val & 0x03;
    break;
  case SIZEP3:
    sizep[3] = val & 0x03;
    break;

  // Missile sizes (2 bits per missile)
  case SIZEM:
    sizem = val;
    break;

  // Player graphics data
  case GRAFP0:
    grafp[0] = val;
    break;
  case GRAFP1:
    grafp[1] = val;
    break;
  case GRAFP2:
    grafp[2] = val;
    break;
  case GRAFP3:
    grafp[3] = val;
    break;

  // Missile graphics data
  case GRAFM:
    grafm = val;
    break;

  // Player/missile colors
  case COLPM0:
    colpm[0] = val;
    break;
  case COLPM1:
    colpm[1] = val;
    break;
  case COLPM2:
    colpm[2] = val;
    break;
  case COLPM3:
    colpm[3] = val;
    break;

  // Playfield colors
  case COLPF0:
    colpf[0] = val;
    break;
  case COLPF1:
    colpf[1] = val;
    break;
  case COLPF2:
    colpf[2] = val;
    break;
  case COLPF3:
    colpf[3] = val;
    break;

  // Background color
  case COLBK:
    colbk = val;
    break;

  // Priority control
  case PRIOR:
    prior = val;
    break;

  // Vertical delay
  case VDELAY:
    vdelay = val;
    break;

  // Graphics control
  case GRACTL:
    gractl = val;
    break;

  // Clear all collision registers
  case HITCLR:
    clearCollisions();
    break;

  // Console output (speaker)
  case CONSOL_W:
    // Bit 3 controls internal speaker
    // We ignore this for now
    break;
  }
}

void GTIA::setCollision(uint8_t type, uint8_t obj, uint8_t bits) {
  if (obj >= 4)
    return;

  switch (type) {
  case 0: // Missile to playfield
    m2pf[obj] |= bits;
    break;
  case 1: // Player to playfield
    p2pf[obj] |= bits;
    break;
  case 2: // Missile to player
    m2pl[obj] |= bits;
    break;
  case 3: // Player to player
    p2pl[obj] |= bits;
    break;
  }
}

void GTIA::clearCollisions() {
  memset(m2pf, 0, sizeof(m2pf));
  memset(p2pf, 0, sizeof(p2pf));
  memset(m2pl, 0, sizeof(m2pl));
  memset(p2pl, 0, sizeof(p2pl));
}

void GTIA::setTrigger(uint8_t index, bool pressed) {
  if (index < 4) {
    // Active-low: 0 = pressed, 1 = not pressed
    trig[index] = pressed ? 0 : 1;
  }
}

void GTIA::setConsoleKey(uint8_t key, bool pressed) {
  // key: 0=START, 1=SELECT, 2=OPTION
  if (key < 3) {
    // Active-low: 0 = pressed
    if (pressed) {
      consol &= ~(1 << key);
    } else {
      consol |= (1 << key);
    }
  }
}
