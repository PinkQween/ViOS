# Prefer system-wide ViOS-libc, fallback to local
VIOS_LIBC_PATH := $(shell if [ -f "/opt/ViOS/lib/libViOSlibc.a" ]; then echo "/opt/ViOS"; else echo "../../external/ViOS-libc"; fi)
VIOS_INCLUDE_PATH := $(VIOS_LIBC_PATH)/include
VIOS_LIB_PATH := $(VIOS_LIBC_PATH)/lib/libViOSlibc.a

CPP_TARGET = cpp_prnt.elf
CPP_SRC = src/main.cpp
CPP_OBJ = cpp_prnt.o
CPP = i686-elf-g++
CPPFLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -I$(VIOS_INCLUDE_PATH)
LIBS = $(VIOS_LIB_PATH)

all: $(CPP_TARGET)

$(CPP_TARGET): $(CPP_OBJ)
	i686-elf-ld -m elf_i386 -T linker.ld $(CPP_OBJ) $(LIBS) -o $(CPP_TARGET)

$(CPP_OBJ): $(CPP_SRC)
	$(CPP) $(CPPFLAGS) -std=gnu++11 -c $< -o $@

clean:
	rm -f $(CPP_OBJ) $(CPP_TARGET)