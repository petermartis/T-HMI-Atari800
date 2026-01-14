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
#ifndef ATARI800EMU_H
#define ATARI800EMU_H

#include "Atari800Sys.h"
#include "board/BoardDriver.h"
#include <atomic>

/**
 * @brief Main Atari 800 Emulator class
 *
 * This class orchestrates the emulator:
 * - Initializes hardware drivers
 * - Allocates memory
 * - Sets up the emulation core
 * - Manages the main loop and display refresh
 */
class Atari800Emu {
private:
  uint8_t *ram;
  BoardDriver *board;
  uint16_t cntSecondsForBatteryCheck;

  void intervalTimerScanKeyboardFunc();
  void intervalTimerProfilingBatteryCheckFunc();
  void cpuCode(void *parameter);

public:
  Atari800Sys sys;
  std::atomic<bool> showperfvalues = false;
  std::atomic<uint8_t> cntRefreshs = 0;
  std::atomic<uint32_t> numofcyclespersecond = 0;

  Atari800Emu();
  ~Atari800Emu();

  void setup();
  void loop();
};

#endif // ATARI800EMU_H
