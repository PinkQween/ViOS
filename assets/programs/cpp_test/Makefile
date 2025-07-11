FILES=./build/cpp_test.o
INCLUDES=-I./src -I../stdlib++/src
FLAGS= -g -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc -fno-rtti -fno-exceptions

.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./build

all: prepare_dirs ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ./build/cpp_test.o ../stdlib++/stdlib.elf -o ./cpp_test.elf

./build/cpp_test.o: ./src/cpp_test.cpp
	i686-elf-g++ ${INCLUDES} $(FLAGS) -c ./src/cpp_test.cpp -o ./build/cpp_test.o

clean:
	rm -rf ${FILES} ./build ./cpp_test.elf 