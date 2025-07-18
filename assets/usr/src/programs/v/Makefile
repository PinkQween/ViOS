CXX = i386-vios-elf-g++
CC  = i386-vios-elf-gcc

SRC = src/main.cpp
OBJ = build/main.o
OUT = ../../../../bin/v

CFLAGS = -O2
LDFLAGS  = 

all: $(OUT)

$(OBJ): $(SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf build $(OUT)
