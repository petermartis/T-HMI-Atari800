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
#ifndef GTIA_H
#define GTIA_H

#include <cstdint>

// GTIA register addresses (offset from base $D000)
// Write registers - Horizontal position
constexpr uint8_t HPOSP0 = 0x00;  // Player 0 horizontal position
constexpr uint8_t HPOSP1 = 0x01;  // Player 1 horizontal position
constexpr uint8_t HPOSP2 = 0x02;  // Player 2 horizontal position
constexpr uint8_t HPOSP3 = 0x03;  // Player 3 horizontal position
constexpr uint8_t HPOSM0 = 0x04;  // Missile 0 horizontal position
constexpr uint8_t HPOSM1 = 0x05;  // Missile 1 horizontal position
constexpr uint8_t HPOSM2 = 0x06;  // Missile 2 horizontal position
constexpr uint8_t HPOSM3 = 0x07;  // Missile 3 horizontal position

// Write registers - Size
constexpr uint8_t SIZEP0 = 0x08;  // Player 0 size
constexpr uint8_t SIZEP1 = 0x09;  // Player 1 size
constexpr uint8_t SIZEP2 = 0x0A;  // Player 2 size
constexpr uint8_t SIZEP3 = 0x0B;  // Player 3 size
constexpr uint8_t SIZEM = 0x0C;   // All missiles size

// Write registers - Graphics data
constexpr uint8_t GRAFP0 = 0x0D;  // Player 0 graphics
constexpr uint8_t GRAFP1 = 0x0E;  // Player 1 graphics
constexpr uint8_t GRAFP2 = 0x0F;  // Player 2 graphics
constexpr uint8_t GRAFP3 = 0x10;  // Player 3 graphics
constexpr uint8_t GRAFM = 0x11;   // All missiles graphics

// Write registers - Colors
constexpr uint8_t COLPM0 = 0x12;  // Player/missile 0 color
constexpr uint8_t COLPM1 = 0x13;  // Player/missile 1 color
constexpr uint8_t COLPM2 = 0x14;  // Player/missile 2 color
constexpr uint8_t COLPM3 = 0x15;  // Player/missile 3 color
constexpr uint8_t COLPF0 = 0x16;  // Playfield 0 color
constexpr uint8_t COLPF1 = 0x17;  // Playfield 1 color
constexpr uint8_t COLPF2 = 0x18;  // Playfield 2 color
constexpr uint8_t COLPF3 = 0x19;  // Playfield 3 color
constexpr uint8_t COLBK = 0x1A;   // Background color

// Write registers - Control
constexpr uint8_t PRIOR = 0x1B;   // Priority selection
constexpr uint8_t VDELAY = 0x1C;  // Vertical delay
constexpr uint8_t GRACTL = 0x1D;  // Graphics control
constexpr uint8_t HITCLR = 0x1E;  // Clear collision registers
constexpr uint8_t CONSOL_W = 0x1F; // Console switches (active-low output)

// Read registers - Collision detection (active-high)
constexpr uint8_t M0PF = 0x00;    // Missile 0 to playfield collision
constexpr uint8_t M1PF = 0x01;    // Missile 1 to playfield collision
constexpr uint8_t M2PF = 0x02;    // Missile 2 to playfield collision
constexpr uint8_t M3PF = 0x03;    // Missile 3 to playfield collision
constexpr uint8_t P0PF = 0x04;    // Player 0 to playfield collision
constexpr uint8_t P1PF = 0x05;    // Player 1 to playfield collision
constexpr uint8_t P2PF = 0x06;    // Player 2 to playfield collision
constexpr uint8_t P3PF = 0x07;    // Player 3 to playfield collision
constexpr uint8_t M0PL = 0x08;    // Missile 0 to player collision
constexpr uint8_t M1PL = 0x09;    // Missile 1 to player collision
constexpr uint8_t M2PL = 0x0A;    // Missile 2 to player collision
constexpr uint8_t M3PL = 0x0B;    // Missile 3 to player collision
constexpr uint8_t P0PL = 0x0C;    // Player 0 to player collision
constexpr uint8_t P1PL = 0x0D;    // Player 1 to player collision
constexpr uint8_t P2PL = 0x0E;    // Player 2 to player collision
constexpr uint8_t P3PL = 0x0F;    // Player 3 to player collision
constexpr uint8_t TRIG0 = 0x10;   // Joystick 0 trigger
constexpr uint8_t TRIG1 = 0x11;   // Joystick 1 trigger
constexpr uint8_t TRIG2 = 0x12;   // Joystick 2 trigger
constexpr uint8_t TRIG3 = 0x13;   // Joystick 3 trigger
constexpr uint8_t PAL = 0x14;     // PAL/NTSC flag (PAL=1, NTSC=0 in bits 1-3)
constexpr uint8_t CONSOL_R = 0x1F; // Console switches (active-low input)

