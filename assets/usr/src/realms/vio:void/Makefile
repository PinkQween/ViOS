CXX = i386-vios-elf-g++
CC  = i386-vios-elf-gcc

SRC = src/main.cpp
OBJ = build/main.o
OUTTMP = ../../../../sbin/vio-void
OUT = ../../../../sbin/vio\:void

CFLAGS = -O2
LDFLAGS  = 

all: $(OUTTMP)

$(OBJ): $(SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTTMP): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@
	mv $(OUTTMP) $(OUT) 

clean:
	rm -rf build $(OUT)
