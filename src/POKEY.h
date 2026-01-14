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
#ifndef POKEY_H
#define POKEY_H

#include "Config.h"
#include "sound/SoundDriver.h"
#include <cstdint>

// POKEY register addresses (offset from base $D200)
// Write registers
constexpr uint8_t AUDF1_W = 0x00;  // Audio channel 1 frequency
constexpr uint8_t AUDC1_W = 0x01;  // Audio channel 1 control
constexpr uint8_t AUDF2_W = 0x02;  // Audio channel 2 frequency
constexpr uint8_t AUDC2_W = 0x03;  // Audio channel 2 control
constexpr uint8_t AUDF3_W = 0x04;  // Audio channel 3 frequency
constexpr uint8_t AUDC3_W = 0x05;  // Audio channel 3 control
constexpr uint8_t AUDF4_W = 0x06;  // Audio channel 4 frequency
constexpr uint8_t AUDC4_W = 0x07;  // Audio channel 4 control
constexpr uint8_t AUDCTL_W = 0x08; // Audio control
constexpr uint8_t STIMER_W = 0x09; // Start timers
constexpr uint8_t SKREST_W = 0x0A; // Reset serial status
constexpr uint8_t POTGO_W = 0x0B;  // Start POT scan
constexpr uint8_t SEROUT_W = 0x0D; // Serial output
constexpr uint8_t IRQEN_W = 0x0E;  // IRQ enable
constexpr uint8_t SKCTL_W = 0x0F;  // Serial port control

// Read registers
constexpr uint8_t POT0_R = 0x00;   // Paddle 0
constexpr uint8_t POT1_R = 0x01;   // Paddle 1
constexpr uint8_t POT2_R = 0x02;   // Paddle 2
constexpr uint8_t POT3_R = 0x03;   // Paddle 3
constexpr uint8_t POT4_R = 0x04;   // Paddle 4
constexpr uint8_t POT5_R = 0x05;   // Paddle 5
constexpr uint8_t POT6_R = 0x06;   // Paddle 6
constexpr uint8_t POT7_R = 0x07;   // Paddle 7
constexpr uint8_t ALLPOT_R = 0x08; // All paddle port status
constexpr uint8_t KBCODE_R = 0x09; // Keyboard code
constexpr uint8_t RANDOM_R = 0x0A; // Random number generator
constexpr uint8_t SERIN_R = 0x0D;  // Serial input
constexpr uint8_t IRQST_R = 0x0E;  // IRQ status
constexpr uint8_t SKSTAT_R = 0x0F; // Serial port status

// AUDCTL bits
constexpr uint8_t AUDCTL_POLY9 = 0x80;     // Use 9-bit poly instead of 17-bit
constexpr uint8_t AUDCTL_CH1_179 = 0x40;   // Channel 1 uses 1.79 MHz clock
constexpr uint8_t AUDCTL_CH3_179 = 0x20;   // Channel 3 uses 1.79 MHz clock
constexpr uint8_t AUDCTL_CH1_CH2 = 0x10;   // Channels 1+2 clocked together (16-bit)
constexpr uint8_t AUDCTL_CH3_CH4 = 0x08;   // Channels 3+4 clocked together (16-bit)
constexpr uint8_t AUDCTL_CH1_HPFILT = 0x04; // Channel 1 high-pass filter
constexpr uint8_t AUDCTL_CH2_HPFILT = 0x02; // Channel 2 high-pass filter
constexpr uint8_t AUDCTL_15KHZ = 0x01;     // Use 15 kHz clock instead of 64 kHz

// IRQEN/IRQST bits (active-low in IRQST)
constexpr uint8_t IRQ_TIMER1 = 0x01;       // Timer 1 underflow
constexpr uint8_t IRQ_TIMER2 = 0x02;       // Timer 2 underflow
constexpr uint8_t IRQ_TIMER4 = 0x04;       // Timer 4 underflow
constexpr uint8_t IRQ_SERIAL_OUT = 0x08;   // Serial output needed
constexpr uint8_t IRQ_SERIAL_IN = 0x10;    // Serial input ready
constexpr uint8_t IRQ_KEYPRESS = 0x40;     // Keyboard key pressed
constexpr uint8_t IRQ_BREAK = 0x80;        // Break key pressed

