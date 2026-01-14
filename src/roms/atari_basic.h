/*
 Copyright (C) 2024-2025 retroelec <retroelec42@gmail.com>

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 3 of the License, or (at your
 option) any later version.

 Atari BASIC ROM
 ===============
 This is a placeholder for the Atari BASIC ROM.
 For full compatibility, use Altirra BASIC (freely distributable)
 or an original Atari BASIC ROM (Rev C recommended).

 The Altirra BASIC can be obtained from: https://www.virtualdub.org/altirra.html

 BASIC ROM resides at $A000-$BFFF (8KB)
*/
#ifndef ATARI_BASIC_H
#define ATARI_BASIC_H

#include <cstdint>

// Atari BASIC ROM size: 8KB ($A000-$BFFF)
constexpr uint16_t ATARI_BASIC_SIZE = 8192;

// Minimal BASIC ROM placeholder
extern const uint8_t atari_basic_rom[ATARI_BASIC_SIZE];

#endif // ATARI_BASIC_H
