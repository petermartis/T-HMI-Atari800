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

 T-HMI-Atari800 - Atari 800 XL Emulator for Lilygo T-HMI
 ======================================================

 This is an Atari 800 XL emulator designed to run on the Lilygo T-HMI board
 (ESP32-S3 based with 320x240 LCD display).

 Features:
 - Full Atari 800 XL emulation (6502 CPU, ANTIC, GTIA, POKEY, PIA)
 - 64KB RAM
 - Support for Altirra OS/BASIC ROMs
 - Audio output (on Waveshare board with I2S)
 - Joystick support
 - BLE or Web keyboard input

 Hardware Requirements:
 - Lilygo T-HMI (ESP32-S3, 320x240 LCD)
 - Or: Waveshare ESP32-S3-LCD-2.8 (includes audio output)
 - Optional: Analog joystick shield
 - Optional: SD card for loading programs

 Based on the T-HMI-C64 emulator architecture by retroelec.
*/

#include "src/Atari800Emu.h"
#include "src/platform/PlatformManager.h"

// Global emulator instance
Atari800Emu atari800Emu;

// Core 0 task handle
TaskHandle_t core0TaskHandle;

static const char *TAG = "T-HMI-Atari800";

/**
 * @brief Core 0 task - runs the main emulator loop
 */
void core0Task(void *param) {
  Serial.println("core0Task starting setup...");
  atari800Emu.setup();
  Serial.println("core0Task setup complete");

  vTaskDelay(pdMS_TO_TICKS(1000));
  PlatformManager::getInstance().log(LOG_INFO, TAG, "Atari 800 XL Emulator ready");

  while (true) {
    atari800Emu.loop();
  }
}

/**
 * @brief Arduino setup function
 */
void setup() {
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(500));

  Serial.println("T-HMI-Atari800 Starting...");

  // Create emulation task on core 0
  xTaskCreatePinnedToCore(
      core0Task,
      "core0Task",
      8192,           // Stack size
      nullptr,
      1,              // Priority
      &core0TaskHandle,
      0               // Core 0
  );
}

/**
 * @brief Arduino loop function (runs on core 1, mostly idle)
 */
void loop() {
  // Main loop is empty - all work happens in core0Task
  vTaskDelay(pdMS_TO_TICKS(1000));
}
