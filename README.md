# ViOS – A Custom x86 Multithreaded Kernel

> 🧠 A handcrafted 32-bit x86 Operating System  
> 🕯️ Built in memory of **Vio** from SiegedSec

![ViOS Logo](/logo.png)

---

## 📊 Repository Stats

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: x86_32](https://img.shields.io/badge/Platform-x86_32-blue)](#)
![GitHub Repo Size](https://img.shields.io/github/repo-size/PinkQween/ViOS)
![Files in Repo](https://img.shields.io/github/directory-file-count/PinkQween/ViOS?type=file)
![Last Commit](https://img.shields.io/github/last-commit/PinkQween/ViOS)
![Release](https://img.shields.io/github/v/release/PinkQween/ViOS)
![Downloads](https://img.shields.io/github/downloads/PinkQween/ViOS/total)

---

### ViOS Toolchain Versions

The following components are versioned together and must be used in compatible versions:

![GitHub Release](https://img.shields.io/github/v/release/pinkqween/vios-libc?label=Libc%20Version)
![GitHub Release](https://img.shields.io/github/v/release/pinkqween/vios-binutils?label=Binutils%20Version)
![GitHub Release](https://img.shields.io/github/v/release/pinkqween/vios?label=ViOS%20Version)

---

## 🔧 Build Status

[![Build: Libc](https://github.com/PinkQween/ViOS-Libc/actions/workflows/build.yml/badge.svg)](https://github.com/PinkQween/ViOS-Libc/actions/workflows/build.yml)
[![Build: Binutils](https://github.com/PinkQween/ViOS-binutils/actions/workflows/build.yml/badge.svg)](https://github.com/PinkQween/ViOS-binutils/actions/workflows/build.yml)
[![Build: OS (ViOS)](https://github.com/PinkQween/ViOS/actions/workflows/build.yml/badge.svg)](https://github.com/PinkQween/ViOS/actions/workflows/build.yml)

---

## 🧠 Community and Contribution

![GitHub Discussions](https://img.shields.io/github/discussions/PinkQween/ViOS)
![Forks](https://img.shields.io/github/forks/PinkQween/ViOS?style=social)
![Commit Activity](https://img.shields.io/github/commit-activity/w/PinkQween/ViOS)
![Total Issues](https://img.shields.io/github/issues/PinkQween/ViOS?label=total%20issues&color=blue)
![Total PRs](https://img.shields.io/github/issues-pr/PinkQween/ViOS?label=total%20PRs&color=blue)

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
├── .env
├── .github
│   ├── dependabot.yml
│   ├── FUNDING.yml
│   └── workflows
│       └── build.yml
├── .gitignore
├── assets
│   ├── etc
│   │   ├── passwd
│   │   └── shadow
│   ├── shared
│   │   ├── fonts
│   │   │   └── .keep
│   │   └── lib
│   │       └── .keep
│   ├── sys
│   │   └── src
│   │       └── reloivd
│   │           ├── Makefile
│   │           └── src
│   │               └── main.cpp
│   ├── tmp
│   │   └── .keep
│   ├── usr
│   │   └── src
│   │       ├── programs
│   │       │   ├── echo
│   │       │   │   ├── Makefile
│   │       │   │   └── src
│   │       │   │       └── main.cpp
│   │       │   ├── ls
│   │       │   │   ├── Makefile
│   │       │   │   └── src
│   │       │   │       └── main.cpp
│   │       │   ├── öviash
│   │       │   │   ├── Makefile
│   │       │   │   └── src
│   │       │   │       └── main.cpp
│   │       │   ├── pwd
│   │       │   │   ├── Makefile
│   │       │   │   └── src
│   │       │   │       └── main.cpp
│   │       │   └── v
│   │       │       ├── Makefile
│   │       │       └── src
│   │       │           └── main.cpp
│   │       └── realms
│   │           ├── vio:nova
│   │           │   ├── Makefile
│   │           │   └── src
│   │           │       └── main.cpp
│   │           └── vio:void
│   │               ├── Makefile
│   │               └── src
│   │                   └── main.cpp
│   └── var
│       └── log
│           └── .keep
├── build.sh
├── buildExternal.sh
├── docs
│   └── api
│       ├── copy_string_from_task.md
│       ├── disable_interrupts.md
│       ├── enable_interrupts.md
│       ├── heap_create.md
│       ├── heap_free.md
│       ├── heap_malloc.md
│       ├── idt_init.md
│       ├── isr80h_register_command.md
│       ├── kfree.md
│       ├── kheap_init.md
│       ├── kmalloc.md
│       ├── kzalloc.md
│       ├── paging_new_4gb.md
│       ├── paging_switch.md
│       ├── process_load.md
│       ├── process_switch.md
│       ├── README.md
│       ├── sys_exit.md
│       ├── sys_free.md
│       ├── sys_getkey.md
│       ├── sys_malloc.md
│       ├── sys_print.md
│       ├── sys_sleep.md
│       ├── task_current.md
│       ├── task_get_stack_item.md
│       ├── task_new.md
│       ├── task_switch.md
│       ├── vix_clear_screen.md
│       ├── vix_draw_pixel.md
│       └── vix_present_frame.md
├── LICENSE
├── Makefile
├── README.md
├── run.sh
├── setupRemoteBuild.sh
├── src
│   ├── audio
│   │   ├── audio.c
│   │   ├── audio.h
│   │   ├── sb16.c
│   │   └── sb16.h
│   ├── boot
│   │   └── boot.asm
│   ├── config.h
│   ├── debug
│   │   ├── simple_serial.c
│   │   └── simple_serial.h
│   ├── disk
│   │   ├── disk.c
│   │   ├── disk.h
│   │   ├── streamer.c
│   │   └── streamer.h
│   ├── fs
│   │   ├── fat
│   │   │   ├── fat16.c
│   │   │   ├── fat16.h
│   │   │   ├── fat32.c
│   │   │   └── fat32.h
│   │   ├── file.c
│   │   ├── file.h
│   │   ├── pparser.c
│   │   └── pparser.h
│   ├── gdt
│   │   ├── gdt.asm
│   │   ├── gdt.c
│   │   └── gdt.h
│   ├── idt
│   │   ├── idt.asm
│   │   ├── idt.c
│   │   └── idt.h
│   ├── io
│   │   ├── io.asm
│   │   └── io.h
│   ├── isr80h
│   │   ├── file.c
│   │   ├── file.h
│   │   ├── heap.c
│   │   ├── heap.h
│   │   ├── isr80h.c
│   │   ├── isr80h.h
│   │   ├── keyboard.c
│   │   ├── keyboard.h
│   │   ├── process.c
│   │   ├── process.h
│   │   ├── serial.c
│   │   ├── serial.h
│   │   ├── waits.c
│   │   └── waits.h
│   ├── kernel
│   │   ├── init.c
│   │   ├── init.h
│   │   ├── mainloop.c
│   │   └── mainloop.h
│   ├── kernel.asm
│   ├── kernel.c
│   ├── kernel.h
│   ├── keyboard
│   │   ├── keyboard.c
│   │   ├── keyboard.h
│   │   ├── ps2_keyboard.c
│   │   └── ps2_keyboard.h
│   ├── linker.ld
│   ├── loader
│   │   └── formats
│   │       ├── elf.c
│   │       ├── elf.h
│   │       ├── elfloader.c
│   │       └── elfloader.h
│   ├── math
│   │   ├── fpu_math.c
│   │   └── fpu_math.h
│   ├── memory
│   │   ├── heap
│   │   │   ├── heap.c
│   │   │   ├── heap.h
│   │   │   ├── kheap.c
│   │   │   └── kheap.h
│   │   ├── memory.c
│   │   ├── memory.h
│   │   └── paging
│   │       ├── paging.asm
│   │       ├── paging.c
│   │       └── paging.h
│   ├── mouse
│   │   ├── mouse.c
│   │   ├── mouse.h
│   │   ├── ps2_mouse.c
│   │   └── ps2_mouse.h
│   ├── panic
│   │   ├── panic.c
│   │   └── panic.h
│   ├── pci
│   │   ├── pci.c
│   │   ├── pci.h
│   │   ├── virtio_gpu_pci.c
│   │   ├── virtio_gpu_pci.h
│   │   ├── virtio_pci.c
│   │   └── virtio_pci.h
│   ├── power
│   │   ├── power.c
│   │   └── power.h
│   ├── rtc
│   │   ├── rtc.c
│   │   └── rtc.h
│   ├── status.h
│   ├── string
│   │   ├── string.c
│   │   └── string.h
│   ├── task
│   │   ├── process.c
│   │   ├── process.h
│   │   ├── task.asm
│   │   ├── task.c
│   │   ├── task.h
│   │   ├── tss.asm
│   │   └── tss.h
│   ├── utils
│   │   ├── utils.c
│   │   └── utils.h
│   └── vigfx
│       ├── vigfx.c
│       ├── vigfx.h
│       ├── virtio_gpu.c
│       └── virtio_gpu.h
├── utilities
│   ├── fonts
│   │   ├── Arial.ttf
│   │   ├── AtariST8x16SystemFont.ttf
│   │   ├── Brightly.otf
│   │   ├── Cheri.ttf
│   │   └── RobotoThin.ttf
│   ├── generateFonts.py
│   ├── generateFonts.sh
│   ├── updateBoot.sh
│   └── updateREADME.sh
└── ViOS_LOGO_PNG.png

65 directories, 176 files
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

> _"Not all hackers wear masks. Some wear purpose."_  
> — **Vio** (SiegedSec)
