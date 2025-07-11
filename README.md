ViOS – A Custom x86 Multithreaded Kernel
========================================

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Build](https://img.shields.io/badge/Build-Passing-brightgreen)
![Platform](https://img.shields.io/badge/Platform-x86_32-blue)

![logo](/ViOS_LOGO_PNG.png)

> 🧠 A handcrafted 32-bit x86 Operating System  
> 🕯️ Built in memory of **Vio** from SiegedSec

## 📚 Table of Contents

- [In Memory of Vio](#in-memory-of-vio)
- [About ViOS](#about-vios)
- [Features](#features)
- [Project Structure](#project-structure)
- [Build Requirements](#build-requirements)
- [Building ViOS](#building-vios)
- [Why ViOS?](#why-vios)
- [Contributing](#contributing)
- [Contribution Guidelines](#contribution-guidelines)
- [Ideas to Get Involved](#ideas-to-get-involved)
- [Just Starting?](#just-starting)
- [License](#license)
- [Author](#author)

___________
<a id="in-memory-of-vio"></a>
🕯️ In Memory of Vio
--------------------

**Vio** was a voice for transparency, a low-level coder, and a hacker who believed in teaching others how systems truly work. This OS is a tribute to that spirit. It is open, raw, and built to teach by showing—not just telling.

___________
<a id="about-vios"></a>
⚙️ About ViOS
-------------

**ViOS** is a learning-focused multithreaded operating system for the x86 (32-bit) architecture. It features a full bootloader-to-kernel stack written in Assembly and C, and aims to be both an educational platform and a statement about digital autonomy.

___________
<a id="features"></a>
🧩 Features
-----------

*   🧬 Real Mode Bootloader (Assembly)
    
*   🧠 Protected Mode kernel (C)
    
*   🧷 Paging, heap, memory management
    
*   📁 FAT16 Filesystem parser
    
*   🧵 Process and task switching (multitasking support)
    
*   🧩 ELF executable loader
    
*   🔐 Virtual memory
    
*   📟 IO port & IRQ support
    
*   🌀 Disk reading/streamer layer
    
*   ⌨️ Keyboard driver
    
*   💬 Minimal shell (WIP)
    
*   🛠️ Designed for use with GDB and QEMU
    

___________
<a id="project-structure"></a>
🗂️ Project Structure
---------------------

```
.
├── assets
│   ├── file.txt
│   ├── logo.bin
│   ├── logo.pal
│   └── programs
│       ├── blank
│       │   ├── blank.c
│       │   ├── build
│       │   ├── linker.ld
│       │   ├── Makefile
│       │   ├── malicioustest.c
│       │   └── simpletest.c
│       ├── mal
│       │   ├── build
│       │   ├── linker.ld
│       │   ├── Makefile
│       │   └── mal.c
│       ├── shell
│       │   ├── blank.elf
│       │   ├── linker.ld
│       │   ├── Makefile
│       │   └── src
│       │       ├── shell.c
│       │       └── shell.h
│       ├── stdlib
│       │   ├── linker.ld
│       │   ├── Makefile
│       │   ├── src
│       │   │   ├── memory.c
│       │   │   ├── memory.h
│       │   │   ├── start.asm
│       │   │   ├── start.c
│       │   │   ├── stdio.c
│       │   │   ├── stdio.h
│       │   │   ├── stdlib.c
│       │   │   ├── stdlib.h
│       │   │   ├── string.c
│       │   │   ├── string.h
│       │   │   ├── vios.asm
│       │   │   ├── vios.c
│       │   │   └── vios.h
│       │   └── stdlib.elf
│       ├── tests
│       │   ├── linker.ld
│       │   ├── Makefile
│       │   └── tests.c
│       └── wait
│           ├── linker.ld
│           ├── Makefile
│           └── wait.c
├── build.sh
├── LICENSE
├── Makefile
├── README.md
├── src
│   ├── boot
│   │   └── boot.asm
│   ├── config.h
│   ├── disk
│   │   ├── disk.c
│   │   ├── disk.h
│   │   ├── streamer.c
│   │   └── streamer.h
│   ├── fs
│   │   ├── fat
│   │   │   ├── fat16.c
│   │   │   └── fat16.h
│   │   ├── file.c
│   │   ├── file.h
│   │   ├── pparser.c
│   │   └── pparser.h
│   ├── gdt
│   │   ├── gdt.asm
│   │   ├── gdt.c
│   │   └── gdt.h
│   ├── idt
│   │   ├── idt.asm
│   │   ├── idt.c
│   │   └── idt.h
│   ├── io
│   │   ├── io.asm
│   │   └── io.h
│   ├── isr80h
│   │   ├── heap.c
│   │   ├── heap.h
│   │   ├── io.c
│   │   ├── io.h
│   │   ├── isr80h.c
│   │   ├── isr80h.h
│   │   ├── process.c
│   │   └── process.h
│   ├── kernel.asm
│   ├── kernel.c
│   ├── kernel.h
│   ├── keyboard
│   │   ├── classic.c
│   │   ├── classic.h
│   │   ├── keyboard.c
│   │   └── keyboard.h
│   ├── linker.ld
│   ├── loader
│   │   └── formats
│   │       ├── elf.c
│   │       ├── elf.h
│   │       ├── elfloader.c
│   │       └── elfloader.h
│   ├── memory
│   │   ├── heap
│   │   │   ├── heap.c
│   │   │   ├── heap.h
│   │   │   ├── kheap.c
│   │   │   └── kheap.h
│   │   ├── memory.c
│   │   ├── memory.h
│   │   └── paging
│   │       ├── paging.asm
│   │       ├── paging.c
│   │       └── paging.h
│   ├── panic
│   │   ├── panic.c
│   │   └── panic.h
│   ├── rtc
│   │   ├── rtc.c
│   │   └── rtc.h
│   ├── status.h
│   ├── string
│   │   ├── string.c
│   │   └── string.h
│   ├── task
│   │   ├── process.c
│   │   ├── process.h
│   │   ├── task.asm
│   │   ├── task.c
│   │   ├── task.h
│   │   ├── tss.asm
│   │   └── tss.h
│   ├── terminal
│   │   ├── terminal.c
│   │   └── terminal.h
│   └── utils
│       ├── utils.c
│       └── utils.h
└── ViOS_LOGO_PNG.png

34 directories, 110 files
```

___________
<a id="build-requirements"></a>
🚧 Build Requirements
---------------------

Install the following:

*   `nasm` – Assembler
    
*   `i686-elf-gcc` – Cross-compiler
    
*   `qemu` – Emulator (optional)
    
*   `grub-mkrescue` – ISO generation (optional)
    

### macOS (Homebrew)

```bash
brew install nasm qemu x86_64-elf-gcc
```

### Ubuntu/Debian

```bash
sudo apt install build-essential nasm qemu gcc-multilib grub-pc-bin xorriso
```

___________
<a id="building-vios"></a>
🚀 Building ViOS
----------------

To build the OS:

```bash
./build.sh
```

This will:

1. Assemble the bootloader and kernel  
2. Compile all components  
3. Link the final kernel binary to `./bin/os.bin`  
4. (If `grub-mkrescue` is installed) Generate a bootable ISO image as `./bin/os_disk.img`
    

To emulate with QEMU:

```bash
qemu-system-i386 -kernel bin/os.bin
```

___________
<a id="why-vios"></a>
🌈 Why ViOS?
------------

ViOS is a platform for those who want to go deep into systems programming. It’s handcrafted, educational, and designed to be extended. Whether you’re learning how memory works or building custom features, ViOS is for you.

___________
<a id="contributing"></a>
🤝 Contributing
---------------

Contributions are **highly encouraged and deeply appreciated**. ViOS is more than an OS—it's a learning tool and a tribute to hacker culture. Whether you're fixing a bug, improving documentation, or building a whole new feature, your work helps keep the spirit of Vio and low-level computing alive.

### 🧭 How to Contribute

1.  **Fork the Repo**  
    Click the **Fork** button on [GitHub](https://github.com/PinkQween/ViOS) to create your own copy of the project.
    
2.  **Clone Your Fork**
    
    ```bash
    git clone https://github.com/YOUR_USERNAME/ViOS.git
    cd ViOS
    ```
    
3.  **Create a New Branch**
    
    ```bash
    git checkout -b your-feature-name
    ```
    
4.  **Make Your Changes**  
    Add your code, fix bugs, write docs, or improve the build system.
    
5.  **Test Your Changes**  
    Run `./build.sh` and test the OS in QEMU:
    
    ```bash
    qemu-system-i386 -kernel bin/os.bin
    ```
    
6.  **Commit & Push**
    
    ```bash
    git add .
    git commit -m "Add: [short description of your change]"
    git push origin your-feature-name
    ```
    
7.  **Open a Pull Request**  
    Go to your fork on GitHub and click **New pull request**.
    

___________
<a id="contribution-guidelines"></a>
### 🧼 Contribution Guidelines

*   Keep commits clean and descriptive.
    
*   If you’re adding new files, place them in a logical subdirectory.
    
*   Contributions can include:
    
    *   🔧 Bug fixes
        
    *   📄 Documentation
        
    *   ⚙️ Drivers or kernel features
        
    *   💬 Shell improvements
        
    *   📦 File system or memory improvements
        

___________
<a id="ideas-to-get-involved"></a>
🧪 Ideas to Get Involved
------------------------

*   🌐 Implement networking functionality (e.g. TCP/IP stack or USB Ethernet)

*   🌐 Implement networking functionality (e.g. TCP/IP stack or USB Ethernet)
    
*   📦 Add support for system updates or patching mechanism
    
*   🧠 Add new syscalls or user-mode execution support
    
*   🛠️ Expand the shell with built-in commands (like `ls`, `cat`, `cd`)
    
*   🧳 Build a lightweight `init` system or process manager
    
*   🧾 Add support for EXT4 or exFAT filesystems
    
*   🎮 Build demo applications or a TUI-based game on top of ViOS

*   🧬 Add long mode (x86_64) support

*   🧱 Add support for other architectures
    

___________
<a id="just-starting"></a>
🧵 Just Starting?
-----------------

No worries! Open an issue with a question, start a discussion, or contribute to the documentation to get your feet wet. Everyone starts somewhere—and every little bit helps.

> _"The OS belongs to everyone who dares to open the binary."_  
> – You, after your first PR

___________
<a id="license"></a>
🪪 License
----------

MIT License — use it, fork it, build on it.  
Just don’t forget where you came from.

___________
<a id="author"></a>
✍️ Author
---------

Built and maintained by **Hanna Skairipa**  
🔗 [PinkQween on GitHub](https://github.com/PinkQween)
This fork is Mauntained by **Bradley Wu**

> _"Not all hackers wear masks. Some wear purpose."_  
> — **Vio** (SiegedSec)
