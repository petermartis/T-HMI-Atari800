# T-HMI-Atari800 Makefile
# Atari 800 XL Emulator for Lilygo T-HMI
#
# Based on T-HMI-C64 build system

# Board selection: T_HMI, T_DISPLAY_S3, WAVESHARE
BOARD ?= T_HMI

# Keyboard type: BLE_KEYBOARD, WEB_KEYBOARD
KEYBOARD ?= WEB_KEYBOARD

# Arduino CLI path
ARDUINO_CLI ?= arduino-cli

# ESP32 Arduino core version
ESP32_CORE_VERSION = esp32:esp32@3.2.0

# Project name
PROJECT = T-HMI-Atari800

# Output directory
BUILD_DIR = build-$(BOARD)-$(KEYBOARD)

# Board-specific settings
ifeq ($(BOARD),T_HMI)
    FQBN = esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=huge_app,FlashSize=16M
    BUILD_FLAGS = -DBOARD_T_HMI
else ifeq ($(BOARD),T_DISPLAY_S3)
    FQBN = esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=huge_app,FlashSize=16M
    BUILD_FLAGS = -DBOARD_T_DISPLAY_S3
else ifeq ($(BOARD),WAVESHARE)
    FQBN = esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=app3M_fat9M_16MB,FlashSize=16M
    BUILD_FLAGS = -DBOARD_WAVESHARE
else
    $(error Unknown BOARD: $(BOARD). Use T_HMI, T_DISPLAY_S3, or WAVESHARE)
endif

# Keyboard settings
ifeq ($(KEYBOARD),BLE_KEYBOARD)
    BUILD_FLAGS += -DUSE_BLE_KEYBOARD
else ifeq ($(KEYBOARD),WEB_KEYBOARD)
    BUILD_FLAGS += -DUSE_WEB_KEYBOARD
else
    $(error Unknown KEYBOARD: $(KEYBOARD). Use BLE_KEYBOARD or WEB_KEYBOARD)
endif

# Add ESP32 define for Arduino compatibility
BUILD_FLAGS += -DESP32

# Serial port (auto-detect or override)
PORT ?= $(shell ls /dev/ttyACM* 2>/dev/null | head -1)
ifeq ($(PORT),)
    PORT := $(shell ls /dev/ttyUSB* 2>/dev/null | head -1)
endif

# Upload baud rate
UPLOAD_SPEED ?= 921600

.PHONY: all compile upload clean monitor install-core install-libs help test test-cpu test-memory test-all test-clean test-verbose

# Default target
all: compile

# Help target
help:
	@echo "T-HMI-Atari800 Build System"
	@echo "==========================="
	@echo ""
	@echo "Usage: make [target] [BOARD=board] [KEYBOARD=keyboard]"
	@echo ""
	@echo "Targets:"
	@echo "  compile      - Compile the project (default)"
	@echo "  upload       - Upload to device"
	@echo "  clean        - Clean build files"
	@echo "  monitor      - Open serial monitor"
	@echo "  install-core - Install ESP32 Arduino core"
	@echo "  install-libs - Install required Arduino libraries"
	@echo "  test         - Build and run all tests"
	@echo "  test-cpu     - Run CPU emulation tests"
	@echo "  test-memory  - Run memory system tests"
	@echo "  test-verbose - Run tests with verbose output"
	@echo "  test-clean   - Clean test build files"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Board options:"
	@echo "  T_HMI        - Lilygo T-HMI (default)"
	@echo "  T_DISPLAY_S3 - Lilygo T-Display S3 AMOLED"
	@echo "  WAVESHARE    - Waveshare ESP32-S3-LCD-2.8"
	@echo ""
	@echo "Keyboard options:"
	@echo "  WEB_KEYBOARD - Web-based keyboard (default)"
	@echo "  BLE_KEYBOARD - Bluetooth LE keyboard"
	@echo ""
	@echo "Examples:"
	@echo "  make BOARD=T_HMI KEYBOARD=WEB_KEYBOARD"
	@echo "  make upload PORT=/dev/ttyACM0"

# Install ESP32 Arduino core
install-core:
	$(ARDUINO_CLI) core update-index
	$(ARDUINO_CLI) core install $(ESP32_CORE_VERSION)

