FILES=./build/wait.o
INCLUDES= -I../stdlib/src
FLAGS= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ${FILES}
	i686-elf-gcc -g -T ./linker.ld -o ./wait.elf -ffreestanding -O0 -nostdlib -fpic -g ${FILES} ../stdlib/stdlib.elf

./build/wait.o: ./wait.c
	i686-elf-gcc ${INCLUDES}  -I./$(FLAGS) -std=gnu99 -c ./wait.c -o ./build/wait.o

clean:
	rm -rf ${FILES}
	rm ./wait.elf