/*
 Copyright (C) 2024-2025 retroelec <retroelec42@gmail.com>

 Minimal Atari BASIC ROM placeholder
 ====================================
 This provides a minimal BASIC that just returns.
 Replace with Altirra BASIC or original Atari BASIC for full functionality.
*/
#include "atari_basic.h"
#include <cstring>

// Minimal BASIC ROM - starts as empty, filled at runtime
static uint8_t atari_basic_rom_data[ATARI_BASIC_SIZE];
static bool basic_initialized = false;

const uint8_t* getAtariBasicRom() {
  if (!basic_initialized) {
    // Fill with NOP pattern - BASIC is optional for many games
    memset(atari_basic_rom_data, 0xEA, ATARI_BASIC_SIZE);
    basic_initialized = true;
  }
  return atari_basic_rom_data;
}

// For backward compatibility, provide a const reference
// Note: This will be zero-initialized until getAtariBasicRom() is called
alignas(4) const uint8_t atari_basic_rom[ATARI_BASIC_SIZE] = {0};
