/*
 ANTIC Display Tests
 Tests for ANTIC display list processor constants and structure
*/
#include "catch.hpp"
#include <cstdint>

// =============================================================================
// ANTIC CONSTANTS AND REGISTERS
// =============================================================================

TEST_CASE("ANTIC register addresses", "[antic][registers]") {
    SECTION("ANTIC lives at $D400-$D4FF") {
        const uint16_t ANTIC_BASE = 0xD400;
        const uint16_t ANTIC_END = 0xD4FF;

        REQUIRE(ANTIC_BASE == 0xD400);
        REQUIRE(ANTIC_END == 0xD4FF);
        REQUIRE((ANTIC_END - ANTIC_BASE + 1) == 256);
    }

    SECTION("Key ANTIC registers") {
        const uint16_t DMACTL = 0xD400;  // DMA control
        const uint16_t CHACTL = 0xD401;  // Character control
        const uint16_t DLISTL = 0xD402;  // Display list pointer low
        const uint16_t DLISTH = 0xD403;  // Display list pointer high
        const uint16_t HSCROL = 0xD404;  // Horizontal scroll
        const uint16_t VSCROL = 0xD405;  // Vertical scroll
        const uint16_t PMBASE = 0xD407;  // Player/missile base
        const uint16_t CHBASE = 0xD409;  // Character set base
        const uint16_t WSYNC = 0xD40A;   // Wait for horizontal sync
        const uint16_t VCOUNT = 0xD40B;  // Vertical line counter
        const uint16_t NMIEN = 0xD40E;   // NMI enable
        const uint16_t NMIRES = 0xD40F;  // NMI reset

        REQUIRE(DMACTL == 0xD400);
        REQUIRE(CHACTL == 0xD401);
        REQUIRE(DLISTL == 0xD402);
        REQUIRE(DLISTH == 0xD403);
        REQUIRE(HSCROL == 0xD404);
        REQUIRE(VSCROL == 0xD405);
        REQUIRE(PMBASE == 0xD407);
        REQUIRE(CHBASE == 0xD409);
        REQUIRE(WSYNC == 0xD40A);
        REQUIRE(VCOUNT == 0xD40B);
        REQUIRE(NMIEN == 0xD40E);
        REQUIRE(NMIRES == 0xD40F);
    }
}

// =============================================================================
// DISPLAY MODE CONSTANTS
// =============================================================================

TEST_CASE("ANTIC display modes", "[antic][modes]") {
    SECTION("Blank modes") {
        const uint8_t MODE_BLANK_1 = 0x00;
        const uint8_t MODE_BLANK_2 = 0x01;
        const uint8_t MODE_BLANK_4 = 0x02;
        const uint8_t MODE_BLANK_8 = 0x03;

        REQUIRE(MODE_BLANK_1 == 0x00);
        REQUIRE(MODE_BLANK_2 == 0x01);
        REQUIRE(MODE_BLANK_4 == 0x02);
        REQUIRE(MODE_BLANK_8 == 0x03);
    }

    SECTION("Text modes") {
        const uint8_t MODE_2 = 0x02;  // 40 chars, GR.0
        const uint8_t MODE_3 = 0x03;  // 40 chars, 10 scanlines
        const uint8_t MODE_4 = 0x04;  // 40 chars, multicolor
        const uint8_t MODE_5 = 0x05;  // 40 chars, 16 scanlines
        const uint8_t MODE_6 = 0x06;  // 20 chars, 5 colors
        const uint8_t MODE_7 = 0x07;  // 20 chars, 16 scanlines

        REQUIRE(MODE_2 == 0x02);
        REQUIRE(MODE_3 == 0x03);
        REQUIRE(MODE_4 == 0x04);
        REQUIRE(MODE_5 == 0x05);
        REQUIRE(MODE_6 == 0x06);
        REQUIRE(MODE_7 == 0x07);
    }

    SECTION("Graphics modes") {
        const uint8_t MODE_8 = 0x08;   // GR.3
        const uint8_t MODE_9 = 0x09;   // GR.4
        const uint8_t MODE_A = 0x0A;   // GR.5
        const uint8_t MODE_B = 0x0B;   // GR.6
        const uint8_t MODE_C = 0x0C;   // GR.14
        const uint8_t MODE_D = 0x0D;   // GR.7
        const uint8_t MODE_E = 0x0E;   // GR.15
        const uint8_t MODE_F = 0x0F;   // GR.8

        REQUIRE(MODE_8 == 0x08);
        REQUIRE(MODE_9 == 0x09);
        REQUIRE(MODE_A == 0x0A);
        REQUIRE(MODE_B == 0x0B);
        REQUIRE(MODE_C == 0x0C);
        REQUIRE(MODE_D == 0x0D);
        REQUIRE(MODE_E == 0x0E);
        REQUIRE(MODE_F == 0x0F);
    }
}

