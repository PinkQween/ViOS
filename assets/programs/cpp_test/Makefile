CPP_TARGET = cpp_test.elf
CPP_SRC = src/cpp_test.cpp
CPP_OBJ = cpp_test.o
CPP = i686-elf-g++
CPPFLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -I/opt/ViOS/include
LIBS = /opt/ViOS/lib/libViOSlibc.a

all: $(CPP_TARGET)

$(CPP_TARGET): $(CPP_OBJ)
	i686-elf-ld -m elf_i386 -relocatable --entry _start $(CPP_OBJ) $(LIBS) -o $(CPP_TARGET)

$(CPP_OBJ): $(CPP_SRC)
	$(CPP) $(CPPFLAGS) -std=gnu++11 -c $< -o $@

clean:
	rm -f $(CPP_OBJ) $(CPP_TARGET)