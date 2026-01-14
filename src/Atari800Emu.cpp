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
#include "Atari800Emu.h"
#include "board/BoardFactory.h"
#include "joystick/JoystickFactory.h"
#include "keyboard/KeyboardFactory.h"
#include "platform/PlatformFactory.h"
#include "platform/PlatformManager.h"
#include "roms/atarixl_os.h"
#include "roms/atari_basic.h"
#include <cstring>

static const char *TAG = "Atari800Emu";

// Memory size for Atari 800 XL (64KB)
constexpr uint32_t RAM_SIZE = 64 * 1024;

Atari800Emu::Atari800Emu() : ram(nullptr), board(nullptr) {
  cntSecondsForBatteryCheck = 0;
}

Atari800Emu::~Atari800Emu() {
  if (ram) {
    delete[] ram;
    ram = nullptr;
  }
}

void Atari800Emu::intervalTimerScanKeyboardFunc() {
  sys.scanKeyboard();
}

void Atari800Emu::intervalTimerProfilingBatteryCheckFunc() {
  // Update profiling info
  if (showperfvalues.load()) {
    numofcyclespersecond.store(sys.numofcyclespersecond.load());
  }

  // Battery check every 60 seconds
  cntSecondsForBatteryCheck++;
  if (cntSecondsForBatteryCheck >= 60) {
    cntSecondsForBatteryCheck = 0;
    if (board) {
      uint16_t voltage = board->getBatteryVoltage();
      if (voltage > 0 && voltage < 3300) {
        // Low battery warning - could display overlay
        PlatformManager::getInstance().log(LOG_WARN, TAG, "Low battery: %dmV", voltage);
      }
    }
  }
}

void Atari800Emu::cpuCode(void *parameter) {
  sys.run();
}

void Atari800Emu::setup() {
  // Initialize platform first
  PlatformManager::initialize(PlatformNS::create());

  PlatformManager::getInstance().log(LOG_INFO, TAG, "Atari 800 XL Emulator starting...");

  // Initialize board driver
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Creating board driver...");
  board = Board::create();
  if (board) {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "Initializing board...");
    board->init();
  }
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Board initialized");

  // Allocate RAM
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Allocating RAM...");
  ram = new uint8_t[RAM_SIZE];
  memset(ram, 0, RAM_SIZE);
  PlatformManager::getInstance().log(LOG_INFO, TAG, "RAM allocated");

  // Initialize system with ROMs (use getters to get initialized ROM data)
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Initializing system...");
  sys.init(ram, getAtariOSRom(), getAtariBasicRom());
  PlatformManager::getInstance().log(LOG_INFO, TAG, "System initialized");

  // Create keyboard driver
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Creating keyboard...");
  sys.keyboard = Keyboard::create();
  if (sys.keyboard) {
    sys.keyboard->init();
  }
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Keyboard initialized");

  // Create joystick driver
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Creating joystick...");
  JoystickDriver *joystick = Joystick::create();
  if (joystick) {
    joystick->init();
    sys.setJoystick(joystick);
  }
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Joystick initialized");

  // Start CPU task on core 1
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Starting CPU task...");
  PlatformManager::getInstance().startTask(
      [this](void *param) { this->cpuCode(param); },
      1,  // Core 1
      5   // Priority
  );

  // Start keyboard scanner timer (every 8ms)
  PlatformManager::getInstance().startIntervalTimer(
      [this]() { this->intervalTimerScanKeyboardFunc(); },
      8000  // 8ms
  );

  // Start profiling/battery timer (every 1 second)
  PlatformManager::getInstance().startIntervalTimer(
      [this]() { this->intervalTimerProfilingBatteryCheckFunc(); },
      1000000  // 1 second
  );

  PlatformManager::getInstance().log(LOG_INFO, TAG, "Setup complete");
}

void Atari800Emu::loop() {
  // Main loop - refresh display
  sys.antic.refresh();

  // Feed watchdog
  PlatformManager::getInstance().feedWDT();

  // Small delay
  PlatformManager::getInstance().waitMS(Config::REFRESHDELAY);

  // Update refresh counter
  cntRefreshs.store(sys.antic.cntRefreshs.load());
}