// =============================================================================
// DISPLAY LIST INSTRUCTIONS
// =============================================================================

TEST_CASE("Display list instruction bits", "[antic][displaylist]") {
    SECTION("Mode line bits") {
        const uint8_t DL_MODE_MASK = 0x0F;  // Lower 4 bits
        const uint8_t DL_DLI = 0x80;        // Display list interrupt
        const uint8_t DL_LMS = 0x40;        // Load memory scan
        const uint8_t DL_VSCROLL = 0x20;    // Vertical scroll
        const uint8_t DL_HSCROLL = 0x10;    // Horizontal scroll

        REQUIRE(DL_MODE_MASK == 0x0F);
        REQUIRE(DL_DLI == 0x80);
        REQUIRE(DL_LMS == 0x40);
        REQUIRE(DL_VSCROLL == 0x20);
        REQUIRE(DL_HSCROLL == 0x10);

        // Test combining flags
        uint8_t mode_with_dli = 0x02 | DL_DLI;
        REQUIRE(mode_with_dli == 0x82);

        uint8_t mode_with_lms = 0x02 | DL_LMS;
        REQUIRE(mode_with_lms == 0x42);

        uint8_t mode_with_all = 0x02 | DL_DLI | DL_LMS | DL_VSCROLL | DL_HSCROLL;
        REQUIRE(mode_with_all == 0xF2);
    }

    SECTION("Extract mode from instruction") {
        auto getMode = [](uint8_t instruction) -> uint8_t {
            return instruction & 0x0F;
        };

        REQUIRE(getMode(0x02) == 0x02);
        REQUIRE(getMode(0x42) == 0x02);  // Mode 2 with LMS
        REQUIRE(getMode(0x82) == 0x02);  // Mode 2 with DLI
        REQUIRE(getMode(0xF2) == 0x02);  // Mode 2 with all flags
    }

    SECTION("Check DLI bit") {
        auto hasDLI = [](uint8_t instruction) -> bool {
            return (instruction & 0x80) != 0;
        };

        REQUIRE(hasDLI(0x82) == true);
        REQUIRE(hasDLI(0x02) == false);
        REQUIRE(hasDLI(0xFF) == true);
    }

    SECTION("Check LMS bit") {
        auto hasLMS = [](uint8_t instruction) -> bool {
            return (instruction & 0x40) != 0;
        };

        REQUIRE(hasLMS(0x42) == true);
        REQUIRE(hasLMS(0x02) == false);
        REQUIRE(hasLMS(0xFF) == true);
    }
}

// =============================================================================
// SCREEN RESOLUTION CONSTANTS
// =============================================================================

