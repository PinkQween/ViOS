FILES = ./build/start.asm.o ./build/vix_demo.o ./build/vios.asm.o ./build/vios.o ./build/stdlib.o ./build/stdio.o ./build/string.o ./build/memory.o ./build/start.o
INCLUDES = -I./src -I../stdlib/src
FLAGS = -g -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc -std=gnu99
STDLIB = ../stdlib

all: vix_demo.elf

vix_demo.elf: $(FILES)
	i686-elf-ld -m elf_i386 -relocatable $(FILES) -o ./vix_demo.elf

./build/start.asm.o: $(STDLIB)/src/start.asm
	mkdir -p $(dir $@)
	nasm -f elf $< -o $@

./build/vix_demo.o: ./src/vix_demo.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/vios.asm.o: $(STDLIB)/src/vios.asm
	mkdir -p $(dir $@)
	nasm -f elf $< -o $@

./build/vios.o: $(STDLIB)/src/vios.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/stdlib.o: $(STDLIB)/src/stdlib.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/stdio.o: $(STDLIB)/src/stdio.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/string.o: $(STDLIB)/src/string.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/memory.o: $(STDLIB)/src/memory.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

./build/start.o: $(STDLIB)/src/start.c
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

clean:
	rm -rf ./build/vix_demo.o ./build ./vix_demo.elf
