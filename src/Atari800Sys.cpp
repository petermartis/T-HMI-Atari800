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
#include "Atari800Sys.h"
#include "platform/PlatformManager.h"

// Cycles per scanline (PAL: 114 cycles at 1.77MHz)
constexpr int32_t CYCLES_PER_SCANLINE = 114;
constexpr int32_t SCANLINES_PER_FRAME = 312;  // PAL

Atari800Sys::Atari800Sys()
    : ram(nullptr), osRom(nullptr), basicRom(nullptr), charRom(nullptr),
      joystick(nullptr), keyboard(nullptr) {
  osRomEnabled = true;
  basicRomEnabled = true;
  selfTestEnabled = false;
  nmiActive = false;
  lastIRQ = 0;
  cyclesThisScanline = 0;
  cyclesPerScanline = CYCLES_PER_SCANLINE;
  numofcyclespersecond = 0;
  perf = false;
}

Atari800Sys::~Atari800Sys() {
  // RAM is managed externally
}

void Atari800Sys::init(uint8_t *ram, const uint8_t *osRom, const uint8_t *basicRom) {
  this->ram = ram;
  this->osRom = osRom;
  this->basicRom = basicRom;

  // Character ROM is built into OS ROM at offset $E000 (relative to $C000)
  // In the actual OS ROM it's at $E000-$E3FF
  this->charRom = osRom + 0x2000;  // $E000 - $C000 = $2000

  // Initialize chips
  antic.init(ram, &gtia);
  pokey.init();
  gtia.reset();
  pia.reset();

  reset();
}

void Atari800Sys::reset() {
  // Reset CPU
  cpuhalted = false;
  numofcycles = 0;

  // Initialize flags
  cflag = false;
  zflag = false;
  dflag = false;
  bflag = false;
  vflag = false;
  nflag = false;
  iflag = true;  // Interrupts disabled on reset

  // Initialize registers
  a = 0;
  x = 0;
  y = 0;
  sp = 0xFF;

  // Reset chips
  antic.reset();
  gtia.reset();
  pokey.reset();
  pia.reset();

  // Enable ROMs
  osRomEnabled = true;
  basicRomEnabled = true;
  selfTestEnabled = false;

  // Read reset vector from OS ROM
  pc = osRom[0x3FFC - 0x0000] | (osRom[0x3FFD - 0x0000] << 8);

  nmiActive = false;
  cyclesThisScanline = 0;
}

void Atari800Sys::updateBanking() {
  uint8_t portb = pia.getPortB();
  osRomEnabled = (portb & PORTB_OS_ROM) == 0;
  basicRomEnabled = (portb & PORTB_BASIC) == 0;
  selfTestEnabled = (portb & PORTB_SELFTEST) == 0;
}

uint8_t Atari800Sys::getMem(uint16_t addr) {
  // RAM area ($0000-$BFFF with holes for ROM)
  if (addr < 0xA000) {
    // Check for self-test ROM area ($5000-$57FF)
    if (selfTestEnabled && addr >= 0x5000 && addr < 0x5800) {
      return osRom[addr - 0x5000 + 0x1000];  // Self-test at $D000 in OS ROM
    }
    return ram[addr];
  }

  // BASIC ROM area ($A000-$BFFF)
  if (addr < 0xC000) {
    if (basicRomEnabled && basicRom) {
      return basicRom[addr - 0xA000];
    }
    return ram[addr];
  }

  // Hardware I/O area ($D000-$D7FF)
  if (addr >= 0xD000 && addr < 0xD800) {
    return readIO(addr);
  }

  // OS ROM area ($C000-$CFFF, $D800-$FFFF)
  if (osRomEnabled && osRom) {
    if (addr >= 0xC000 && addr < 0xD000) {
      return osRom[addr - 0xC000];
    }
    if (addr >= 0xD800) {
      return osRom[addr - 0xC000];
    }
  }

  // Fall back to RAM
  return ram[addr];
}

void Atari800Sys::setMem(uint16_t addr, uint8_t val) {
  // RAM always writable (except for ROM-shadowed areas)
  if (addr < 0xA000) {
    ram[addr] = val;
    return;
  }

  // BASIC ROM area - write to RAM
  if (addr < 0xC000) {
    ram[addr] = val;
    return;
  }

  // Hardware I/O area ($D000-$D7FF)
  if (addr >= 0xD000 && addr < 0xD800) {
    writeIO(addr, val);
    return;
  }

  // OS ROM area - write to underlying RAM (if accessible)
  if (addr >= 0xD800) {
    ram[addr] = val;
  }
}

uint8_t Atari800Sys::readIO(uint16_t addr) {
  uint8_t reg = addr & 0xFF;

  // GTIA: $D000-$D0FF (mirrored every 32 bytes)
  if (addr >= 0xD000 && addr < 0xD100) {
    return gtia.read(reg & 0x1F);
  }

  // POKEY: $D200-$D2FF (mirrored every 16 bytes)
  if (addr >= 0xD200 && addr < 0xD300) {
    return pokey.read(reg & 0x0F);
  }

  // PIA: $D300-$D3FF (mirrored every 4 bytes)
  if (addr >= 0xD300 && addr < 0xD400) {
    return pia.read(reg & 0x03);
  }

  // ANTIC: $D400-$D4FF (mirrored every 16 bytes)
  if (addr >= 0xD400 && addr < 0xD500) {
    return antic.read(reg & 0x0F);
  }

  // Unused/cartridge area
  return 0xFF;
}