// PRIOR register bits
constexpr uint8_t PRIOR_P0_ABOVE = 0x01;   // Player 0-1 above playfield
constexpr uint8_t PRIOR_P2_ABOVE = 0x02;   // Player 2-3 above playfield
constexpr uint8_t PRIOR_PF_ABOVE = 0x04;   // Playfield above players
constexpr uint8_t PRIOR_PF01_P23 = 0x08;   // PF0-1 above players 2-3
constexpr uint8_t PRIOR_5TH_PLAYER = 0x10; // Enable 5th player (missiles as 5th player)
constexpr uint8_t PRIOR_MULTICOLOR = 0x20; // Multicolor players
constexpr uint8_t PRIOR_GTIA_MODE = 0xC0;  // GTIA mode selection (GR.9/10/11)

// GRACTL bits
constexpr uint8_t GRACTL_MISSILE = 0x01;   // Enable missile DMA
constexpr uint8_t GRACTL_PLAYER = 0x02;    // Enable player DMA
constexpr uint8_t GRACTL_LATCH = 0x04;     // Latch trigger inputs

// Player/Missile sizes
constexpr uint8_t PM_SIZE_NORMAL = 0x00;
constexpr uint8_t PM_SIZE_DOUBLE = 0x01;
constexpr uint8_t PM_SIZE_QUAD = 0x03;

/**
 * @brief GTIA - Graphics Television Interface Adapter
 *
 * The GTIA chip handles:
 * - Color registers (9 colors: 4 player/missile, 4 playfield, 1 background)
 * - Player/Missile graphics positioning and sizing
 * - Collision detection between all graphics objects
 * - Console switches (START, SELECT, OPTION)
 * - Joystick trigger inputs
 * - Priority selection between graphics layers
 * - Special GTIA graphics modes (GR.9, GR.10, GR.11)
 */
class GTIA {
private:
  // Horizontal positions (0-255, visible range ~48-208)
  uint8_t hposp[4];    // Player horizontal positions
  uint8_t hposm[4];    // Missile horizontal positions

  // Sizes
  uint8_t sizep[4];    // Player sizes
  uint8_t sizem;       // Missile sizes (2 bits each)

  // Graphics data (software written)
  uint8_t grafp[4];    // Player graphics
  uint8_t grafm;       // Missile graphics

  // Color registers
  uint8_t colpm[4];    // Player/missile colors
  uint8_t colpf[4];    // Playfield colors
  uint8_t colbk;       // Background color

  // Control registers
  uint8_t prior;       // Priority control
  uint8_t vdelay;      // Vertical delay
  uint8_t gractl;      // Graphics control

  // Collision registers (active-high)
  uint8_t m2pf[4];     // Missile to playfield
  uint8_t p2pf[4];     // Player to playfield
  uint8_t m2pl[4];     // Missile to player
  uint8_t p2pl[4];     // Player to player

  // Trigger inputs (active-low: 0=pressed)
  uint8_t trig[4];

  // Console switches (active-low: 0=pressed)
  uint8_t consol;      // Bits: 0=START, 1=SELECT, 2=OPTION

  // PAL/NTSC flag
  bool isPAL;

public:
  GTIA();
  void reset();

  // Register access
  uint8_t read(uint8_t addr);
  void write(uint8_t addr, uint8_t val);

  // Color accessors for ANTIC
  uint8_t getBackgroundColor() const { return colbk; }
  uint8_t getPlayfieldColor(uint8_t index) const {
    return (index < 4) ? colpf[index] : 0;
  }
  uint8_t getPlayerColor(uint8_t index) const {
    return (index < 4) ? colpm[index] : 0;
  }

  // Player/Missile accessors
  uint8_t getPlayerHPos(uint8_t index) const {
    return (index < 4) ? hposp[index] : 0;
  }
  uint8_t getMissileHPos(uint8_t index) const {
    return (index < 4) ? hposm[index] : 0;
  }
  uint8_t getPlayerSize(uint8_t index) const {
    return (index < 4) ? sizep[index] : 0;
  }
  uint8_t getMissileSize(uint8_t index) const {
    return (sizem >> (index * 2)) & 0x03;
  }
  uint8_t getPlayerGraphics(uint8_t index) const {
    return (index < 4) ? grafp[index] : 0;
  }
  uint8_t getMissileGraphics() const { return grafm; }

  // Priority and mode
  uint8_t getPrior() const { return prior; }
  uint8_t getGTIAMode() const { return (prior >> 6) & 0x03; }

  // Collision handling
  void setCollision(uint8_t type, uint8_t obj, uint8_t bits);
  void clearCollisions();

  // Input handling
  void setTrigger(uint8_t index, bool pressed);
  void setConsoleKey(uint8_t key, bool pressed);

  // Configuration
  void setPAL(bool pal) { isPAL = pal; }
};

#endif // GTIA_H
