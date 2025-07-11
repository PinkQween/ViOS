INCLUDES = -I../stdlib/src
FLAGS = -g -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc -std=gnu99
OBJECTS = ./build/audiotest.o

all: ./audiotest.elf

./audiotest.elf: $(OBJECTS)
	mkdir -p build
	i686-elf-gcc -g -T ./linker.ld -o ./audiotest.elf -O0 -nostdlib -fpic -g $(OBJECTS) ../stdlib/stdlib.elf

./build/audiotest.o: ./audiotest.c
	mkdir -p ./build
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c ./audiotest.c -o ./build/audiotest.o

clean:
	rm -rf ./build/audiotest.o ./audiotest.elf ./build