void Atari800Sys::writeIO(uint16_t addr, uint8_t val) {
  uint8_t reg = addr & 0xFF;

  // GTIA: $D000-$D0FF
  if (addr >= 0xD000 && addr < 0xD100) {
    gtia.write(reg & 0x1F, val);
    return;
  }

  // POKEY: $D200-$D2FF
  if (addr >= 0xD200 && addr < 0xD300) {
    pokey.write(reg & 0x0F, val);
    return;
  }

  // PIA: $D300-$D3FF
  if (addr >= 0xD300 && addr < 0xD400) {
    pia.write(reg & 0x03, val);
    // Check for banking changes
    updateBanking();
    return;
  }

  // ANTIC: $D400-$D4FF
  if (addr >= 0xD400 && addr < 0xD500) {
    antic.write(reg & 0x0F, val);
    return;
  }
}

void Atari800Sys::checkInterrupts() {
  // Check for NMI (from ANTIC)
  if (antic.checkVBI() || antic.checkDLI()) {
    handleNMI();
  }

  // Check for IRQ (from POKEY)
  if (!iflag && pokey.checkIRQ()) {
    handleIRQ();
  }
}

bool Atari800Sys::handleNMI() {
  if (nmiActive) {
    return false;
  }

  nmiActive = true;

  // Push PC and status
  pushtostack(pc >> 8);
  pushtostack(pc & 0xFF);
  uint8_t status = 0x20;  // Unused bit always set
  if (nflag) status |= 0x80;
  if (vflag) status |= 0x40;
  if (bflag) status |= 0x10;
  if (dflag) status |= 0x08;
  if (iflag) status |= 0x04;
  if (zflag) status |= 0x02;
  if (cflag) status |= 0x01;
  pushtostack(status);

  // Read NMI vector
  pc = getMem(0xFFFA) | (getMem(0xFFFB) << 8);
  iflag = true;
  numofcycles += 7;

  return true;
}

bool Atari800Sys::handleIRQ() {
  if (iflag) {
    return false;
  }

  // Push PC and status
  pushtostack(pc >> 8);
  pushtostack(pc & 0xFF);
  uint8_t status = 0x20;
  if (nflag) status |= 0x80;
  if (vflag) status |= 0x40;
  // B flag clear for IRQ
  if (dflag) status |= 0x08;
  if (iflag) status |= 0x04;
  if (zflag) status |= 0x02;
  if (cflag) status |= 0x01;
  pushtostack(status);

  // Read IRQ vector
  pc = getMem(0xFFFE) | (getMem(0xFFFF) << 8);
  iflag = true;
  numofcycles += 7;

  return true;
}

void Atari800Sys::cmd6502brk() {
  // BRK instruction - software interrupt
  pc++;  // Skip padding byte

  pushtostack(pc >> 8);
  pushtostack(pc & 0xFF);

  uint8_t status = 0x30;  // B flag set for BRK
  if (nflag) status |= 0x80;
  if (vflag) status |= 0x40;
  if (dflag) status |= 0x08;
  if (iflag) status |= 0x04;
  if (zflag) status |= 0x02;
  if (cflag) status |= 0x01;
  pushtostack(status);

  pc = getMem(0xFFFE) | (getMem(0xFFFF) << 8);
  iflag = true;
  numofcycles = 7;
}

void Atari800Sys::logDebugInfo() {
  // Debug logging if enabled
}

void Atari800Sys::scanKeyboard() {
  if (keyboard) {
    // Keyboard scanning handled by external keyboard driver
    // Key codes sent to POKEY via setKeyCode()
  }

  // Update joystick
  if (joystick) {
    bool up, down, left, right, fire;
    joystick->getValues(up, down, left, right, fire);
    pia.setJoystick1(up, down, left, right);
    gtia.setTrigger(0, fire);
  }
}

void Atari800Sys::run() {
  int64_t lastMeasuredTime = PlatformManager::getInstance().getTimeUS();
  uint32_t totalCycles = 0;

  while (!cpuhalted) {
    // Execute instructions for one scanline
    cyclesThisScanline = 0;
    int32_t targetCycles = cyclesPerScanline - antic.dmaCycles;

    while (cyclesThisScanline < targetCycles) {
      // Check for WSYNC halt
      if (antic.isWSYNCHalted()) {
        cyclesThisScanline = targetCycles;
        break;
      }

      // Execute one instruction
      numofcycles = 0;
      logDebugInfo();
      execute(getMem(pc++));
      cyclesThisScanline += numofcycles;
      totalCycles += numofcycles;

      // Check interrupts
      checkInterrupts();
    }

    // Release WSYNC at end of scanline
    antic.releaseWSYNC();

    // Draw scanline
    antic.drawScanline();

    // Generate audio samples for this scanline
    pokey.fillBuffer(antic.getScanline());

    // Advance to next scanline
    antic.nextScanline();

    // End of frame handling
    if (antic.getScanline() == 0) {
      // Play accumulated audio
      pokey.playAudio();

      // Reset NMI latch
      nmiActive = false;

      // Frame timing
      int64_t nominalFrameTime = lastMeasuredTime + (1000000 / 50);  // 50 Hz PAL
      int64_t now = PlatformManager::getInstance().getTimeUS();

      if (nominalFrameTime > now) {
        PlatformManager::getInstance().waitUS(nominalFrameTime - now);
      }

      lastMeasuredTime = PlatformManager::getInstance().getTimeUS();

      // Update profiling
      if (perf) {
        numofcyclespersecond = totalCycles * 50;
      }
      totalCycles = 0;
    }
  }
}

// Need to declare pushtostack as it's used in interrupt handling
inline void Atari800Sys::pushtostack(uint8_t val) {
  ram[0x100 + sp] = val;
  sp--;
}
