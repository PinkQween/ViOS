FILES=./build/blank.o
MALICIOUS_FILES=./build/malicioustest.o
SIMPLE_FILES=./build/simpletest.o
INCLUDES= -I../stdlib/src
FLAGS= -g -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./build

all: prepare_dirs ${FILES} ${MALICIOUS_FILES} ${SIMPLE_FILES}
	mkdir -p build
	i686-elf-gcc -g -T ./linker.ld -o ./blank.elf -O0 -nostdlib -fpic -g ${FILES} ../stdlib/stdlib.elf
	i686-elf-gcc -g -T ./linker.ld -o ./malicioustest.elf -O0 -nostdlib -fpic -g ${MALICIOUS_FILES} ../stdlib/stdlib.elf
	i686-elf-gcc -g -T ./linker.ld -o ./simpletest.elf -O0 -nostdlib -fpic -g ${SIMPLE_FILES} ../stdlib/stdlib.elf

./build/blank.o: ./blank.c
	i686-elf-gcc ${INCLUDES} ${FLAGS} -std=gnu99 -c ./blank.c -o ./build/blank.o

./build/malicioustest.o: ./malicioustest.c
	i686-elf-gcc ${INCLUDES} ${FLAGS} -std=gnu99 -c ./malicioustest.c -o ./build/malicioustest.o

./build/simpletest.o: ./simpletest.c
	i686-elf-gcc ${INCLUDES} ${FLAGS} -std=gnu99 -c ./simpletest.c -o ./build/simpletest.o

clean:
	rm -f ${FILES} ${MALICIOUS_FILES} ${SIMPLE_FILES} ./blank.elf ./malicioustest.elf ./simpletest.elf