// SKSTAT bits (active-low)
constexpr uint8_t SKSTAT_SERIN = 0x10;     // Serial input shift register busy
constexpr uint8_t SKSTAT_KEYDOWN = 0x04;   // Any key pressed
constexpr uint8_t SKSTAT_LASTKEY = 0x08;   // Last key still pressed

// POKEY base clock frequency (NTSC)
constexpr uint32_t POKEY_FREQ = 1789790;
constexpr uint32_t POKEY_DIV_64 = 28;      // 64 kHz divisor (~63.9 kHz)
constexpr uint32_t POKEY_DIV_15 = 114;     // 15 kHz divisor (~15.7 kHz)

/**
 * @brief Represents a single POKEY audio channel
 */
class POKEYChannel {
public:
  uint8_t audf;           // Frequency divider value
  uint8_t audc;           // Control register (distortion + volume)
  uint32_t divider;       // Current divider counter
  uint32_t period;        // Full period length
  bool output;            // Current output state (high/low)
  int16_t lastOutput;     // Last output sample (for high-pass filter)

  POKEYChannel();
  void reset();
  void setFrequency(uint8_t freq);
  void setControl(uint8_t ctrl);

  uint8_t getVolume() const { return audc & 0x0F; }
  uint8_t getDistortion() const { return (audc >> 4) & 0x07; }
  bool isVolumeOnly() const { return (audc & 0x10) != 0; }
};

/**
 * @brief POKEY - POtentiometer and KEYboard chip
 *
 * The POKEY chip handles:
 * - 4 audio channels with various distortion/noise modes
 * - Keyboard scanning and debouncing
 * - Serial I/O (SIO bus for disk/cassette)
 * - Paddle (potentiometer) reading
 * - Random number generation (polynomial counters)
 * - Timer interrupts (using audio timers)
 */
class POKEY {
private:
  static const uint16_t NUMSAMPLESPERFRAME = AUDIO_SAMPLE_RATE / 50;
  int16_t samples[NUMSAMPLESPERFRAME];
  SoundDriver *sound;
  uint16_t actSampleIdx;

  // Audio channels
  POKEYChannel channel[4];

  // Audio control
  uint8_t audctl;
  bool poly9Mode;
  bool ch1_179mhz;
  bool ch3_179mhz;
  bool ch12_joined;
  bool ch34_joined;
  bool ch1_highpass;
  bool ch2_highpass;
  bool clock15khz;

  // Polynomial counters for noise generation
  uint32_t poly4;         // 4-bit polynomial counter
  uint32_t poly5;         // 5-bit polynomial counter
  uint32_t poly9;         // 9-bit polynomial counter
  uint32_t poly17;        // 17-bit polynomial counter
  uint32_t polyStep;      // Counter for polynomial updates

  // Timer/interrupt related
  uint8_t irqen;          // IRQ enable register
  uint8_t irqst;          // IRQ status register (active-low)

  // Keyboard
  uint8_t kbcode;         // Last keyboard code
  bool keyPressed;        // Any key currently pressed
  uint8_t skctl;          // Serial control
  uint8_t skstat;         // Serial status

  // Paddle (POT) inputs
  uint8_t pot[8];         // Paddle values
  uint8_t allpot;         // All paddle scan status

  // Serial I/O
  uint8_t serout;         // Serial output register
  uint8_t serin;          // Serial input register

  // Random number generator
  uint8_t random;

  void updatePolynomials();
  void updateChannelPeriods();
  int16_t generateSample();

public:
  // Volume control
  float emuVolume;
  uint8_t emuVolumeScaled;

  POKEY();
  void init();
  void reset();

  // Register access
  uint8_t read(uint8_t addr);
  void write(uint8_t addr, uint8_t val);

  // Audio generation (called each scanline)
  void fillBuffer(uint16_t scanline);
  void playAudio();

  // Keyboard interface
  void setKeyCode(uint8_t code, bool pressed);
  void setBreakKey(bool pressed);

  // Paddle interface
  void setPaddle(uint8_t num, uint8_t value);
  void startPotScan();

  // IRQ interface
  bool checkIRQ();
  uint8_t getIRQStatus() const { return irqst; }
  void acknowledgeIRQ(uint8_t mask);
  void triggerTimerIRQ(uint8_t timer);

  // Timer tick (called by CPU)
  void tickTimers(uint8_t cycles);

  // Volume control
  uint8_t getEmuVolume() const;
  void setEmuVolume(uint8_t volume);
};

#endif // POKEY_H
