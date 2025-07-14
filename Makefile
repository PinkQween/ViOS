

# Object files grouped by functionality
FILES = \
  ./build/kernel.asm.o \
  ./build/kernel.o \
  ./build/kernel/init.o \
  ./build/kernel/mainloop.o \
  ./build/graphics/renderer.o \
  ./build/graphics/graphics.o \
  ./build/disk/disk.o \
  ./build/disk/streamer.o \
  ./build/fs/pparser.o \
  ./build/fs/file.o \
  ./build/fs/fat/fat16.o \
  ./build/string/string.o \
  ./build/idt/idt.asm.o \
  ./build/idt/idt.o \
  ./build/gdt/gdt.o \
  ./build/gdt/gdt.asm.o \
  ./build/memory/memory.o \
  ./build/memory/heap/heap.o \
  ./build/memory/heap/kheap.o \
  ./build/memory/paging/paging.o \
  ./build/memory/paging/paging.asm.o \
  ./build/io/io.asm.o \
  ./build/task/tss.asm.o \
  ./build/task/process.o \
  ./build/task/task.o \
  ./build/task/task.asm.o \
  ./build/isr80h/isr80h.o \
  ./build/isr80h/heap.o \
  ./build/isr80h/process.o \
  ./build/isr80h/file.o \
  ./build/isr80h/serial.o \
  ./build/isr80h/waits.o \
  ./build/isr80h/keyboard.o \
  ./build/keyboard/keyboard.o \
  ./build/keyboard/ps2_keyboard.o \
  ./build/loader/formats/elfloader.o \
  ./build/loader/formats/elf.o \
  ./build/rtc/rtc.o \
  ./build/panic/panic.o \
  ./build/utils/utils.o \
  ./build/fonts/characters_Arial.o \
  ./build/fonts/characters_AtariST8x16SystemFont.o \
  ./build/fonts/characters_Brightly.o \
  ./build/fonts/characters_Cheri.o \
  ./build/fonts/characters_RobotoThin.o \
  ./build/mouse/mouse.o \
  ./build/mouse/ps2_mouse.o \
  ./build/math/fpu_math.o \
  ./build/debug/simple_serial.o \
  ./build/audio/audio.o \
  ./build/audio/sb16.o

INCLUDES = -I./src
CFLAGS  = -std=gnu99 -Wall -Werror -O0 -g
KFLAGS  = -ffreestanding -fno-builtin -nostdlib -nostartfiles -nodefaultlibs
EXTRAS  = -falign-jumps -falign-functions -falign-labels -falign-loops \
          -fstrength-reduce -fomit-frame-pointer -finline-functions \
          -Wno-unused-function -Wno-unused-label -Wno-cpp -Wno-unused-parameter
FLAGS   = $(CFLAGS) $(KFLAGS) $(EXTRAS) $(INCLUDES)
UNAME_S := $(shell uname -s)

# Create bin and build directories first
.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./bin ./build

all: build install

build: prepare_dirs fonts ./bin/boot_with_size.bin ./bin/kernel.bin user_programs ./bin/os.bin
	@echo "Build complete! User programs built but not installed to disk image."
	@echo "To install user programs to disk image, run: make install"

fonts:
	./generateFonts.sh

