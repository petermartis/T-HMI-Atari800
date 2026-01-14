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
#ifndef ANTIC_H
#define ANTIC_H

#include "display/AtariDisplayDriver.h"
#include <atomic>
#include <cstdint>

// Forward declaration
class GTIA;

// ANTIC register addresses (offset from base $D400)
// Write registers
constexpr uint8_t DMACTL = 0x00;   // DMA control
constexpr uint8_t CHACTL = 0x01;   // Character control
constexpr uint8_t DLISTL = 0x02;   // Display list low byte
constexpr uint8_t DLISTH = 0x03;   // Display list high byte
constexpr uint8_t HSCROL = 0x04;   // Horizontal scroll
constexpr uint8_t VSCROL = 0x05;   // Vertical scroll
constexpr uint8_t PMBASE = 0x07;   // Player/missile base address
constexpr uint8_t CHBASE = 0x09;   // Character base address
constexpr uint8_t WSYNC = 0x0A;    // Wait for horizontal sync
constexpr uint8_t VCOUNT_W = 0x0B; // Vertical line counter (write)
constexpr uint8_t PENH = 0x0C;     // Light pen horizontal
constexpr uint8_t PENV = 0x0D;     // Light pen vertical
constexpr uint8_t NMIEN = 0x0E;    // NMI enable
constexpr uint8_t NMIRES = 0x0F;   // NMI reset

// Read registers
constexpr uint8_t VCOUNT_R = 0x0B; // Vertical line counter (read)
constexpr uint8_t NMIST = 0x0F;    // NMI status

// DMACTL bits
constexpr uint8_t DMACTL_NARROW = 0x01;    // Narrow playfield (128 color clocks)
constexpr uint8_t DMACTL_STANDARD = 0x02;  // Standard playfield (160 color clocks)
constexpr uint8_t DMACTL_WIDE = 0x03;      // Wide playfield (192 color clocks)
constexpr uint8_t DMACTL_PLAYFIELD = 0x03; // Playfield mask
constexpr uint8_t DMACTL_MISSILE = 0x04;   // Enable missile DMA
constexpr uint8_t DMACTL_PLAYER = 0x08;    // Enable player DMA
constexpr uint8_t DMACTL_PM_1LINE = 0x10;  // Single line P/M resolution
constexpr uint8_t DMACTL_DL = 0x20;        // Enable display list DMA

// CHACTL bits
constexpr uint8_t CHACTL_INVERT = 0x02;    // Invert characters
constexpr uint8_t CHACTL_REFLECT = 0x04;   // Reflect characters vertically

// NMIEN/NMIST bits
constexpr uint8_t NMI_DLI = 0x80;          // Display list interrupt
constexpr uint8_t NMI_VBI = 0x40;          // Vertical blank interrupt
constexpr uint8_t NMI_RESET = 0x20;        // Reset button

// ANTIC display modes (mode line instructions)
constexpr uint8_t MODE_BLANK_1 = 0x00;  // 1 blank line
constexpr uint8_t MODE_BLANK_2 = 0x10;  // 2 blank lines
constexpr uint8_t MODE_BLANK_3 = 0x20;  // 3 blank lines
constexpr uint8_t MODE_BLANK_4 = 0x30;  // 4 blank lines
constexpr uint8_t MODE_BLANK_5 = 0x40;  // 5 blank lines
constexpr uint8_t MODE_BLANK_6 = 0x50;  // 6 blank lines
constexpr uint8_t MODE_BLANK_7 = 0x60;  // 7 blank lines
constexpr uint8_t MODE_BLANK_8 = 0x70;  // 8 blank lines

constexpr uint8_t MODE_JMP = 0x01;      // Jump (load new display list address)
constexpr uint8_t MODE_JVB = 0x41;      // Jump and wait for vertical blank

constexpr uint8_t MODE_2 = 0x02;        // 40 chars, 8 scanlines, 1.5 colors
constexpr uint8_t MODE_3 = 0x03;        // 40 chars, 10 scanlines, 1.5 colors
constexpr uint8_t MODE_4 = 0x04;        // 40 chars, 8 scanlines, 4 colors
constexpr uint8_t MODE_5 = 0x05;        // 40 chars, 16 scanlines, 4 colors
constexpr uint8_t MODE_6 = 0x06;        // 20 chars, 8 scanlines, 5 colors
constexpr uint8_t MODE_7 = 0x07;        // 20 chars, 16 scanlines, 5 colors
constexpr uint8_t MODE_8 = 0x08;        // 40 pixels, 8 scanlines, 4 colors (map)
constexpr uint8_t MODE_9 = 0x09;        // 80 pixels, 4 scanlines, 2 colors (map)
constexpr uint8_t MODE_A = 0x0A;        // 80 pixels, 4 scanlines, 4 colors (map)
constexpr uint8_t MODE_B = 0x0B;        // 160 pixels, 2 scanlines, 2 colors (map)
constexpr uint8_t MODE_C = 0x0C;        // 160 pixels, 1 scanline, 2 colors (map)
constexpr uint8_t MODE_D = 0x0D;        // 160 pixels, 2 scanlines, 4 colors (map)
constexpr uint8_t MODE_E = 0x0E;        // 160 pixels, 1 scanline, 4 colors (map)
constexpr uint8_t MODE_F = 0x0F;        // 320 pixels, 1 scanline, 1.5 colors (hires)

