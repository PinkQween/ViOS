ViOS
====

> 🧠 A handcrafted 32-bit x86 Operating System  
> 🕯️ Built in memory of **Vio** from SiegedSec  
> 💻 Written in pure C and Assembly, from bootloader to kernel.

___________

🕯️ In Memory of Vio
--------------------

**Vio** was a deeply respected voice in the hacktivist collective **SiegedSec**. A fiercely independent hacker and coder, they spent their time dissecting systems, sharing knowledge, and fighting for transparency and resistance in cyberspace. Vio was known for their low-level expertise and direct contributions to digital protest. They were the kind of person who tore apart binaries not for attention—but for truth.

This OS is a tribute. It’s not commercial, not corporate, and not for everyone.  
It’s for the ones who want to learn what makes a machine tick—bit by bit, opcode by opcode.

___________

⚙️ About ViOS
-------------

**ViOS** is a fully custom operating system designed for the x86 (32-bit) architecture. It features a complete bootloader and kernel stack written from scratch, offering a deep dive into low-level systems programming.

Whether you're a student, hacker, or enthusiast—this project is for exploring how an OS truly works underneath all the abstractions.

___________

🧩 Features
-----------

*   🧬 Custom bootloader (`boot.asm`)
    
*   🧠 Protected Mode kernel written in C
    
*   🛠️ Paging, heap, memory manager
    
*   🧷 Full IDT + IRQ support
    
*   📁 Basic file system parser
    
*   🌀 Disk reading / streaming interface
    
*   📟 IO ports & direct hardware communication
    
*   💬 Minimal shell (WIP)
    

___________

🗂️ Directory Layout
--------------------

```
ViOS/
├── bin/              # Compiled binaries (bootloader, kernel, OS)
├── build/            # Intermediate build files (.o)
│   ├── disk/         # Disk reading & streaming
│   ├── fs/           # File system parser
│   ├── idt/          # Interrupt Descriptor Table
│   ├── io/           # IO port assembly
│   ├── memory/       # Memory management (heap, paging)
│   └── string/       # Custom string implementation
├── src/              # All source files (C/ASM)
│   ├── boot/         # Bootloader
│   ├── idt/, io/, memory/, fs/, disk/, string/
│   ├── kernel.c/.h   # Main kernel
│   └── linker.ld     # Linker script
├── build.sh          # Easy build script
├── Makefile          # Optional manual build
└── README.md         # You're reading it!
```

___________

🛠️ Requirements
----------------

Before building ViOS, install:

*   [`nasm`](https://www.nasm.us/) – assembler
    
*   `i686-elf-gcc` – cross compiler for kernel
    
*   [`qemu`](https://www.qemu.org/) – to emulate the OS (optional)
    
*   `grub-mkrescue` – for bootable ISO (optional)
    

### macOS (via Homebrew)

```bash
brew install nasm qemu x86_64-elf-gcc
```

___________

🚀 Building ViOS
----------------

To compile the OS, just run:

```bash
./build.sh
```

This will:

1.  Assemble the bootloader and system code
    
2.  Compile all kernel components
    
3.  Link everything together
    
4.  Output the final executable to `./bin/os.bin`
    

You can then run it in QEMU or write it to an ISO or disk image of your choice.

___________

🌈 Why ViOS?
------------

ViOS isn’t just another OS project. It's an **educational base** and a **political artifact**. It teaches how real systems work and celebrates those who’ve dedicated their lives to digital liberation.

___________

📬 Contact
----------

Built by Hanna Skairipa  
🌐 GitHub: [PinkQween/ViOS](https://github.com/PinkQween/ViOS.git)

___________

🪪 License
----------

This project is licensed under the **MIT License**. Use it, break it, share it.  
Just keep the spirit alive.

> _"Not all hackers wear masks. Some wear purpose."_  
> — Vio (SiegedSec)