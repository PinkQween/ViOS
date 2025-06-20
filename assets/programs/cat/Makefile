FILES = ./build/cat.o
INCLUDES = -I../stdlib/src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./build

all: prepare_dirs $(FILES)
	i686-elf-gcc -g -T ./linker.ld -o ./cat.elf -ffreestanding -O0 -nostdlib -fpic -g $(FILES) ../stdlib/stdlib.elf

./build/cat.o: ./src/cat.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/cat.c -o ./build/cat.o

clean:
	rm -rf $(FILES) ./build
	rm -f ./cat.elf