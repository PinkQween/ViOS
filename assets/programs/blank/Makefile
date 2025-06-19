FILES=./build/blank.o
MALICIOUS_FILES=./build/malicious_test.o
SIMPLE_FILES=./build/simple_test.o
INCLUDES= -I../stdlib/src
FLAGS= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ${FILES} ${MALICIOUS_FILES} ${SIMPLE_FILES}
	i686-elf-gcc -g -T ./linker.ld -o ./blank.elf -ffreestanding -O0 -nostdlib -fpic -g ${FILES} ../stdlib/stdlib.elf
	i686-elf-gcc -g -T ./linker.ld -o ./malicious_test.elf -ffreestanding -O0 -nostdlib -fpic -g ${MALICIOUS_FILES} ../stdlib/stdlib.elf
	i686-elf-gcc -g -T ./linker.ld -o ./simple_test.elf -ffreestanding -O0 -nostdlib -fpic -g ${SIMPLE_FILES} ../stdlib/stdlib.elf

./build/blank.o: ./blank.c
	i686-elf-gcc ${INCLUDES}  -I./$(FLAGS) -std=gnu99 -c ./blank.c -o ./build/blank.o

./build/malicious_test.o: ./malicious_test.c
	i686-elf-gcc ${INCLUDES}  -I./$(FLAGS) -std=gnu99 -c ./malicious_test.c -o ./build/malicious_test.o

./build/simple_test.o: ./simple_test.c
	i686-elf-gcc ${INCLUDES}  -I./$(FLAGS) -std=gnu99 -c ./simple_test.c -o ./build/simple_test.o

clean:
	rm -rf ${FILES} ${MALICIOUS_FILES} ${SIMPLE_FILES}
	rm ./blank.elf ./malicious_test.elf ./simple_test.elf