// Mode line modifier bits
constexpr uint8_t MODE_LMS = 0x40;      // Load memory scan counter
constexpr uint8_t MODE_DLI = 0x80;      // Display list interrupt
constexpr uint8_t MODE_HSCROL = 0x10;   // Enable horizontal scroll
constexpr uint8_t MODE_VSCROL = 0x20;   // Enable vertical scroll

// Screen dimensions
constexpr uint16_t ATARI_WIDTH = 320;   // Standard playfield width in pixels
constexpr uint16_t ATARI_HEIGHT = 192;  // Standard playfield height in scanlines
constexpr uint16_t SCANLINES_PAL = 312;
constexpr uint16_t SCANLINES_NTSC = 262;
constexpr uint16_t VBLANK_START = 248;

/**
 * @brief ANTIC - Alphanumeric Television Interface Controller
 *
 * The ANTIC chip is the heart of the Atari display system. It acts as
 * a coprocessor that reads display list instructions from memory and
 * generates the video signal. Key features:
 *
 * - Display list based rendering (similar to modern display lists)
 * - 14 different graphics modes (character and bitmap)
 * - Hardware scrolling (fine and coarse)
 * - Player/Missile graphics DMA
 * - NMI generation (DLI, VBI)
 */
class ANTIC {
private:
  uint8_t *ram;
  uint16_t *bitmap;            // Output bitmap (ATARI_WIDTH x ATARI_HEIGHT)
  AtariDisplayDriver *display;
  GTIA *gtia;

  // ANTIC registers
  uint8_t dmactl;              // DMA control register
  uint8_t chactl;              // Character control register
  uint16_t dlist;              // Display list pointer
  uint8_t hscrol;              // Horizontal scroll value
  uint8_t vscrol;              // Vertical scroll value
  uint8_t pmbase;              // Player/Missile base address (high byte)
  uint8_t chbase;              // Character base address (high byte)
  uint8_t nmien;               // NMI enable register
  uint8_t nmist;               // NMI status register

  // Internal state
  uint16_t scanline;           // Current scanline
  uint16_t displayListPC;      // Current display list program counter
  uint16_t memScan;            // Memory scan counter
  uint8_t modeLineCount;       // Lines remaining in current mode
  uint8_t currentMode;         // Current display mode
  bool inDisplayList;          // Currently processing display list
  bool dliPending;             // DLI requested
  bool vbiPending;             // VBI requested
  bool wsyncHalt;              // CPU halted waiting for WSYNC

  // Mode line state
  uint8_t rowInMode;           // Current row within mode line
  uint8_t scanLinesPerMode;    // Number of scanlines for current mode
  bool isCharMode;             // Character mode vs bitmap mode
  uint8_t bytesPerLine;        // Bytes of screen data per line
  uint8_t pixelsPerByte;       // Pixels per byte of data

  // Scroll state
  bool hscrolEnabled;
  bool vscrolEnabled;
  uint8_t vscrolLines;         // Lines scrolled vertically

  // Drawing functions
  void drawBlankLine();
  void drawModeLine();
  void drawCharacterMode2();   // ANTIC mode 2 (BASIC GR.0)
  void drawCharacterMode3();   // ANTIC mode 3
  void drawCharacterMode4();   // ANTIC mode 4 (multicolor text)
  void drawCharacterMode5();   // ANTIC mode 5
  void drawCharacterMode6();   // ANTIC mode 6 (20 col, 5 colors)
  void drawCharacterMode7();   // ANTIC mode 7
  void drawBitmapMode8();      // ANTIC mode 8 (GR.3)
  void drawBitmapMode9();      // ANTIC mode 9 (GR.4)
  void drawBitmapModeA();      // ANTIC mode A (GR.5)
  void drawBitmapModeB();      // ANTIC mode B (GR.6)
  void drawBitmapModeC();      // ANTIC mode C
  void drawBitmapModeD();      // ANTIC mode D (GR.7)
  void drawBitmapModeE();      // ANTIC mode E (GR.15)
  void drawBitmapModeF();      // ANTIC mode F (GR.8 hires)

  void processDisplayList();
  uint8_t fetchDisplayListByte();
  void setModeLineParams(uint8_t mode);

public:
  // Profiling info
  std::atomic<uint8_t> cntRefreshs;

  // DMA cycle stealing (for CPU timing)
  uint8_t dmaCycles;

  ANTIC();
  void init(uint8_t *ram, GTIA *gtia);
  void reset();

  // Register access
  uint8_t read(uint8_t addr);
  void write(uint8_t addr, uint8_t val);

  // Frame rendering
  uint8_t nextScanline();      // Advance to next scanline, return DMA cycles
  void drawScanline();         // Draw current scanline
  void refresh();              // Refresh display (send bitmap to LCD)

  // Interrupt interface
  bool checkDLI();             // Check for DLI
  bool checkVBI();             // Check for VBI
  void clearNMI(uint8_t mask);

  // WSYNC support
  bool isWSYNCHalted() const { return wsyncHalt; }
  void releaseWSYNC() { wsyncHalt = false; }

  // Accessors
  uint16_t getScanline() const { return scanline; }
  uint8_t *getBitmap() { return (uint8_t *)bitmap; }
  uint16_t *getBitmap16() { return bitmap; }
};

#endif // ANTIC_H
