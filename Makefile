FILES = \
  ./build/kernel.asm.o \
  ./build/kernel.o \
  ./build/disk/disk.o \
  ./build/disk/streamer.o \
  ./build/fs/pparser.o \
  ./build/fs/file.o \
  ./build/fs/fat/fat16.o \
  ./build/string/string.o \
  ./build/idt/idt.asm.o \
  ./build/idt/idt.o \
  ./build/memory/memory.o \
  ./build/io/io.asm.o \
  ./build/gdt/gdt.o \
  ./build/gdt/gdt.asm.o \
  ./build/task/tss.asm.o \
  ./build/memory/heap/heap.o \
  ./build/memory/heap/kheap.o \
  ./build/memory/paging/paging.o \
  ./build/memory/paging/paging.asm.o \
  ./build/task/process.o \
  ./build/task/task.o \
  ./build/task/task.asm.o \
  ./build/isr80h/isr80h.o \
  ./build/isr80h/io.o \
  ./build/isr80h/heap.o \
  ./build/isr80h/process.o \
  ./build/isr80h/audio.o \
  ./build/keyboard/keyboard.o \
  ./build/keyboard/ps2_keyboard.o \
  ./build/loader/formats/elfloader.o \
  ./build/loader/formats/elf.o \
  ./build/rtc/rtc.o \
  ./build/panic/panic.o \
  ./build/isr80h/file.o \
  ./build/utils/utils.o \
  ./build/graphics/graphics.o \
  ./build/fonts/characters_Arial.o \
  ./build/fonts/characters_AtariST8x16SystemFont.o \
  ./build/fonts/characters_Brightly.o \
  ./build/fonts/characters_Cheri.o \
  ./build/fonts/characters_RobotoThin.o \
  ./build/mouse/mouse.o \
  ./build/mouse/ps2_mouse.o \
  ./build/math/fpu_math.o \
  ./build/debug/serial.o \
  ./build/debug/klog.o \
  ./build/audio/sb16.o \
  ./build/audio/audio.o

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
	mkdir -p ./bin ./build ./mnt/d

all: prepare_dirs ./bin/boot.bin ./bin/kernel.bin user_programs install

install: ./bin/os.bin
ifeq ($(UNAME_S),Linux)
	mformat -i ./bin/os.bin@@1M -F
    mcopy -i ./bin/os.bin@@1M -s ./assets/* ::/
    find ./assets/programs -name '*.elf' -exec mcopy -i ./bin/os.bin@@1M {} ::/ \;
else ifeq ($(UNAME_S),Darwin)
	@echo "Mounting FAT16 image on macOS..."
	@hdiutil attach -imagekey diskimage-class=CRawDiskImage ./bin/os.bin -mountpoint ./mnt/d || \
		(echo "Failed to mount. Trying manual attach..."; \
		hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount ./bin/os.bin; \
		diskutil list; \
		echo "Please mount the correct partition manually if needed.")
	cp -r ./assets/* ./mnt/d || true
	find ./assets/programs -name '*.elf' -exec cp {} ./mnt/d/ \; || true
	@hdiutil detach ./mnt/d || true
else
	@echo "Skipping install (switch to linux or darwin)"
endif

./bin/kernel.elf: prepare_dirs $(FILES)
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.elf -ffreestanding -O0 -nostdlib $(FILES)

./bin/kernel.bin: ./bin/kernel.elf
	i686-elf-objcopy -O binary ./bin/kernel.elf ./bin/kernel.bin

./bin/boot.bin: prepare_dirs ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./bin/os.bin: ./bin/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin of=./bin/os.bin bs=512 conv=notrunc
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
	@$(MAKE) -C ./assets/programs/stdlib all
	@{ \
		for dir in $(shell find ./assets/programs -mindepth 1 -maxdepth 1 -type d -not -name "stdlib"); do \
			echo "Building $$dir..."; \
			$(MAKE) -C $$dir all || exit 1; \
		done; \
	}

user_programs_clean:
	@for dir in ./assets/programs/*/ ; do \
		$(MAKE) -C $$dir clean || true; \
	done

clean: user_programs_clean
	rm -rf ./bin/boot.bin ./bin/kernel.bin ./bin/os.bin ./build/kernelfull.o ./build/kernelfull-elf.o $(FILES) ./bin/ ./build/