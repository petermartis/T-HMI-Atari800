/*
 Copyright (C) 2024-2025 retroelec <retroelec42@gmail.com>

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 3 of the License, or (at your
 option) any later version.

 Atari 800 Keyboard Codes
 ========================
 POKEY keyboard codes as read from KBCODE register ($D209)

 The keyboard code format is:
 Bit 7: Unused
 Bit 6: Control key modifier
 Bit 5: Shift key modifier
 Bits 4-0: Key code (0-63)
*/
#ifndef ATARIKEYCODES_H
#define ATARIKEYCODES_H

#include <cstdint>

// Key codes (without modifiers)
constexpr uint8_t ATARI_KEY_L = 0x00;
constexpr uint8_t ATARI_KEY_J = 0x01;
constexpr uint8_t ATARI_KEY_SEMICOLON = 0x02;
constexpr uint8_t ATARI_KEY_F1 = 0x03;      // Function key (800)
constexpr uint8_t ATARI_KEY_F2 = 0x04;      // Function key (800)
constexpr uint8_t ATARI_KEY_K = 0x05;
constexpr uint8_t ATARI_KEY_PLUS = 0x06;
constexpr uint8_t ATARI_KEY_ASTERISK = 0x07;

constexpr uint8_t ATARI_KEY_O = 0x08;
constexpr uint8_t ATARI_KEY_NONE = 0x09;    // No key
constexpr uint8_t ATARI_KEY_P = 0x0A;
constexpr uint8_t ATARI_KEY_U = 0x0B;
constexpr uint8_t ATARI_KEY_RETURN = 0x0C;
constexpr uint8_t ATARI_KEY_I = 0x0D;
constexpr uint8_t ATARI_KEY_MINUS = 0x0E;
constexpr uint8_t ATARI_KEY_EQUALS = 0x0F;

constexpr uint8_t ATARI_KEY_V = 0x10;
constexpr uint8_t ATARI_KEY_HELP = 0x11;    // Help key (XL/XE)
constexpr uint8_t ATARI_KEY_C = 0x12;
constexpr uint8_t ATARI_KEY_F3 = 0x13;      // Function key (800)
constexpr uint8_t ATARI_KEY_F4 = 0x14;      // Function key (800)
constexpr uint8_t ATARI_KEY_B = 0x15;
constexpr uint8_t ATARI_KEY_X = 0x16;
constexpr uint8_t ATARI_KEY_Z = 0x17;

constexpr uint8_t ATARI_KEY_4 = 0x18;
constexpr uint8_t ATARI_KEY_NONE2 = 0x19;
constexpr uint8_t ATARI_KEY_3 = 0x1A;
constexpr uint8_t ATARI_KEY_6 = 0x1B;
constexpr uint8_t ATARI_KEY_ESC = 0x1C;
constexpr uint8_t ATARI_KEY_5 = 0x1D;
constexpr uint8_t ATARI_KEY_2 = 0x1E;
constexpr uint8_t ATARI_KEY_1 = 0x1F;

constexpr uint8_t ATARI_KEY_COMMA = 0x20;
constexpr uint8_t ATARI_KEY_SPACE = 0x21;
constexpr uint8_t ATARI_KEY_PERIOD = 0x22;
constexpr uint8_t ATARI_KEY_N = 0x23;
constexpr uint8_t ATARI_KEY_NONE3 = 0x24;
constexpr uint8_t ATARI_KEY_M = 0x25;
constexpr uint8_t ATARI_KEY_SLASH = 0x26;
constexpr uint8_t ATARI_KEY_INVERSE = 0x27; // Atari/Inverse key

constexpr uint8_t ATARI_KEY_R = 0x28;
constexpr uint8_t ATARI_KEY_NONE4 = 0x29;
constexpr uint8_t ATARI_KEY_E = 0x2A;
constexpr uint8_t ATARI_KEY_Y = 0x2B;
constexpr uint8_t ATARI_KEY_TAB = 0x2C;
constexpr uint8_t ATARI_KEY_T = 0x2D;
constexpr uint8_t ATARI_KEY_W = 0x2E;
constexpr uint8_t ATARI_KEY_Q = 0x2F;

constexpr uint8_t ATARI_KEY_9 = 0x30;
constexpr uint8_t ATARI_KEY_NONE5 = 0x31;
constexpr uint8_t ATARI_KEY_0 = 0x32;
constexpr uint8_t ATARI_KEY_7 = 0x33;
constexpr uint8_t ATARI_KEY_BACKSPACE = 0x34;
constexpr uint8_t ATARI_KEY_8 = 0x35;
constexpr uint8_t ATARI_KEY_LESS = 0x36;
constexpr uint8_t ATARI_KEY_GREATER = 0x37;

constexpr uint8_t ATARI_KEY_F = 0x38;
constexpr uint8_t ATARI_KEY_H = 0x39;
constexpr uint8_t ATARI_KEY_D = 0x3A;
constexpr uint8_t ATARI_KEY_NONE6 = 0x3B;
constexpr uint8_t ATARI_KEY_CAPS = 0x3C;
constexpr uint8_t ATARI_KEY_G = 0x3D;
constexpr uint8_t ATARI_KEY_S = 0x3E;
constexpr uint8_t ATARI_KEY_A = 0x3F;

