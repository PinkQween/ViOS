all:
	nasm -f elf ./main.asm -o ./asm_test.o
	i686-elf-gcc -g -T ./linker.ld -o ./asm_test.elf -ffreestanding -O0 -nostdlib -fpic -g ./asm_test.o

clean:
	rm -f ./asm_test.o ./asm_test.elf ./asm_test.bin