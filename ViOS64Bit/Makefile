FILES = \
  ./build/kernel.asm.o \
  ./build/kernel.o \
  ./build/string/string.o \
  ./build/memory/memory.o \
  ./build/memory/heap/kheap.o \
  ./build/memory/heap/heap.o \
  ./build/memory/paging/paging.asm.o \
  ./build/memory/paging/paging.o \
  ./build/memory/heap/multiheap.o \
  ./build/io/io.asm.o \
  ./build/idt/idt.o \
  ./build/idt/idt.asm.o \
  ./build/task/process.o \
  ./build/task/task.asm.o \
  ./build/task/task.o \
  ./build/fs/file.o \
  ./build/fs/fat/fat16.o \
  ./build/fs/pparser.o \
  ./build/disk/disk.o \
  ./build/disk/streamer.o \
  ./build/task/tss.asm.o \
  ./build/gdt/gdt.asm.o \
  ./build/gdt/gdt.o \
  ./build/keyboard/keyboard.o \
  ./build/keyboard/classic.o \
  ./build/isr80h/heap.o \
  ./build/isr80h/io.o \
  ./build/loader/formats/elf.o \
  ./build/loader/formats/elfloader.o \
  ./build/isr80h/isr80h.o \
  ./build/isr80h/misc.o \
  ./build/isr80h/process.o \
  ./build/idt/irq.o \
  ./build/disk/gpt.o \
  ./build/lib/vector/vector.o \
  ./build/graphics/graphics.o \
  ./build/graphics/image/image.o \
  ./build/graphics/image/bmp.o \
  ./build/graphics/font.o \
  ./build/graphics/terminal.o 

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

build: prepare_dirs data ./bin/kernel.elf ./bin/kernel.bin user_programs
	@echo "Build complete!"
	
data:
	@echo "Skipping data generation (utilities may not exist yet)"
	@# ./utilities/generateLogo.sh
	@# ./utilities/generateFonts.sh

install: ./bin/os.bin
ifeq ($(UNAME_S),Linux)
	@echo "Installing user programs and assets to disk image (Linux)..."
	@# Mount partition 2 using offset calculation
	@OFFSET=$$(sudo fdisk -l ./bin/os.bin | grep "os.bin2" | awk '{print $$2 * 512}'); \
	sudo mkdir -p /mnt/d; \
	sudo umount /mnt/d 2>/dev/null || true; \
	sudo mount -t vfat -o loop,offset=$$OFFSET ./bin/os.bin /mnt/d || { echo "Failed to mount partition 2"; exit 1; }; \
	echo "Copying ELF files..."; \
	find ./assets -name '*.elf' -exec sudo cp {} /mnt/d/ \; 2>/dev/null || true; \
	echo "Copying data files..."; \
	if [ -f "./data/images/bkground.bmp" ]; then sudo cp ./data/images/bkground.bmp /mnt/d/; fi; \
	if [ -f "./data/images/fonts/sysfont.bmp" ]; then sudo cp ./data/images/fonts/sysfont.bmp /mnt/d/; fi; \
	sudo umount /mnt/d; \
	echo "User programs and assets installed to disk image!"
else ifeq ($(UNAME_S),Darwin)
	@echo "Attaching disk image (macOS)..."
	@bash -c '\
		DISK_ID=$$(hdiutil attach -imagekey diskimage-class=CRawDiskImage ./bin/os.bin | grep "ViOS" | awk "{print \$$1}"); \
		if [ -z "$$DISK_ID" ]; then \
			echo "Error: Could not find partition 2 (ViOS data partition)"; \
			hdiutil detach /dev/$$(hdiutil info | grep "os.bin" | awk "{print \$$1}" | sed "s/\/dev\///") 2>/dev/null || true; \
			exit 1; \
		fi; \
		echo "Found data partition at $$DISK_ID"; \
		sudo mkdir -p /Volumes/viosmnt; \
		sudo mount -t msdos $$DISK_ID /Volumes/viosmnt || { echo "Failed to mount $$DISK_ID"; hdiutil detach $${DISK_ID%s*}; exit 1; }; \
		echo "Copying ELF files..."; \
		find ./assets -name "*.elf" -exec sudo cp {} /Volumes/viosmnt/ \; 2>/dev/null || true; \
		echo "Copying data files..."; \
		if [ -f "./data/images/bkground.bmp" ]; then sudo cp ./data/images/bkground.bmp /Volumes/viosmnt/; fi; \
		if [ -f "./data/images/fonts/sysfont.bmp" ]; then sudo cp ./data/images/fonts/sysfont.bmp /Volumes/viosmnt/; fi; \
		sync; \
		echo "Files on partition:"; \
		ls -lah /Volumes/viosmnt/; \
		echo "Unmounting..."; \
		sudo umount /Volumes/viosmnt; \
		hdiutil detach $${DISK_ID%s*}; \
		echo "User programs and assets installed to disk image!" \
	'
