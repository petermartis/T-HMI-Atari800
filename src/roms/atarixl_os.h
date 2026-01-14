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

 Atari XL OS ROM
 ===============
 This is a minimal OS ROM that provides basic functionality for testing.
 For full compatibility, replace with the Altirra OS ROM which is freely
 distributable, or an original Atari XL OS ROM (REV2 recommended).

 The Altirra OS can be obtained from: https://www.virtualdub.org/altirra.html
 The Altirra OS/BASIC are released under a permissive license for emulator use.

 OS ROM resides at $C000-$FFFF (16KB)
 - $C000-$CFFF: Floating-point routines and misc
 - $D000-$D7FF: Self-test ROM (when enabled via PORTB)
 - $D800-$DFFF: I/O space (not ROM, but used for character set shadow)
 - $E000-$E3FF: Internal character set
 - $E400-$FFFF: Main OS routines and vectors
*/
#ifndef ATARIXL_OS_H
#define ATARIXL_OS_H

#include <cstdint>

// Atari XL OS ROM size: 16KB ($C000-$FFFF)
constexpr uint16_t ATARIXL_OS_SIZE = 16384;

// Minimal boot ROM that displays a test pattern and waits
// This is a placeholder - replace with full Altirra or original OS for games
extern const uint8_t atarixl_os_rom[ATARIXL_OS_SIZE];

#endif // ATARIXL_OS_H
