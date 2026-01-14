/*
 Copyright (C) 2024-2025 retroelec <retroelec42@gmail.com>

 Minimal Atari BASIC ROM placeholder
 ====================================
 This provides a minimal BASIC that just prints "NO BASIC" and returns.
 Replace with Altirra BASIC or original Atari BASIC for full functionality.
*/
#include "atari_basic.h"

// Minimal BASIC ROM - filled with NOPs, with entry point that just returns
alignas(4) const uint8_t atari_basic_rom[ATARI_BASIC_SIZE] = {
    // Fill with RTI/NOP pattern - BASIC is optional for many games
    [0 ... ATARI_BASIC_SIZE - 1] = 0xEA  // NOP
};
