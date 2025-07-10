FILES=./build/simplecheck.o
INCLUDES= -I../stdlib/src
FLAGS= -g -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./build

all: prepare_dirs ${FILES}
	mkdir -p build
	i686-elf-gcc -g -T ./linker.ld -o ./simplecheck.elf -O0 -nostdlib -fpic -g ${FILES} ../stdlib/stdlib.elf

./build/simplecheck.o: ./simplecheck.c
	i686-elf-gcc ${INCLUDES} ${FLAGS} -std=gnu99 -c ./simplecheck.c -o ./build/simplecheck.o

clean:
	rm -f ${FILES} ./simplecheck.elf ./build