TEST_CASE("Atari screen dimensions", "[antic][screen]") {
    SECTION("Standard NTSC dimensions") {
        const uint16_t ATARI_WIDTH = 384;
        const uint16_t ATARI_HEIGHT = 240;

        REQUIRE(ATARI_WIDTH == 384);
        REQUIRE(ATARI_HEIGHT == 240);
        REQUIRE((ATARI_WIDTH * ATARI_HEIGHT) == 92160); // Total pixels
    }

    SECTION("PAL dimensions (if supported)") {
        const uint16_t ATARI_WIDTH_PAL = 384;
        const uint16_t ATARI_HEIGHT_PAL = 288;

        REQUIRE(ATARI_WIDTH_PAL == 384);
        REQUIRE(ATARI_HEIGHT_PAL == 288);
    }

    SECTION("Playfield dimensions") {
        // Normal playfield
        const uint16_t PLAYFIELD_NORMAL = 256;
        // Wide playfield
        const uint16_t PLAYFIELD_WIDE = 320;
        // Full width (with borders)
        const uint16_t PLAYFIELD_FULL = 384;

        REQUIRE(PLAYFIELD_NORMAL == 256);
        REQUIRE(PLAYFIELD_WIDE == 320);
        REQUIRE(PLAYFIELD_FULL == 384);
    }
}

// =============================================================================
// DMACTL REGISTER BITS
// =============================================================================

TEST_CASE("DMACTL register", "[antic][dmactl]") {
    SECTION("DMA control bits") {
        const uint8_t DMACTL_DL = 0x20;        // Enable display list DMA
        const uint8_t DMACTL_PM_1LINE = 0x08;  // Player/missile 1-line resolution
        const uint8_t DMACTL_PM_2LINE = 0x0C;  // Player/missile 2-line resolution
        const uint8_t DMACTL_PLAYFIELD_NARROW = 0x01;
        const uint8_t DMACTL_PLAYFIELD_NORMAL = 0x02;
        const uint8_t DMACTL_PLAYFIELD_WIDE = 0x03;

        REQUIRE(DMACTL_DL == 0x20);
        REQUIRE(DMACTL_PM_1LINE == 0x08);
        REQUIRE(DMACTL_PM_2LINE == 0x0C);
        REQUIRE(DMACTL_PLAYFIELD_NARROW == 0x01);
        REQUIRE(DMACTL_PLAYFIELD_NORMAL == 0x02);
        REQUIRE(DMACTL_PLAYFIELD_WIDE == 0x03);
    }

    SECTION("DMACTL combinations") {
        // Enable display list with normal playfield
        uint8_t dmactl = 0x20 | 0x02;
        REQUIRE(dmactl == 0x22);

        // Enable display list with wide playfield and player/missile graphics
        dmactl = 0x20 | 0x03 | 0x08;
        REQUIRE(dmactl == 0x2B);
    }
}

// =============================================================================
// NMI (NON-MASKABLE INTERRUPT) TESTS
// =============================================================================

TEST_CASE("ANTIC NMI sources", "[antic][nmi]") {
    SECTION("NMI enable bits") {
        const uint8_t NMIEN_VBI = 0x40;  // Vertical blank interrupt
        const uint8_t NMIEN_DLI = 0x80;  // Display list interrupt

        REQUIRE(NMIEN_VBI == 0x40);
        REQUIRE(NMIEN_DLI == 0x80);

        // Enable both
        uint8_t nmien = NMIEN_VBI | NMIEN_DLI;
        REQUIRE(nmien == 0xC0);
    }

    SECTION("NMI status bits") {
        const uint8_t NMIST_VBI = 0x40;  // VBI occurred
        const uint8_t NMIST_DLI = 0x80;  // DLI occurred

        REQUIRE(NMIST_VBI == 0x40);
        REQUIRE(NMIST_DLI == 0x80);
    }
}

// =============================================================================
// CHARACTER SET TESTS
// =============================================================================

