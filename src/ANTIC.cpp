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
#include "ANTIC.h"
#include "GTIA.h"
#include "display/DisplayFactory.h"
#include <cstring>

// Mode line parameters: scanlines, bytes per line, characters/pixels
static const struct {
  uint8_t scanlines;
  uint8_t bytesPerLine;
  bool isChar;
} modeParams[16] = {
    {1, 0, false},   // 0: blank line instructions handled separately
    {1, 0, false},   // 1: jump instruction
    {8, 40, true},   // 2: 40 chars, 8 scanlines (GR.0)
    {10, 40, true},  // 3: 40 chars, 10 scanlines
    {8, 40, true},   // 4: 40 chars, 8 scanlines, multicolor
    {16, 40, true},  // 5: 40 chars, 16 scanlines, multicolor
    {8, 20, true},   // 6: 20 chars, 8 scanlines, 5 colors
    {16, 20, true},  // 7: 20 chars, 16 scanlines, 5 colors
    {8, 10, false},  // 8: 40 pixels, 8 scanlines (GR.3)
    {4, 10, false},  // 9: 80 pixels, 4 scanlines (GR.4)
    {4, 20, false},  // A: 80 pixels, 4 scanlines, 4 colors (GR.5)
    {2, 20, false},  // B: 160 pixels, 2 scanlines (GR.6)
    {1, 20, false},  // C: 160 pixels, 1 scanline
    {2, 40, false},  // D: 160 pixels, 2 scanlines, 4 colors (GR.7)
    {1, 40, false},  // E: 160 pixels, 1 scanline, 4 colors (GR.15)
    {1, 40, false},  // F: 320 pixels, 1 scanline, hires (GR.8)
};

ANTIC::ANTIC() : ram(nullptr), bitmap(nullptr), display(nullptr), gtia(nullptr) {
  cntRefreshs = 0;
  reset();
}

void ANTIC::init(uint8_t *ram, GTIA *gtia) {
  this->ram = ram;
  this->gtia = gtia;

  // Allocate bitmap for ATARI_WIDTH x ATARI_HEIGHT pixels (16-bit RGB565)
  bitmap = new uint16_t[ATARI_WIDTH * ATARI_HEIGHT];
  memset(bitmap, 0, ATARI_WIDTH * ATARI_HEIGHT * sizeof(uint16_t));

  // Create display driver
  display = (AtariDisplayDriver *)DisplayFactory::create();
  if (display) {
    display->init();
  }

  reset();
}

void ANTIC::reset() {
  dmactl = 0;
  chactl = 0;
  dlist = 0;
  hscrol = 0;
  vscrol = 0;
  pmbase = 0;
  chbase = 0;
  nmien = 0;
  nmist = 0x1F;  // No NMIs pending

  scanline = 0;
  displayListPC = 0;
  memScan = 0;
  modeLineCount = 0;
  currentMode = 0;
  inDisplayList = false;
  dliPending = false;
  vbiPending = false;
  wsyncHalt = false;

  rowInMode = 0;
  scanLinesPerMode = 0;
  isCharMode = false;
  bytesPerLine = 0;
  pixelsPerByte = 1;

  hscrolEnabled = false;
  vscrolEnabled = false;
  vscrolLines = 0;

  dmaCycles = 0;
}

uint8_t ANTIC::read(uint8_t addr) {
  addr &= 0x0F;

  switch (addr) {
  case VCOUNT_R:
    // Return scanline / 2 (0-155 for NTSC, 0-155 for PAL visible)
    return (scanline >> 1) & 0xFF;

  case PENH:
    return 0x00; // Light pen horizontal (not implemented)

  case PENV:
    return 0x00; // Light pen vertical (not implemented)

  case NMIST:
    return nmist;

  default:
    return 0xFF;
  }
}

void ANTIC::write(uint8_t addr, uint8_t val) {
  addr &= 0x0F;

  switch (addr) {
  case DMACTL:
    dmactl = val;
    break;

  case CHACTL:
    chactl = val;
    break;

  case DLISTL:
    dlist = (dlist & 0xFF00) | val;
    break;

  case DLISTH:
    dlist = (dlist & 0x00FF) | (val << 8);
    break;

  case HSCROL:
    hscrol = val & 0x0F;
    break;

  case VSCROL:
    vscrol = val & 0x0F;
    break;

  case PMBASE:
    pmbase = val;
    break;

  case CHBASE:
    chbase = val;
    break;

  case WSYNC:
    // Halt CPU until end of scanline
    wsyncHalt = true;
    break;

  case NMIEN:
    nmien = val;
    break;

  case NMIRES:
    // Clear NMI status bits
    nmist = 0x1F;
    dliPending = false;
    vbiPending = false;
    break;
  }
}

