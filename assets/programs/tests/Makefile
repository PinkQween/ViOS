FILES=./build/tests.o
INCLUDES= -I../stdlib/src
FLAGS= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./build

all: prepare_dirs ${FILES}
	mkdir -p build
	i686-elf-gcc -g -T ./linker.ld -o ./tests.elf -ffreestanding -O0 -nostdlib -fpic -g ${FILES} ../stdlib/stdlib.elf

./build/tests.o: ./tests.c
	i686-elf-gcc ${INCLUDES}  -I./$(FLAGS) -std=gnu99 -c ./tests.c -o ./build/tests.o

clean:
	rm -rf ${FILES}
	rm ./tests.elf