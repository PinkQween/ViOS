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
  ./build/isr80h/misc.o ./build/isr80h/isr80h.o ./build/isr80h/io.o ./build/isr80h/heap.o \
  ./build/isr80h/process.o \
  ./build/keyboard/keyboard.o ./build/keyboard/classic.o \
  ./build/loader/formats/elfloader.o ./build/loader/formats/elf.o

INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops \
        -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function \
        -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter \
        -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/boot.bin ./bin/kernel.bin user_programs
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin of=./bin/os.bin bs=512 conv=notrunc
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin
	sudo mount -t vfat ./bin/os.bin /mnt/d
	sudo cp -r ./assets/* /mnt/d
	find ./assets/programs -name '*.elf' -exec sudo cp {} /mnt/d/ \;
	sudo umount /mnt/d

./bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker-elf.ld -o ./build/kernelfull-elf.o -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

# Generic C and ASM file rules
./build/%.o: ./src/%.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/%.asm.o: ./src/%.asm
	mkdir -p $(dir $@)
	nasm -f elf -g $< -o $@

user_programs:
	@$(MAKE) -C ./assets/programs/stdlib || exit 1
	@for dir in ./assets/programs/*/ ; do \
		if [ "$$dir" != "./assets/programs/stdlib/" ]; then \
			$(MAKE) -C $$dir || exit 1; \
		fi \
	done

user_programs_clean:
	@for dir in ./assets/programs/*/ ; do \
		$(MAKE) -C $$dir clean || true; \
	done

clean: user_programs_clean
	rm -rf ./bin/boot.bin ./bin/kernel.bin ./bin/os.bin ./build/kernelfull.o $(FILES)