uint8_t ANTIC::fetchDisplayListByte() {
  if (!(dmactl & DMACTL_DL)) {
    return 0;
  }
  uint8_t byte = ram[displayListPC & 0xFFFF];
  displayListPC++;
  dmaCycles++;
  return byte;
}

void ANTIC::setModeLineParams(uint8_t mode) {
  uint8_t modeIdx = mode & 0x0F;
  scanLinesPerMode = modeParams[modeIdx].scanlines;
  bytesPerLine = modeParams[modeIdx].bytesPerLine;
  isCharMode = modeParams[modeIdx].isChar;
  rowInMode = 0;
}

void ANTIC::processDisplayList() {
  if (!(dmactl & DMACTL_DL)) {
    return;
  }

  // Only fetch new instruction at start of mode line
  if (modeLineCount == 0) {
    uint8_t instruction = fetchDisplayListByte();

    // Check for DLI on this instruction
    if (instruction & MODE_DLI) {
      if (nmien & NMI_DLI) {
        dliPending = true;
        nmist &= ~NMI_DLI;
      }
    }

    hscrolEnabled = (instruction & MODE_HSCROL) != 0;
    vscrolEnabled = (instruction & MODE_VSCROL) != 0;

    uint8_t mode = instruction & 0x0F;

    // Handle blank lines (modes 0x00, 0x10, 0x20, etc.)
    if ((instruction & 0x0F) == 0 && instruction != 0) {
      // Blank line instruction
      modeLineCount = ((instruction >> 4) & 0x07) + 1;
      currentMode = 0;
      return;
    }

    // Handle jump instructions
    if (mode == 0x01) {
      // JMP or JVB
      uint8_t lo = fetchDisplayListByte();
      uint8_t hi = fetchDisplayListByte();
      displayListPC = lo | (hi << 8);

      if (instruction == MODE_JVB) {
        // Jump and wait for vertical blank
        inDisplayList = false;
        // Trigger VBI if enabled
        if (nmien & NMI_VBI) {
          vbiPending = true;
          nmist &= ~NMI_VBI;
        }
      }
      return;
    }

    // Set up mode line parameters
    setModeLineParams(mode);
    currentMode = mode;
    modeLineCount = scanLinesPerMode;

    // Check for LMS (Load Memory Scan)
    if (instruction & MODE_LMS) {
      uint8_t lo = fetchDisplayListByte();
      uint8_t hi = fetchDisplayListByte();
      memScan = lo | (hi << 8);
    }
  }
}

void ANTIC::drawBlankLine() {
  // Fill scanline with background color from GTIA
  uint16_t bgColor = display->colorToRGB565(gtia->getBackgroundColor());
  uint16_t *line = &bitmap[scanline * ATARI_WIDTH];
  for (int x = 0; x < ATARI_WIDTH; x++) {
    line[x] = bgColor;
  }
}

void ANTIC::drawCharacterMode2() {
  // ANTIC Mode 2: 40 characters, 8 scanlines per char, 2 colors
  // This is BASIC GR.0 (standard text mode)
  const uint16_t *colors = display->getAtariColors();
  uint8_t bgColor = gtia->getBackgroundColor();
  uint8_t fgColor = gtia->getPlayfieldColor(0) | (gtia->getPlayfieldColor(0) & 0x0F);

  uint16_t bgRGB = colors[bgColor];
  uint16_t fgRGB = colors[fgColor];

  uint16_t charBase = chbase << 8;
  uint16_t *line = &bitmap[scanline * ATARI_WIDTH];
  uint8_t charRow = rowInMode;

  // Apply character control
  bool invert = (chactl & CHACTL_INVERT) != 0;
  bool reflect = (chactl & CHACTL_REFLECT) != 0;

  if (reflect) {
    charRow = 7 - charRow;
  }

  int xpos = 0;
  for (int col = 0; col < 40 && xpos < ATARI_WIDTH; col++) {
    uint8_t charCode = ram[(memScan + col) & 0xFFFF];
    bool charInvert = (charCode & 0x80) != 0;
    charCode &= 0x7F;

    uint8_t charData = ram[(charBase + charCode * 8 + charRow) & 0xFFFF];

    // Apply inversion
    if (invert ^ charInvert) {
      charData ^= 0xFF;
    }

    // Draw 8 pixels
    for (int bit = 7; bit >= 0 && xpos < ATARI_WIDTH; bit--) {
      if (charData & (1 << bit)) {
        line[xpos] = fgRGB;
      } else {
        line[xpos] = bgRGB;
      }
      xpos++;
    }
  }
}

