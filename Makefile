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

.PHONY: all compile upload clean monitor install-core help

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