TEST_CASE("Character set configuration", "[antic][charset]") {
    SECTION("Character set base calculation") {
        // CHBASE register specifies upper byte of character set address
        // Character set must be on 1K boundary ($XX00)
        auto charsetAddress = [](uint8_t chbase) -> uint16_t {
            return static_cast<uint16_t>(chbase) << 8;
        };

        REQUIRE(charsetAddress(0xE0) == 0xE000);
        REQUIRE(charsetAddress(0xF0) == 0xF000);
        REQUIRE(charsetAddress(0x10) == 0x1000);
    }

    SECTION("Character definitions") {
        // Each character is 8 bytes (8x8 bitmap)
        const uint8_t CHAR_HEIGHT = 8;
        const uint8_t BYTES_PER_CHAR = 8;
        const uint16_t CHARS_IN_SET = 128;  // Standard set
        const uint16_t CHARSET_SIZE = CHARS_IN_SET * BYTES_PER_CHAR;

        REQUIRE(CHAR_HEIGHT == 8);
        REQUIRE(BYTES_PER_CHAR == 8);
        REQUIRE(CHARS_IN_SET == 128);
        REQUIRE(CHARSET_SIZE == 1024);
    }
}

// =============================================================================
// HORIZONTAL AND VERTICAL SCROLL
// =============================================================================

TEST_CASE("Scrolling", "[antic][scroll]") {
    SECTION("Horizontal scroll range") {
        // HSCROL register: 0-15
        const uint8_t HSCROL_MIN = 0;
        const uint8_t HSCROL_MAX = 15;

        REQUIRE(HSCROL_MIN == 0);
        REQUIRE(HSCROL_MAX == 15);
    }

    SECTION("Vertical scroll range") {
        // VSCROL register: 0-15
        const uint8_t VSCROL_MIN = 0;
        const uint8_t VSCROL_MAX = 15;

        REQUIRE(VSCROL_MIN == 0);
        REQUIRE(VSCROL_MAX == 15);
    }
}

// =============================================================================
// PLAYER/MISSILE GRAPHICS
// =============================================================================

TEST_CASE("Player/Missile graphics", "[antic][pm]") {
    SECTION("Player/Missile base") {
        // PMBASE specifies upper byte of P/M graphics area
        // Must be on page boundary
        auto pmAddress = [](uint8_t pmbase) -> uint16_t {
            return static_cast<uint16_t>(pmbase) << 8;
        };

        REQUIRE(pmAddress(0x20) == 0x2000);
        REQUIRE(pmAddress(0x40) == 0x4000);
    }

    SECTION("Player/Missile sizes") {
        // Single line resolution: 2K for P/M graphics
        const uint16_t PM_SIZE_SINGLE = 2048;
        // Double line resolution: 1K for P/M graphics
        const uint16_t PM_SIZE_DOUBLE = 1024;

        REQUIRE(PM_SIZE_SINGLE == 2048);
        REQUIRE(PM_SIZE_DOUBLE == 1024);
    }
}

// =============================================================================
// WSYNC (WAIT FOR SYNC) TESTS
// =============================================================================

TEST_CASE("WSYNC behavior", "[antic][wsync]") {
    SECTION("WSYNC register") {
        const uint16_t WSYNC_ADDR = 0xD40A;
        REQUIRE(WSYNC_ADDR == 0xD40A);

        // Writing any value to WSYNC halts CPU until horizontal sync
        // Reading returns undefined value
        // This is used for raster effects and timing-critical code
    }
}

// =============================================================================
// VCOUNT (VERTICAL COUNTER) TESTS
// =============================================================================

TEST_CASE("VCOUNT register", "[antic][vcount]") {
    SECTION("VCOUNT range") {
        const uint16_t VCOUNT_ADDR = 0xD40B;
        const uint8_t VCOUNT_MIN_NTSC = 0;
        const uint8_t VCOUNT_MAX_NTSC = 131;  // Approximately

        REQUIRE(VCOUNT_ADDR == 0xD40B);
        REQUIRE(VCOUNT_MIN_NTSC == 0);
        REQUIRE(VCOUNT_MAX_NTSC == 131);
    }
}

// Note: Full ANTIC behavior testing requires integration with display driver
// and GTIA. These tests verify constants and basic calculations.
