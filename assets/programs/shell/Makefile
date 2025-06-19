FILES=./build/shell.o
INCLUDES= -I../stdlib/src
FLAGS= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./build

all: prepare_dirs ${FILES}
	mkdir -p build
	i686-elf-gcc -g -T ./linker.ld -o ./shell.elf -ffreestanding -O0 -nostdlib -fpic -g ${FILES} ../stdlib/stdlib.elf

./build/shell.o: ./src/shell.c
	i686-elf-gcc ${INCLUDES} -I./$(FLAGS) -std=gnu99 -c ./src/shell.c -o ./build/shell.o

clean:
	rm -rf ${FILES} ./build
	rm ./shell.elf