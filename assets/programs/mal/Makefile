MALICIOUS_FILES=./build/mal.o
INCLUDES= -I../stdlib/src
FLAGS= -g -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./build

all: prepare_dirs ${FILES} ${MALICIOUS_FILES} ${SIMPLE_FILES}
	mkdir -p build
	i686-elf-gcc -g -T ./linker.ld -o ./mal.elf -O0 -nostdlib -fpic -g ${MALICIOUS_FILES} ../stdlib/stdlib.elf
	
./build/mal.o: ./mal.c
	i686-elf-gcc ${INCLUDES} ${FLAGS} -std=gnu99 -c ./mal.c -o ./build/mal.o

clean:
	rm -f ${FILES} ${MALICIOUS_FILES} ${SIMPLE_FILES} ./blank.elf ./mal.elf ./simpletest.elf ./build