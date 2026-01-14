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
#include "POKEY.h"
#include "sound/SoundFactory.h"
#include <cstring>

// Polynomial taps:
// 4-bit: x^4 + x^3 + 1 (period 15)
// 5-bit: x^5 + x^3 + 1 (period 31)
// 9-bit: x^9 + x^4 + 1 (period 511)
// 17-bit: x^17 + x^12 + 1 (period 131071)

POKEYChannel::POKEYChannel() { reset(); }

void POKEYChannel::reset() {
  audf = 0;
  audc = 0;
  divider = 0;
  period = 1;
  output = false;
  lastOutput = 0;
}

void POKEYChannel::setFrequency(uint8_t freq) {
  audf = freq;
}

void POKEYChannel::setControl(uint8_t ctrl) {
  audc = ctrl;
}

POKEY::POKEY() : sound(nullptr), actSampleIdx(0) {
  emuVolume = 1.0f;
  emuVolumeScaled = 128;
  reset();
}

void POKEY::init() {
  sound = Sound::create();
  if (sound) {
    sound->init();
  }
  reset();
}

void POKEY::reset() {
  for (int i = 0; i < 4; i++) {
    channel[i].reset();
  }

  audctl = 0;
  poly9Mode = false;
  ch1_179mhz = false;
  ch3_179mhz = false;
  ch12_joined = false;
  ch34_joined = false;
  ch1_highpass = false;
  ch2_highpass = false;
  clock15khz = false;

  // Initialize polynomial counters
  poly4 = 0x0F;
  poly5 = 0x1F;
  poly9 = 0x1FF;
  poly17 = 0x1FFFF;
  polyStep = 0;

  irqen = 0;
  irqst = 0xFF;  // All interrupts inactive (active-low)

  kbcode = 0xFF;
  keyPressed = false;
  skctl = 0;
  skstat = 0xFF;

  for (int i = 0; i < 8; i++) {
    pot[i] = 228;  // Middle position
  }
  allpot = 0;

  serout = 0;
  serin = 0;
  random = 0xFF;
  actSampleIdx = 0;

  memset(samples, 0, sizeof(samples));
}

void POKEY::updatePolynomials() {
  // Update 4-bit polynomial (x^4 + x^3 + 1)
  uint8_t bit4 = ((poly4 >> 3) ^ (poly4 >> 2)) & 1;
  poly4 = ((poly4 << 1) | bit4) & 0x0F;

  // Update 5-bit polynomial (x^5 + x^3 + 1)
  uint8_t bit5 = ((poly5 >> 4) ^ (poly5 >> 2)) & 1;
  poly5 = ((poly5 << 1) | bit5) & 0x1F;

  // Update 9-bit polynomial (x^9 + x^4 + 1)
  uint16_t bit9 = ((poly9 >> 8) ^ (poly9 >> 3)) & 1;
  poly9 = ((poly9 << 1) | bit9) & 0x1FF;

  // Update 17-bit polynomial (x^17 + x^12 + 1)
  uint32_t bit17 = ((poly17 >> 16) ^ (poly17 >> 11)) & 1;
  poly17 = ((poly17 << 1) | bit17) & 0x1FFFF;

  // Update random number
  if (poly9Mode) {
    random = (uint8_t)(poly9 ^ (poly9 >> 1));
  } else {
    random = (uint8_t)(poly17 ^ (poly17 >> 1));
  }
}

void POKEY::updateChannelPeriods() {
  uint32_t baseDiv = clock15khz ? POKEY_DIV_15 : POKEY_DIV_64;

  // Channel 1
  if (ch12_joined) {
    // 16-bit mode
    uint32_t freq16 = (channel[0].audf << 8) | channel[1].audf;
    if (ch1_179mhz) {
      channel[0].period = freq16 + 1;
    } else {
      channel[0].period = (freq16 + 1) * baseDiv;
    }
    channel[1].period = 0;  // Disabled when joined
  } else {
    if (ch1_179mhz) {
      channel[0].period = channel[0].audf + 4;
    } else {
      channel[0].period = (channel[0].audf + 1) * baseDiv;
    }
    channel[1].period = (channel[1].audf + 1) * baseDiv;
  }

  // Channel 3
  if (ch34_joined) {
    // 16-bit mode
    uint32_t freq16 = (channel[2].audf << 8) | channel[3].audf;
    if (ch3_179mhz) {
      channel[2].period = freq16 + 1;
    } else {
      channel[2].period = (freq16 + 1) * baseDiv;
    }
    channel[3].period = 0;  // Disabled when joined
  } else {
    if (ch3_179mhz) {
      channel[2].period = channel[2].audf + 4;
    } else {
      channel[2].period = (channel[2].audf + 1) * baseDiv;
    }
    channel[3].period = (channel[3].audf + 1) * baseDiv;
  }
}