void ANTIC::drawCharacterMode4() {
  // ANTIC Mode 4: 40 characters, 8 scanlines, 4 colors
  const uint16_t *colors = display->getAtariColors();
  uint8_t colorRegs[4] = {
      gtia->getBackgroundColor(),
      gtia->getPlayfieldColor(0),
      gtia->getPlayfieldColor(1),
      gtia->getPlayfieldColor(2)};

  uint16_t charBase = chbase << 8;
  uint16_t *line = &bitmap[scanline * ATARI_WIDTH];
  uint8_t charRow = rowInMode;

  int xpos = 0;
  for (int col = 0; col < 40 && xpos < ATARI_WIDTH; col++) {
    uint8_t charCode = ram[(memScan + col) & 0xFFFF];
    uint8_t charData = ram[(charBase + (charCode & 0x7F) * 8 + charRow) & 0xFFFF];

    // Color selection based on high bits of character code
    uint8_t colorPair = (charCode >> 6) & 0x03;

    // Draw 8 pixels (4 color clocks, 2 bits each)
    for (int i = 0; i < 4 && xpos < ATARI_WIDTH; i++) {
      uint8_t pixelBits = (charData >> (6 - i * 2)) & 0x03;
      uint16_t color = colors[colorRegs[pixelBits]];
      line[xpos++] = color;
      line[xpos++] = color;
    }
  }
}

void ANTIC::drawCharacterMode6() {
  // ANTIC Mode 6: 20 characters, 8 scanlines, 5 colors
  const uint16_t *colors = display->getAtariColors();

  uint16_t charBase = chbase << 8;
  uint16_t *line = &bitmap[scanline * ATARI_WIDTH];
  uint8_t charRow = rowInMode;

  int xpos = 0;
  for (int col = 0; col < 20 && xpos < ATARI_WIDTH; col++) {
    uint8_t charCode = ram[(memScan + col) & 0xFFFF];
    uint8_t charData = ram[(charBase + (charCode & 0x3F) * 8 + charRow) & 0xFFFF];

    // Color selection based on bits 6-7
    uint8_t colorSelect = (charCode >> 6) & 0x03;
    uint8_t fgColor;
    switch (colorSelect) {
    case 0:
      fgColor = gtia->getPlayfieldColor(0);
      break;
    case 1:
      fgColor = gtia->getPlayfieldColor(1);
      break;
    case 2:
      fgColor = gtia->getPlayfieldColor(2);
      break;
    default:
      fgColor = gtia->getPlayfieldColor(3);
      break;
    }

    uint16_t bgRGB = colors[gtia->getBackgroundColor()];
    uint16_t fgRGB = colors[fgColor];

    // Draw 16 pixels (each character is double-width)
    for (int bit = 7; bit >= 0 && xpos < ATARI_WIDTH; bit--) {
      uint16_t pixel = (charData & (1 << bit)) ? fgRGB : bgRGB;
      line[xpos++] = pixel;
      line[xpos++] = pixel;
    }
  }
}

void ANTIC::drawBitmapModeD() {
  // ANTIC Mode D: 160x2, 4 colors (GR.7)
  const uint16_t *colors = display->getAtariColors();
  uint8_t colorRegs[4] = {
      gtia->getBackgroundColor(),
      gtia->getPlayfieldColor(0),
      gtia->getPlayfieldColor(1),
      gtia->getPlayfieldColor(2)};

  uint16_t *line = &bitmap[scanline * ATARI_WIDTH];

  int xpos = 0;
  for (int byte = 0; byte < 40 && xpos < ATARI_WIDTH; byte++) {
    uint8_t data = ram[(memScan + byte) & 0xFFFF];

    // Each byte contains 4 pixels (2 bits each)
    for (int pixel = 0; pixel < 4 && xpos < ATARI_WIDTH; pixel++) {
      uint8_t colorIdx = (data >> (6 - pixel * 2)) & 0x03;
      uint16_t color = colors[colorRegs[colorIdx]];
      // Double-width pixels
      line[xpos++] = color;
      line[xpos++] = color;
    }
  }
}