install: ./bin/os.bin
ifeq ($(UNAME_S),Linux)
	@echo "Installing user programs and assets to disk image (Linux)..."
	@# Create mount point if it doesn't exist
	@sudo mkdir -p /mnt/d
	@# Unmount if already mounted
	@sudo umount /mnt/d 2>/dev/null || true
	@# Mount the disk image
	@sudo mount -t vfat ./bin/os.bin /mnt/d
	@# Copy all assets recursively
	@sudo cp -r ./assets/* /mnt/d/ 2>/dev/null || true
	@# Find and copy all .elf files from programs directory
	@find ./assets/programs -name '*.elf' -exec sudo cp {} /mnt/d/ \;
	@# Unmount the disk image
	@sudo umount /mnt/d
	@echo "User programs and assets installed to disk image!"
else ifeq ($(UNAME_S),Darwin)
	@echo "Attaching disk image (macOS)..."
	@bash -c '\
		DISK_ID=$$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount ./bin/os.bin | awk "/\/dev\// {print \$$1}"); \
		echo "Attached as $$DISK_ID"; \
		sudo mkdir -p /Volumes/viosmnt; \
		sudo mount -t msdos $$DISK_ID /Volumes/viosmnt || { echo "Failed to mount $$DISK_ID"; exit 1; }; \
		echo "Copying files..."; \
		sudo cp -r ./assets/* /Volumes/viosmnt/ 2>/dev/null || true; \
		find ./assets/programs -name "*.elf" -exec sudo cp {} /Volumes/viosmnt/ \;; \
		sync; \
		echo "Unmounting..."; \
		sudo umount /Volumes/viosmnt; \
		hdiutil detach $$DISK_ID; \
		echo "User programs and assets installed to disk image!" \
	'
endif
	@echo "User programs and assets installed to disk image!"

./bin/kernel.bin: prepare_dirs $(FILES)
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib $(FILES)

./bin/boot.bin: prepare_dirs ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

# Calculate kernel size and update boot sector
./bin/boot_with_size.bin: ./bin/boot.bin ./bin/kernel.bin
	@echo "Calculating kernel size and updating boot sector..."
	./update_boot.sh

./bin/os.bin: ./bin/boot_with_size.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot_with_size.bin of=./bin/os.bin bs=512 conv=notrunc
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=128 >> ./bin/os.bin

# Generic C and ASM file rules
./build/%.o: ./src/%.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/%.asm.o: ./src/%.asm
	mkdir -p $(dir $@)
	nasm -f elf -g $< -o $@

user_programs:
	@{ \
		for dir in $(shell find ./assets/programs -mindepth 1 -maxdepth 1 -type d); do \
			echo "Building user program $$dir..."; \
			$(MAKE) -C $$dir all || exit 1; \
		done; \
	}

user_programs_clean:
	@for dir in ./assets/programs/*/ ; do \
		$(MAKE) -C $$dir clean || true; \
	done

clean: user_programs_clean clean_mounts
	rm -rf ./bin/boot.bin ./bin/boot_with_size.bin ./bin/kernel.bin ./bin/os.bin ./build/kernelfull.o ./build/kernelfull-elf.o $(FILES) ./bin/ ./build/ ./src/fonts

# Clean up any mounted disk images (macOS specific)
clean_mounts:
ifeq ($(UNAME_S),Darwin)
	@echo "Cleaning up any mounted disk images..."
	@# Detach any mount at our mount point
	@hdiutil detach ./mnt/d 2>/dev/null || true
	@rmdir ./mnt/d 2>/dev/null || true
	@# Find and detach all ViOS disk images
	@echo "Looking for ViOS disk images to detach..."
	@for disk in $$(diskutil list | grep -E "VIOS BOOT|disk[0-9]+" | grep -o "disk[0-9]\+" | sort -u); do \
		echo "Detaching $$disk..."; \
		hdiutil detach /dev/$$disk 2>/dev/null || true; \
	done
	@echo "Mount cleanup completed."
else
	@echo "clean_mounts is only available on macOS"
endif

# Build kernel ELF with symbols for debugging (separate target)
./bin/kernel.elf.debug: prepare_dirs $(FILES)
	@echo "Building kernel ELF with debug symbols..."
	@# Create a temporary linker script without OUTPUT_FORMAT(binary) for ELF output
	@cp ./src/linker.ld ./src/linker.ld.tmp
	@sed 's/OUTPUT_FORMAT(binary)/\/\* OUTPUT_FORMAT(binary) \*\//' ./src/linker.ld > ./src/linker.ld.elf
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld.elf -o ./bin/kernel.elf.debug -ffreestanding -O0 -nostdlib -g $(FILES)
	@rm ./src/linker.ld.elf