int16_t POKEY::generateSample() {
  int32_t output = 0;

  // Update polynomials (roughly once per sample)
  polyStep++;
  if (polyStep >= 40) {  // ~44kHz / 40 = ~1.1kHz polynomial update rate
    updatePolynomials();
    polyStep = 0;
  }

  // Process each channel
  for (int ch = 0; ch < 4; ch++) {
    POKEYChannel &c = channel[ch];

    // Skip disabled channels (period == 0)
    if (c.period == 0) continue;

    // Volume-only mode - direct DAC output
    if (c.isVolumeOnly()) {
      output += c.getVolume() * 2048;
      continue;
    }

    // Skip silent channels
    if (c.getVolume() == 0) continue;

    // Update divider
    if (c.divider > 0) {
      c.divider--;
    } else {
      c.divider = c.period;
      c.output = !c.output;
    }

    // Generate output based on distortion mode
    uint8_t dist = c.getDistortion();
    bool finalOutput = c.output;

    switch (dist) {
    case 0:  // 5+17/9-bit polynomial (noise)
      finalOutput = c.output && (poly5 & 1) &&
                    (poly9Mode ? (poly9 & 1) : (poly17 & 1));
      break;
    case 1:  // 5-bit polynomial only
      finalOutput = c.output && (poly5 & 1);
      break;
    case 2:  // 5+4-bit polynomial
      finalOutput = c.output && (poly5 & 1) && (poly4 & 1);
      break;
    case 3:  // 5-bit polynomial
      finalOutput = c.output && (poly5 & 1);
      break;
    case 4:  // 17/9-bit polynomial only (white noise)
      finalOutput = c.output && (poly9Mode ? (poly9 & 1) : (poly17 & 1));
      break;
    case 5:  // Pure tone (no polynomial)
      finalOutput = c.output;
      break;
    case 6:  // 4-bit polynomial
      finalOutput = c.output && (poly4 & 1);
      break;
    case 7:  // Pure tone (no polynomial)
      finalOutput = c.output;
      break;
    }

    // Apply output
    int16_t channelOut = finalOutput ? (c.getVolume() * 2048) : 0;

    // High-pass filter for channels 1 and 2
    if (ch == 0 && ch1_highpass) {
      channelOut = channelOut - c.lastOutput;
    } else if (ch == 1 && ch2_highpass) {
      channelOut = channelOut - c.lastOutput;
    }
    c.lastOutput = channelOut;

    output += channelOut;
  }

  // Apply master volume and clamp
  output = (int32_t)(output * emuVolume);
  if (output > 32767) output = 32767;
  if (output < -32768) output = -32768;

  return (int16_t)output;
}

uint8_t POKEY::read(uint8_t addr) {
  addr &= 0x0F;

  switch (addr) {
  case POT0_R:
  case POT1_R:
  case POT2_R:
  case POT3_R:
  case POT4_R:
  case POT5_R:
  case POT6_R:
  case POT7_R:
    return pot[addr];

  case ALLPOT_R:
    return allpot;

  case KBCODE_R:
    return kbcode;

  case RANDOM_R:
    // Reading RANDOM updates polynomials
    updatePolynomials();
    return random;

  case SERIN_R:
    return serin;

  case IRQST_R:
    return irqst;

  case SKSTAT_R:
    return skstat;

  default:
    return 0xFF;
  }
}

