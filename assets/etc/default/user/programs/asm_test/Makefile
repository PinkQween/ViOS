all:
	nasm -f elf ./main.asm -o ./asm_test.o
	i386-vios-elf-ld -T ./linker.ld -o ./asm_test.elf ./asm_test.o

clean:
	rm -f ./asm_test.o ./asm_test.elf