void ANTIC::drawBitmapModeE() {
  // ANTIC Mode E: 160x1, 4 colors (GR.15)
  const uint16_t *colors = display->getAtariColors();
  uint8_t colorRegs[4] = {
      gtia->getBackgroundColor(),
      gtia->getPlayfieldColor(0),
      gtia->getPlayfieldColor(1),
      gtia->getPlayfieldColor(2)};

  uint16_t *line = &bitmap[scanline * ATARI_WIDTH];

  int xpos = 0;
  for (int byte = 0; byte < 40 && xpos < ATARI_WIDTH; byte++) {
    uint8_t data = ram[(memScan + byte) & 0xFFFF];

    // Each byte contains 4 pixels (2 bits each)
    for (int pixel = 0; pixel < 4 && xpos < ATARI_WIDTH; pixel++) {
      uint8_t colorIdx = (data >> (6 - pixel * 2)) & 0x03;
      uint16_t color = colors[colorRegs[colorIdx]];
      // Double-width pixels
      line[xpos++] = color;
      line[xpos++] = color;
    }
  }
}

void ANTIC::drawBitmapModeF() {
  // ANTIC Mode F: 320x1, 2 colors (GR.8 hires)
  const uint16_t *colors = display->getAtariColors();
  uint16_t bgRGB = colors[gtia->getBackgroundColor()];
  uint16_t fgRGB = colors[gtia->getPlayfieldColor(0) | (gtia->getPlayfieldColor(0) & 0x0F)];

  uint16_t *line = &bitmap[scanline * ATARI_WIDTH];

  int xpos = 0;
  for (int byte = 0; byte < 40 && xpos < ATARI_WIDTH; byte++) {
    uint8_t data = ram[(memScan + byte) & 0xFFFF];

    // Each byte contains 8 pixels
    for (int bit = 7; bit >= 0 && xpos < ATARI_WIDTH; bit--) {
      line[xpos++] = (data & (1 << bit)) ? fgRGB : bgRGB;
    }
  }
}

void ANTIC::drawModeLine() {
  switch (currentMode) {
  case 0:
    drawBlankLine();
    break;
  case 2:
    drawCharacterMode2();
    break;
  case 3:
    drawCharacterMode2(); // Similar to mode 2 but taller
    break;
  case 4:
    drawCharacterMode4();
    break;
  case 5:
    drawCharacterMode4(); // Similar to mode 4 but taller
    break;
  case 6:
    drawCharacterMode6();
    break;
  case 7:
    drawCharacterMode6(); // Similar to mode 6 but taller
    break;
  case 8:
  case 9:
  case 0x0A:
  case 0x0B:
  case 0x0C:
    drawBitmapModeD(); // Simplified - use mode D rendering
    break;
  case 0x0D:
    drawBitmapModeD();
    break;
  case 0x0E:
    drawBitmapModeE();
    break;
  case 0x0F:
    drawBitmapModeF();
    break;
  default:
    drawBlankLine();
    break;
  }
}

void ANTIC::drawScanline() {
  if (scanline < 8 || scanline >= VBLANK_START) {
    // Vertical blank area - draw blank
    drawBlankLine();
    return;
  }

  if (!(dmactl & DMACTL_PLAYFIELD)) {
    // DMA disabled - blank line
    drawBlankLine();
    return;
  }

  // Process display list if needed
  if (inDisplayList) {
    processDisplayList();
  }

  // Draw the current mode line
  if (modeLineCount > 0) {
    drawModeLine();

    rowInMode++;
    modeLineCount--;

    // Advance memory scan at end of character row (for char modes)
    if (isCharMode && rowInMode >= scanLinesPerMode) {
      memScan += bytesPerLine;
    }
    // For bitmap modes, advance memory scan every line
    if (!isCharMode) {
      memScan += bytesPerLine;
    }
  } else {
    drawBlankLine();
  }
}

uint8_t ANTIC::nextScanline() {
  dmaCycles = 0;

  scanline++;

  if (scanline >= SCANLINES_PAL) {
    // End of frame
    scanline = 0;
    displayListPC = dlist;
    inDisplayList = true;
    modeLineCount = 0;
    rowInMode = 0;

    // Trigger VBI at start of vertical blank
    if (nmien & NMI_VBI) {
      vbiPending = true;
      nmist &= ~NMI_VBI;
    }
  }

  // Release WSYNC halt at end of scanline
  wsyncHalt = false;

  return dmaCycles;
}

void ANTIC::refresh() {
  if (display) {
    display->drawBitmap(bitmap);
    // Draw border with background color
    display->drawFrame(gtia->getBackgroundColor());
  }
  cntRefreshs++;
}

bool ANTIC::checkDLI() {
  if (dliPending) {
    dliPending = false;
    return true;
  }
  return false;
}

bool ANTIC::checkVBI() {
  if (vbiPending) {
    vbiPending = false;
    return true;
  }
  return false;
}

void ANTIC::clearNMI(uint8_t mask) { nmist |= mask; }
