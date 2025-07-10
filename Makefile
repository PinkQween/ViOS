FILES = \
  ./build/kernel.asm.o ./build/kernel.o \
  ./build/disk/disk.o ./build/disk/streamer.o \
  ./build/fs/pparser.o ./build/fs/file.o ./build/fs/fat/fat16.o \
  ./build/string/string.o \
  ./build/idt/idt.asm.o ./build/idt/idt.o \
  ./build/memory/memory.o \
  ./build/io/io.asm.o \
  ./build/gdt/gdt.o ./build/gdt/gdt.asm.o \
  ./build/task/tss.asm.o ./build/memory/heap/heap.o ./build/memory/heap/kheap.o \
  ./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o \
  ./build/task/process.o ./build/task/task.o ./build/task/task.asm.o \
  ./build/isr80h/isr80h.o ./build/isr80h/io.o ./build/isr80h/heap.o \
  ./build/isr80h/process.o \
  ./build/keyboard/keyboard.o ./build/keyboard/classic.o \
  ./build/loader/formats/elfloader.o ./build/loader/formats/elf.o \
  ./build/rtc/rtc.o ./build/terminal/terminal.o ./build/panic/panic.o \
  ./build/isr80h/file.o ./build/utils/utils.o ./build/keyboard/uart.o

INCLUDES = -I./src
CFLAGS  = -std=gnu99 -Wall -Werror -O0 -g
KFLAGS  = -ffreestanding -fno-builtin -nostdlib -nostartfiles -nodefaultlibs
EXTRAS  = -falign-jumps -falign-functions -falign-labels -falign-loops \
          -fstrength-reduce -fomit-frame-pointer -finline-functions \
          -Wno-unused-function -Wno-unused-label -Wno-cpp -Wno-unused-parameter
FLAGS   = $(CFLAGS) $(KFLAGS) $(EXTRAS) $(INCLUDES)

# Create bin and build directories first
.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./bin ./build

all: prepare_dirs ./bin/boot.bin ./bin/kernel.bin user_programs
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin of=./bin/os.bin bs=512 conv=notrunc
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin
	# Note: Filesystem mounting disabled for macOS compatibility
	# On macOS, the OS will boot but filesystem features may be limited
	# Mount operations commented out for macOS compatibility:
	# sudo mount -t vfat ./bin/os.bin /mnt/d
	# sudo cp -r ./assets/* /mnt/d
	# find ./assets/programs -name '*.elf' -exec sudo cp {} /mnt/d/ \;
	# sudo umount /mnt/d

./bin/kernel.bin: prepare_dirs $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./bin/boot.bin: prepare_dirs ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

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