# Install required libraries
install-libs:
	@echo "Installing required libraries..."
	$(ARDUINO_CLI) lib install "ESPAsyncWebServer-esphome"
	$(ARDUINO_CLI) lib install "AsyncTCP"
	$(ARDUINO_CLI) lib install "ArduinoJson"
	@echo "Installing ESPAsyncDNSServer from GitHub..."
	$(ARDUINO_CLI) lib install --git-url https://github.com/devyte/ESPAsyncDNSServer.git || \
		(mkdir -p ~/Arduino/libraries && cd ~/Arduino/libraries && git clone https://github.com/devyte/ESPAsyncDNSServer.git 2>/dev/null || true)
	@echo "Libraries installed."

# Compile the project
compile:
	@echo "Building $(PROJECT) for $(BOARD) with $(KEYBOARD)..."
	@mkdir -p $(BUILD_DIR)
	$(ARDUINO_CLI) compile \
		--fqbn $(FQBN) \
		--build-property "build.extra_flags=$(BUILD_FLAGS)" \
		--output-dir $(BUILD_DIR) \
		$(PROJECT).ino

# Upload to device
upload: compile
	@echo "Uploading to $(PORT)..."
	$(ARDUINO_CLI) upload \
		--fqbn $(FQBN) \
		--port $(PORT) \
		--input-dir $(BUILD_DIR)

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -rf build-*

# Serial monitor
monitor:
	$(ARDUINO_CLI) monitor --port $(PORT) --config baudrate=115200

# Docker-based compile (for reproducible builds)
podcompile:
	@echo "Building with Docker..."
	docker run --rm -v $(PWD):/workspace -w /workspace \
		arduino/arduino-cli:latest \
		compile --fqbn $(FQBN) \
		--build-property "build.extra_flags=$(BUILD_FLAGS)" \
		--output-dir $(BUILD_DIR) \
		$(PROJECT).ino

# Format source code
format:
	find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Generate documentation
docs:
	doxygen Doxyfile

# =============================================================================
# TESTING TARGETS
# =============================================================================

# Test configuration
TEST_DIR = test
TEST_BUILD_DIR = test/build
TEST_RUNNER = $(TEST_BUILD_DIR)/test_runner
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src -I./test
LDFLAGS = -lpthread

# Test source files
TEST_SOURCES = $(TEST_DIR)/test_main.cpp \
               $(TEST_DIR)/test_cpu6502.cpp \
               $(TEST_DIR)/test_memory.cpp \
               $(TEST_DIR)/test_antic.cpp

# CPU source files needed for testing
CPU_SOURCES = src/CPU6502.cpp

# Build and run all tests
test: $(TEST_RUNNER)
	@echo "Running all tests..."
	@$(TEST_RUNNER)

# Build test runner
$(TEST_RUNNER): $(TEST_SOURCES) $(CPU_SOURCES)
	@echo "Building test suite..."
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) $(CPU_SOURCES) -o $(TEST_RUNNER) $(LDFLAGS)

# Run only CPU tests
test-cpu: $(TEST_RUNNER)
	@echo "Running CPU tests..."
	@$(TEST_RUNNER) "[cpu]"

# Run only memory tests
test-memory: $(TEST_RUNNER)
	@echo "Running memory tests..."
	@$(TEST_RUNNER) "[memory]"

# Run only ANTIC tests
test-antic: $(TEST_RUNNER)
	@echo "Running ANTIC tests..."
	@$(TEST_RUNNER) "[antic]"

# Run all tests with verbose output
test-verbose: $(TEST_RUNNER)
	@echo "Running tests with verbose output..."
	@$(TEST_RUNNER) -s

# Run tests with success/failure summary
test-all: $(TEST_RUNNER)
	@echo "Running comprehensive test suite..."
	@$(TEST_RUNNER) --reporter console

# Clean test build files
test-clean:
	@echo "Cleaning test build files..."
	@rm -rf $(TEST_BUILD_DIR)

# Coverage report (requires gcov/lcov)
test-coverage: CXXFLAGS += --coverage -fprofile-arcs -ftest-coverage
test-coverage: LDFLAGS += --coverage
test-coverage: test-clean $(TEST_RUNNER)
	@echo "Running tests with coverage..."
	@$(TEST_RUNNER)
	@echo "Generating coverage report..."
	@lcov --capture --directory . --output-file coverage.info 2>/dev/null || echo "lcov not installed, skipping coverage"
	@lcov --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.info 2>/dev/null || true
	@genhtml coverage.info --output-directory test/coverage 2>/dev/null || echo "genhtml not installed, skipping HTML report"
	@echo "Coverage report generated in test/coverage/index.html"