endif

mount: ./bin/os.bin
ifeq ($(UNAME_S),Linux)
	@echo "Installing user programs and assets to disk image (Linux)..."
	@sudo mkdir -p /mnt/d
	@sudo umount /mnt/d 2>/dev/null || true
	@sudo mount -t vfat ./bin/os.bin /mnt/d || { echo "Failed to mount disk image"; exit 1; }
else ifeq ($(UNAME_S),Darwin)
	@echo "Attaching disk image (macOS)..."
	@bash -c '\
		DISK_ID=$$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount ./bin/os.bin | awk "/\/dev\// {print \$$1}"); \
		echo "Attached as $$DISK_ID"; \
		sudo mkdir -p /Volumes/viosmnt; \
		sudo mount -t msdos $$DISK_ID /Volumes/viosmnt || { echo "Failed to mount $$DISK_ID"; exit 1; }; \
	'
endif


# Build kernel as raw binary (using linker script with OUTPUT_FORMAT(binary))
./bin/kernel.bin: prepare_dirs $(FILES)
	@echo "Building kernel binary..."
	x86_64-elf-gcc -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib $(FILES)

# Build kernel as ELF (create temporary linker script without OUTPUT_FORMAT(binary))
./bin/kernel.elf: prepare_dirs $(FILES)
	@echo "Building kernel ELF..."
	@sed 's/OUTPUT_FORMAT(binary)/OUTPUT_FORMAT(elf64-x86-64)/' ./src/linker.ld > ./src/linker.ld.elf
	x86_64-elf-gcc -T ./src/linker.ld.elf -o ./bin/kernel.elf -ffreestanding -O0 -nostdlib -g $(FILES)
	@rm ./src/linker.ld.elf

./bin/boot.bin: prepare_dirs ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

# Calculate kernel size and update boot sector
./bin/boot_with_size.bin: ./bin/boot.bin ./bin/kernel.bin
	@echo "Calculating kernel size and updating boot sector..."
	./utilities/updateBoot.sh


./bin/os.bin: ./bin/boot_with_size.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot_with_size.bin of=./bin/os.bin bs=512 conv=notrunc
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=1500 >> ./bin/os.bin

# Generic C and ASM file rules
./build/%.o: ./src/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/%.asm.o: ./src/%.asm
	mkdir -p $(dir $@)
	nasm -f elf64 -g $< -o $@

user_programs:
	@if [ -d "./assets" ]; then \
		echo "Building stdlib first..."; \
		$(MAKE) -C ./assets/stdlib all || exit 1; \
		for mk in $$(find ./assets -name Makefile \
			! -path "./assets/stdlib/*" \
			! -path "./assets/etc/default/user/programs/*" 2>/dev/null || true); do \
			dir=$$(dirname $$mk); \
			echo "Building in $$dir..."; \
			$(MAKE) -C $$dir all || exit 1; \
		done; \
	else \
		echo "No assets directory found, skipping user programs"; \
	fi

user_programs_clean:
	@for mk in $$(find ./assets -name Makefile \
		! -path "./assets/etc/default/user/programs/*"); do \
		dir=$$(dirname $$mk); \
		echo "Cleaning in $$dir..."; \
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
	x86_64-elf-gcc $(FLAGS) -T ./src/linker.ld.elf -o ./bin/kernel.elf.debug -ffreestanding -O0 -nostdlib -g $(FILES)
	@rm ./src/linker.ld.elf