void POKEY::write(uint8_t addr, uint8_t val) {
  addr &= 0x0F;

  switch (addr) {
  case AUDF1_W:
    channel[0].setFrequency(val);
    updateChannelPeriods();
    break;

  case AUDC1_W:
    channel[0].setControl(val);
    break;

  case AUDF2_W:
    channel[1].setFrequency(val);
    updateChannelPeriods();
    break;

  case AUDC2_W:
    channel[1].setControl(val);
    break;

  case AUDF3_W:
    channel[2].setFrequency(val);
    updateChannelPeriods();
    break;

  case AUDC3_W:
    channel[2].setControl(val);
    break;

  case AUDF4_W:
    channel[3].setFrequency(val);
    updateChannelPeriods();
    break;

  case AUDC4_W:
    channel[3].setControl(val);
    break;

  case AUDCTL_W:
    audctl = val;
    poly9Mode = (val & AUDCTL_POLY9) != 0;
    ch1_179mhz = (val & AUDCTL_CH1_179) != 0;
    ch3_179mhz = (val & AUDCTL_CH3_179) != 0;
    ch12_joined = (val & AUDCTL_CH1_CH2) != 0;
    ch34_joined = (val & AUDCTL_CH3_CH4) != 0;
    ch1_highpass = (val & AUDCTL_CH1_HPFILT) != 0;
    ch2_highpass = (val & AUDCTL_CH2_HPFILT) != 0;
    clock15khz = (val & AUDCTL_15KHZ) != 0;
    updateChannelPeriods();
    break;

  case STIMER_W:
    // Reset all audio channel timers
    for (int i = 0; i < 4; i++) {
      channel[i].divider = channel[i].period;
    }
    break;

  case SKREST_W:
    // Reset serial status register
    skstat = 0xFF;
    break;

  case POTGO_W:
    startPotScan();
    break;

  case SEROUT_W:
    serout = val;
    // Serial output complete interrupt
    if (irqen & IRQ_SERIAL_OUT) {
      irqst &= ~IRQ_SERIAL_OUT;
    }
    break;

  case IRQEN_W:
    irqen = val;
    // Update IRQST - any disabled interrupts become inactive
    irqst |= ~val;
    break;

  case SKCTL_W:
    skctl = val;
    // Writing 0 to SKCTL resets POKEY
    if (val == 0) {
      reset();
    }
    break;
  }
}

void POKEY::fillBuffer(uint16_t scanline) {
  // Calculate target samples for this scanline (PAL: 312 lines)
  uint16_t targetSampleIdx = (uint16_t)(((uint32_t)(scanline + 1) * NUMSAMPLESPERFRAME) / 312);
  if (targetSampleIdx > NUMSAMPLESPERFRAME) {
    targetSampleIdx = NUMSAMPLESPERFRAME;
  }

  while (actSampleIdx < targetSampleIdx) {
    samples[actSampleIdx++] = generateSample();
  }
}

void POKEY::playAudio() {
  if (sound) {
    sound->playAudio(samples, actSampleIdx * sizeof(int16_t));
  }
  actSampleIdx = 0;
}

void POKEY::setKeyCode(uint8_t code, bool pressed) {
  if (pressed) {
    kbcode = code;
    keyPressed = true;
    skstat &= ~SKSTAT_KEYDOWN;  // Active-low: key is down

    // Trigger keyboard interrupt if enabled
    if (irqen & IRQ_KEYPRESS) {
      irqst &= ~IRQ_KEYPRESS;
    }
  } else {
    keyPressed = false;
    skstat |= SKSTAT_KEYDOWN;  // Active-low: no key down
  }
}

void POKEY::setBreakKey(bool pressed) {
  if (pressed && (irqen & IRQ_BREAK)) {
    irqst &= ~IRQ_BREAK;
  }
}

void POKEY::setPaddle(uint8_t num, uint8_t value) {
  if (num < 8) {
    pot[num] = value;
  }
}

void POKEY::startPotScan() {
  // Start POT scan - all POTs start at 0 and count up
  allpot = 0xFF;
  // In a full implementation, this would start a timer
  // For simplicity, we immediately complete the scan
  allpot = 0x00;
}

bool POKEY::checkIRQ() {
  // IRQ is active if any enabled interrupt is pending
  // IRQST uses active-low logic
  return (irqst & irqen) != irqen;
}

void POKEY::acknowledgeIRQ(uint8_t mask) {
  irqst |= mask;
}

void POKEY::triggerTimerIRQ(uint8_t timer) {
  uint8_t mask = 0;
  switch (timer) {
  case 1: mask = IRQ_TIMER1; break;
  case 2: mask = IRQ_TIMER2; break;
  case 4: mask = IRQ_TIMER4; break;
  }
  if (irqen & mask) {
    irqst &= ~mask;
  }
}

void POKEY::tickTimers(uint8_t cycles) {
  // Timer underflow checking would go here
  // For now, simplified - timers checked during audio generation
}

uint8_t POKEY::getEmuVolume() const {
  return emuVolumeScaled;
}

void POKEY::setEmuVolume(uint8_t volume) {
  emuVolumeScaled = volume;
  emuVolume = volume / 128.0f;
}
