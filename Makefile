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
  ./build/fs/fat/fat32.o \
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
  ./build/power/power.o \
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
	@sudo mkdir -p /mnt/d
	@sudo umount /mnt/d 2>/dev/null || true
	@sudo mount -t vfat ./bin/os.bin /mnt/d || { echo "Failed to mount disk image"; exit 1; }
	@if [ -d "./assets" ]; then sudo cp -r ./assets/* /mnt/d/ || { echo "Failed to copy assets"; sudo umount /mnt/d; exit 1; }; fi
	@if [ -d "./assets/etc/default/user/programs" ]; then find ./assets/etc/default/user/programs -name '*.elf' -exec sudo cp {} /mnt/d/ \; || { echo "Failed to copy .elf files"; sudo umount /mnt/d; exit 1; }; fi
	@sudo umount /mnt/d
	@echo "User programs and assets installed to disk image!"
else ifeq ($(UNAME_S),Darwin)
	@echo "Preparing disk image for macOS..."
	@bash -c "\
		set -e; \
		echo 'Attaching disk image...'; \
		ATTACH_OUTPUT=\$$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount ./bin/os.bin 2>/dev/null); \
		DISK_ID=\$$(echo \"\$$ATTACH_OUTPUT\" | grep -o '/dev/disk[0-9]*' | head -1); \
		PARTITION_ID=\"\$${DISK_ID}s1\"; \
		if [ -z \"\$$DISK_ID\" ]; then echo 'Failed to attach disk image'; exit 1; fi; \
		echo \"Attached as \$$DISK_ID, partition \$$PARTITION_ID\"; \
		echo 'Formatting partition as FAT32...'; \
		diskutil eraseVolume MS-DOS VIOSFAT32 \"\$$PARTITION_ID\" >/dev/null 2>&1 || { echo 'Failed to format partition'; hdiutil detach \"\$$DISK_ID\" 2>/dev/null; exit 1; }; \
		echo 'Copying files...'; \
		if [ -d './assets' ]; then \
			echo 'Copying assets...'; \
			cp -r ./assets/* /Volumes/VIOSFAT32/ || { echo 'Failed to copy assets'; diskutil unmount \"\$$PARTITION_ID\"; hdiutil detach \"\$$DISK_ID\" 2>/dev/null; exit 1; }; \
		else \
			echo 'No assets directory found, skipping...'; \
		fi; \
		if [ -d './assets/etc/default/user/programs' ]; then \
			echo 'Copying .elf files...'; \
			find ./assets/etc/default/user/programs -name '*.elf' -exec cp {} /Volumes/VIOSFAT32/ \; || { echo 'Failed to copy .elf files'; diskutil unmount \"\$$PARTITION_ID\"; hdiutil detach \"\$$DISK_ID\" 2>/dev/null; exit 1; }; \
		else \
			echo 'No programs directory found, skipping...'; \
		fi; \
		sync; \
		echo 'Unmounting...'; \
		diskutil unmount \"\$$PARTITION_ID\" >/dev/null 2>&1 || echo 'Unmount failed, continuing...'; \
		hdiutil detach \"\$$DISK_ID\" >/dev/null 2>&1 || echo 'Detach failed, continuing...'; \
		echo 'User programs and assets installed to disk image!'; \
	"
endif


./bin/kernel.bin: prepare_dirs $(FILES)
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib $(FILES)

./bin/boot.bin: prepare_dirs ./src/boot/mbr.asm  ./src/boot/vbr.asm
	nasm -f bin ./src/boot/mbr.asm -o ./bin/mbr.bin
	nasm -f bin ./src/boot/vbr.asm -o ./bin/vbr.bin
	dd if=./bin/mbr.bin of=$@ bs=512 count=1 conv=notrunc 2>/dev/null
	dd if=./bin/vbr.bin of=$@ bs=512 seek=2048 count=1 conv=notrunc 2>/dev/null

# Calculate kernel size and update boot sector
./bin/boot_with_size.bin: ./bin/boot.bin ./bin/kernel.bin
	@echo "Calculating kernel size and updating boot sector..."
	cp ./bin/boot.bin ./bin/boot_with_size.bin


./bin/os.bin: ./bin/boot_with_size.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot_with_size.bin of=./bin/os.bin bs=512 conv=notrunc
	dd if=./bin/kernel.bin of=./bin/os.bin bs=512 seek=2049 conv=notrunc
	#dd if=/dev/zero bs=1048576 count=128 >> ./bin/os.bin

# Generic C and ASM file rules
./build/%.o: ./src/%.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/%.asm.o: ./src/%.asm
	mkdir -p $(dir $@)
	nasm -f elf -g $< -o $@

user_programs:
	@if [ -d "./assets/etc/default/user/programs" ]; then \
		for dir in $$(find ./assets/etc/default/user/programs -mindepth 1 -maxdepth 1 -type d 2>/dev/null); do \
			echo "Building user program $$dir..."; \
			$(MAKE) -C $$dir all || exit 1; \
		done; \
	else \
		echo "No user programs directory found (./assets/etc/default/user/programs), skipping..."; \
	fi

user_programs_clean:
	@for dir in ./assets/etc/default/user/programs/*/ ; do \
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