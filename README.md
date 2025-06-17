# ViOS – A Custom x86 Multithreaded Kernel

> 🧠 A handcrafted 32-bit x86 Operating System  
> 🕯️ Built in memory of **Vio** from SiegedSec  
> 💻 Based on concepts from the "Developing a Multithreaded Kernel from Scratch" course

---

🕯️ In Memory of Vio
--------------------

**Vio** was a voice for transparency, a low-level coder, and a hacker who believed in teaching others how systems truly work. This OS is a tribute to that spirit. It is open, raw, and built to teach by showing—not just telling.

---

⚙️ About ViOS
-------------

**ViOS** is a learning-focused multithreaded operating system for the x86 (32-bit) architecture. It builds on core concepts taught in the *Developing a Multithreaded Kernel from Scratch* course, with custom enhancements, structure, and purpose.

It features a full bootloader-to-kernel stack written in Assembly and C, and aims to be both an educational platform and a statement about digital autonomy.

---

🧩 Features
-----------

* 🧬 Real Mode Bootloader (Assembly)
* 🧠 Protected Mode kernel (C)
* 🧷 Paging, heap, memory management
* 📁 FAT16 Filesystem parser
* 🧵 Process and task switching (multitasking support)
* 🧩 ELF executable loader
* 🔐 Virtual memory
* 📟 IO port & IRQ support
* 🌀 Disk reading/streamer layer
* ⌨️ Keyboard driver
* 💬 Minimal shell (WIP)
* 🛠️ Designed for use with GDB and QEMU

---

📚 Based On
-----------

This project started by following the excellent curriculum from the [Developing a Multithreaded Kernel from Scratch](https://dragonzap.com/course/developing-a-multithreaded-kernel-from-scratch?coupon=GITHUBKERNELDISCOUNT) course. It extends the project with a personalized shell, restructured file layout, and a political voice.

All credit to the original author for providing such a powerful educational base.

---

🗂️ Project Structure
--------------------

```
.
├── LICENSE
├── Makefile
├── PNGToBin.py
├── README.md
├── ViOS_LOGO_PNG.png
├── assets
│   ├── file.txt
│   ├── logo.bin
│   └── logo.pal
├── bin
├── build
│   ├── disk
│   ├── fs
│   │   └── fat
│   ├── gdt
│   ├── idt
│   ├── io
│   ├── memory
│   │   ├── heap
│   │   └── paging
│   ├── string
│   └── task
├── build.sh
└── src
    ├── boot
    │   └── boot.asm
    ├── config.h
    ├── disk
    │   ├── disk.c
    │   ├── disk.h
    │   ├── streamer.c
    │   └── streamer.h
    ├── fs
    │   ├── fat
    │   │   ├── fat16.c
    │   │   └── fat16.h
    │   ├── file.c
    │   ├── file.h
    │   ├── pparser.c
    │   └── pparser.h
    ├── gdt
    │   ├── gdt.asm
    │   ├── gdt.c
    │   └── gdt.h
    ├── idt
    │   ├── idt.asm
    │   ├── idt.c
    │   └── idt.h
    ├── io
    │   ├── io.asm
    │   └── io.h
    ├── kernel.asm
    ├── kernel.c
    ├── kernel.h
    ├── linker.ld
    ├── memory
    │   ├── heap
    │   │   ├── heap.c
    │   │   ├── heap.h
    │   │   ├── kheap.c
    │   │   └── kheap.h
    │   ├── memory.c
    │   ├── memory.h
    │   └── paging
    │       ├── paging.asm
    │       ├── paging.c
    │       └── paging.h
    ├── status.h
    ├── string
    │   ├── string.c
    │   └── string.h
    └── task
        ├── process.c
        ├── process.h
        ├── task.c
        ├── task.h
        ├── tss.asm
        └── tss.h

28 directories, 51 files
```

---

🚧 Build Requirements
---------------------

Install the following:

- `nasm` – Assembler
- `i686-elf-gcc` – Cross-compiler
- `qemu` – Emulator (optional)
- `grub-mkrescue` – ISO generation (optional)

### macOS (Homebrew)

```bash
brew install nasm qemu x86_64-elf-gcc
```

___________

🚀 Building ViOS
----------------

To build the OS:

```bash
./build.sh
```

This will:

1. Assemble the bootloader and kernel

2. Compile all components

3. Link the final binary to ./bin/os.bin
    

To emulate with QEMU:

```bash
qemu-system-i386 -kernel bin/os.bin
```

___________

🌈 Why ViOS?
------------

ViOS is not just a clone of the course—it’s an extension. It keeps the structured educational benefit while evolving with unique features and philosophical goals. It’s for learners, rebels, and those who want to break the system down—one opcode at a time.

___________

🪪 License
----------

MIT License – use it, fork it, build from it. Just don’t forget where you came from.

___________

✍️ Author
----------

This project is licensed under the **MIT License**. Use it, break it, share it.  
Just keep the spirit alive.

Built and customized by Hanna Skairipa
🔗 GitHub – PinkQween/ViOS

> _"Not all hackers wear masks. Some wear purpose."_  
> — Vio (SiegedSec)


---

Let me know if you'd like a version with additional markdown badges (build status, license, etc.) or CI integration (like GitHub Actions for build testing).
