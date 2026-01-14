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
#ifndef ATARIDISPLAYDRIVER_H
#define ATARIDISPLAYDRIVER_H

#include "DisplayDriver.h"
#include <cmath>

/**
 * @brief Atari palette helper for display drivers.
 *
 * Provides Atari-specific 256-color palette support. The Atari 800 uses
 * a 256-color palette with 16 hues x 16 luminances.
 *
 * This class does NOT inherit from DisplayDriver - instead it provides
 * palette conversion utilities that can be used alongside any DisplayDriver.
 */
class AtariPalette {
private:
  // Atari 800 NTSC palette (256 colors in RGB565 format)
  uint16_t atariColors[256];

  void generatePalette() {
    // Generate Atari NTSC palette
    // Color format: HHHHLLLL where H=hue (0-15), L=luminance (0-15)
    for (int color = 0; color < 256; color++) {
      int hue = (color >> 4) & 0x0F;
      int lum = color & 0x0F;

      // Convert to RGB
      float y = lum / 15.0f;  // Luminance 0-1
      float r, g, b;

      if (hue == 0) {
        // Grayscale
        r = g = b = y;
      } else {
        // NTSC color phase angles (approximate)
        static const float hueAngles[16] = {
            0.0f,    // 0: Gray (special case)
            0.0f,    // 1: Gold/Orange
            30.0f,   // 2: Orange
            60.0f,   // 3: Red-Orange
            90.0f,   // 4: Pink/Red
            120.0f,  // 5: Purple
            150.0f,  // 6: Purple-Blue
            180.0f,  // 7: Blue
            210.0f,  // 8: Blue
            240.0f,  // 9: Light Blue
            270.0f,  // 10: Turquoise
            300.0f,  // 11: Green-Blue
            330.0f,  // 12: Green
            350.0f,  // 13: Yellow-Green
            360.0f,  // 14: Orange-Green
            380.0f   // 15: Light Orange
        };

        float angle = hueAngles[hue] * 3.14159f / 180.0f;
        float saturation = 0.5f;

        // YIQ to RGB conversion (simplified)
        float i = saturation * cosf(angle);
        float q = saturation * sinf(angle);

        r = y + 0.956f * i + 0.621f * q;
        g = y - 0.272f * i - 0.647f * q;
        b = y - 1.105f * i + 1.702f * q;

        // Clamp values
        if (r < 0.0f) r = 0.0f; if (r > 1.0f) r = 1.0f;
        if (g < 0.0f) g = 0.0f; if (g > 1.0f) g = 1.0f;
        if (b < 0.0f) b = 0.0f; if (b > 1.0f) b = 1.0f;
      }

      // Convert to RGB565
      uint8_t r5 = (uint8_t)(r * 31.0f);
      uint8_t g6 = (uint8_t)(g * 63.0f);
      uint8_t b5 = (uint8_t)(b * 31.0f);

      atariColors[color] = (r5 << 11) | (g6 << 5) | b5;
    }
  }

public:
  AtariPalette() { generatePalette(); }

  /**
   * @brief Provides access to the Atari palette in RGB565 format.
   */
  const uint16_t *getAtariColors() const { return atariColors; }

  /**
   * @brief Convert Atari color index to RGB565.
   */
  uint16_t colorToRGB565(uint8_t colorIndex) const {
    return atariColors[colorIndex];
  }
};

// For backward compatibility, provide AtariDisplayDriver as an alias
// Note: Display drivers (ST7789V, etc.) inherit from DisplayDriver directly
using AtariDisplayDriver = DisplayDriver;

#endif // ATARIDISPLAYDRIVER_H
