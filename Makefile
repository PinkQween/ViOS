.PHONY: all build clean configure help

BUILD_DIR := build
CMAKE_OPTS := 

ifdef CROSS_COMPILE
CMAKE_OPTS += -DCROSS_COMPILE=$(CROSS_COMPILE)
endif

all: build

configure:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_OPTS) ..

build: configure
	@cd $(BUILD_DIR) && $(MAKE)

clean:
	@if [ -d "$(BUILD_DIR)" ]; then cd $(BUILD_DIR) && $(MAKE) clean-all; fi
	@rm -rf $(BUILD_DIR)

help:
	@echo "ViOS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Configure and build (default)"
	@echo "  build      - Build the project"
	@echo "  configure  - Run CMake configuration"
	@echo "  clean      - Clean all build artifacts"
	@echo "  help       - Show this help"
	@echo ""
	@echo "Options:"
	@echo "  CROSS_COMPILE - Cross-compiler prefix (e.g., x86_64-elf-)"
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make CROSS_COMPILE=x86_64-linux-gnu-"
	@echo "  make clean"