// Modifier bits
constexpr uint8_t ATARI_MOD_SHIFT = 0x40;
constexpr uint8_t ATARI_MOD_CONTROL = 0x80;

// Special key codes (directly trigger POKEY)
constexpr uint8_t ATARI_KEY_BREAK = 0xFF;  // Break key (NMI)

// Console keys (directly to GTIA CONSOL register)
constexpr uint8_t ATARI_CONSOLE_START = 0x01;
constexpr uint8_t ATARI_CONSOLE_SELECT = 0x02;
constexpr uint8_t ATARI_CONSOLE_OPTION = 0x04;

/**
 * @brief Convert ASCII character to Atari key code
 *
 * @param ascii ASCII character code
 * @return Atari key code with appropriate modifiers
 */
inline uint8_t asciiToAtariKey(char ascii) {
  switch (ascii) {
  // Letters (unshifted = uppercase on Atari)
  case 'A': case 'a': return ATARI_KEY_A;
  case 'B': case 'b': return ATARI_KEY_B;
  case 'C': case 'c': return ATARI_KEY_C;
  case 'D': case 'd': return ATARI_KEY_D;
  case 'E': case 'e': return ATARI_KEY_E;
  case 'F': case 'f': return ATARI_KEY_F;
  case 'G': case 'g': return ATARI_KEY_G;
  case 'H': case 'h': return ATARI_KEY_H;
  case 'I': case 'i': return ATARI_KEY_I;
  case 'J': case 'j': return ATARI_KEY_J;
  case 'K': case 'k': return ATARI_KEY_K;
  case 'L': case 'l': return ATARI_KEY_L;
  case 'M': case 'm': return ATARI_KEY_M;
  case 'N': case 'n': return ATARI_KEY_N;
  case 'O': case 'o': return ATARI_KEY_O;
  case 'P': case 'p': return ATARI_KEY_P;
  case 'Q': case 'q': return ATARI_KEY_Q;
  case 'R': case 'r': return ATARI_KEY_R;
  case 'S': case 's': return ATARI_KEY_S;
  case 'T': case 't': return ATARI_KEY_T;
  case 'U': case 'u': return ATARI_KEY_U;
  case 'V': case 'v': return ATARI_KEY_V;
  case 'W': case 'w': return ATARI_KEY_W;
  case 'X': case 'x': return ATARI_KEY_X;
  case 'Y': case 'y': return ATARI_KEY_Y;
  case 'Z': case 'z': return ATARI_KEY_Z;

  // Numbers
  case '1': return ATARI_KEY_1;
  case '2': return ATARI_KEY_2;
  case '3': return ATARI_KEY_3;
  case '4': return ATARI_KEY_4;
  case '5': return ATARI_KEY_5;
  case '6': return ATARI_KEY_6;
  case '7': return ATARI_KEY_7;
  case '8': return ATARI_KEY_8;
  case '9': return ATARI_KEY_9;
  case '0': return ATARI_KEY_0;

  // Shifted numbers (symbols)
  case '!': return ATARI_KEY_1 | ATARI_MOD_SHIFT;
  case '"': return ATARI_KEY_2 | ATARI_MOD_SHIFT;
  case '#': return ATARI_KEY_3 | ATARI_MOD_SHIFT;
  case '$': return ATARI_KEY_4 | ATARI_MOD_SHIFT;
  case '%': return ATARI_KEY_5 | ATARI_MOD_SHIFT;
  case '&': return ATARI_KEY_6 | ATARI_MOD_SHIFT;
  case '\'': return ATARI_KEY_7 | ATARI_MOD_SHIFT;
  case '@': return ATARI_KEY_8 | ATARI_MOD_SHIFT;
  case '(': return ATARI_KEY_9 | ATARI_MOD_SHIFT;
  case ')': return ATARI_KEY_0 | ATARI_MOD_SHIFT;

  // Punctuation
  case ' ': return ATARI_KEY_SPACE;
  case ',': return ATARI_KEY_COMMA;
  case '.': return ATARI_KEY_PERIOD;
  case ';': return ATARI_KEY_SEMICOLON;
  case ':': return ATARI_KEY_SEMICOLON | ATARI_MOD_SHIFT;
  case '-': return ATARI_KEY_MINUS;
  case '=': return ATARI_KEY_EQUALS;
  case '+': return ATARI_KEY_PLUS;
  case '*': return ATARI_KEY_ASTERISK;
  case '/': return ATARI_KEY_SLASH;
  case '?': return ATARI_KEY_SLASH | ATARI_MOD_SHIFT;
  case '<': return ATARI_KEY_LESS;
  case '>': return ATARI_KEY_GREATER;

  // Control characters
  case '\r': case '\n': return ATARI_KEY_RETURN;
  case '\t': return ATARI_KEY_TAB;
  case '\b': case 127: return ATARI_KEY_BACKSPACE;
  case 27: return ATARI_KEY_ESC;

  default: return ATARI_KEY_NONE;
  }
}

#endif // ATARIKEYCODES_H
