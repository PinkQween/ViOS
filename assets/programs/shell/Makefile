# Check for ViOS libc location - prefer system-wide, fallback to local
VIOS_LIBC_PATH := $(shell if [ -f "/opt/ViOS/lib/libViOSlibc.a" ]; then echo "/opt/ViOS"; else echo "../../external/ViOS-libc"; fi)
VIOS_INCLUDE_PATH := $(VIOS_LIBC_PATH)/include
VIOS_LIB_PATH := $(VIOS_LIBC_PATH)/lib/libViOSlibc.a

FILES = ./build/shell.o
INCLUDES = -I$(VIOS_INCLUDE_PATH)
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

.PHONY: prepare_dirs
prepare_dirs:
	mkdir -p ./build

all: prepare_dirs $(FILES)
	i686-elf-gcc -g -T ./linker.ld -o ./shell.elf -ffreestanding -O0 -nostdlib -fpic -g $(FILES) $(VIOS_LIB_PATH)

./build/shell.o: ./src/shell.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/shell.c -o ./build/shell.o

clean:
	rm -rf $(FILES) ./build
	rm -f ./shell.elf