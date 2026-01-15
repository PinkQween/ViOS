# ViOS – A Custom x86 Multithreaded Kernel

> 🧠 A handcrafted 64-bit x86 Operating System  
> 🕯️ Built in memory of **Vio** from SiegedSec

![ViOS Logo](/images/logo.png)

---

## 📊 Repository Stats

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: x86_64](https://img.shields.io/badge/Platform-x86_64-blue)](#)
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
    
*   🧠 Long Mode kernel (C)
    
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
├── bin
│   ├── kernel.bin
│   ├── kernel.elf
│   └── ViOS.efi
├── build.sh
├── clean.sh
├── images
│   └── logo.png
├── mnt
├── README.md
├── run.sh
├── ViOS.c
├── ViOS.inf
├── ViOS.uni
├── ViOS64Bit
│   ├── assets
│   │   ├── blank
│   │   │   ├── blank_test.elf
│   │   │   ├── blank.c
│   │   │   ├── blank.elf
│   │   │   ├── build
│   │   │   ├── linker.ld
│   │   │   └── Makefile
│   │   ├── shell
│   │   │   ├── build
│   │   │   ├── linker.ld
│   │   │   ├── Makefile
│   │   │   └── src
│   │   │       ├── shell.c
│   │   │       └── shell.h
│   │   ├── simple
│   │   │   ├── build
│   │   │   │   └── simple.bin
│   │   │   ├── Makefile
│   │   │   └── src
│   │   │       └── simple.asm
│   │   └── stdlib
│   │       ├── build
│   │       ├── Makefile
│   │       └── src
│   │           ├── memory.c
│   │           ├── memory.h
│   │           ├── start.asm
│   │           ├── start.c
│   │           ├── stdio.c
│   │           ├── stdio.h
│   │           ├── stdlib.c
│   │           ├── stdlib.h
│   │           ├── string.c
│   │           ├── string.h
│   │           ├── vios.asm
│   │           ├── vios.c
│   │           └── vios.h
│   ├── build.sh
│   ├── compile_commands.json
│   ├── data
│   │   └── images
│   │       ├── bkground.bmp
│   │       ├── clsicon.bmp
│   │       └── fonts
│   │           └── sysfont.bmp
│   ├── Makefile
│   ├── run.sh
│   ├── src
│   │   ├── boot
│   │   │   └── boot.asm
│   │   ├── config.h
│   │   ├── disk
│   │   │   ├── disk.c
│   │   │   ├── disk.h
│   │   │   ├── gpt.c
│   │   │   ├── gpt.h
│   │   │   ├── streamer.c
│   │   │   └── streamer.h
│   │   ├── fs
│   │   │   ├── fat
│   │   │   │   ├── fat16.c
│   │   │   │   └── fat16.h
│   │   │   ├── file.c
│   │   │   ├── file.h
│   │   │   ├── pparser.c
│   │   │   └── pparser.h
│   │   ├── gdt
│   │   │   ├── gdt.asm
│   │   │   ├── gdt.c
│   │   │   └── gdt.h
│   │   ├── graphics
│   │   │   ├── font.c
│   │   │   ├── font.h
│   │   │   ├── graphics.c
│   │   │   ├── graphics.h
│   │   │   ├── image
│   │   │   │   ├── bmp.c
│   │   │   │   ├── bmp.h
│   │   │   │   ├── image.c
│   │   │   │   └── image.h
│   │   │   ├── terminal.c
│   │   │   └── terminal.h
│   │   ├── idt
│   │   │   ├── idt.asm
│   │   │   ├── idt.c
│   │   │   ├── idt.h
│   │   │   ├── irq.c
│   │   │   └── irq.h
│   │   ├── io
│   │   │   ├── io.asm
│   │   │   └── io.h
│   │   ├── isr80h
│   │   │   ├── heap.c
│   │   │   ├── heap.h
│   │   │   ├── io.c
│   │   │   ├── io.h
│   │   │   ├── isr80h.c
│   │   │   ├── isr80h.h
│   │   │   ├── misc.c
│   │   │   ├── misc.h
│   │   │   ├── process.c
│   │   │   └── process.h
│   │   ├── kernel.asm
│   │   ├── kernel.c
│   │   ├── kernel.h
│   │   ├── keyboard
│   │   │   ├── classic.c
│   │   │   ├── classic.h
│   │   │   ├── keyboard.c
│   │   │   └── keyboard.h
│   │   ├── lib
│   │   │   └── vector
│   │   │       ├── vector.c
│   │   │       └── vector.h
│   │   ├── linker.ld
│   │   ├── loader
│   │   │   └── formats
│   │   │       ├── elf.c
│   │   │       ├── elf.h
│   │   │       ├── elfloader.c
│   │   │       └── elfloader.h
│   │   ├── memory
│   │   │   ├── heap
│   │   │   │   ├── heap.c
│   │   │   │   ├── heap.h
│   │   │   │   ├── kheap.c
│   │   │   │   ├── kheap.h
│   │   │   │   ├── multiheap.c
│   │   │   │   └── multiheap.h
│   │   │   ├── memory.c
│   │   │   ├── memory.h
│   │   │   └── paging
│   │   │       ├── paging.asm
│   │   │       ├── paging.c
│   │   │       └── paging.h
│   │   ├── status.h
│   │   ├── string
│   │   │   ├── string.c
│   │   │   └── string.h
│   │   ├── task
│   │   │   ├── process.c
│   │   │   ├── process.h
│   │   │   ├── task.asm
│   │   │   ├── task.c
│   │   │   ├── task.h
│   │   │   ├── tss.asm
│   │   │   └── tss.h
│   │   └── types.h
│   └── utilities
│       └── updateBoot.sh
├── ViOSExtra.uni
└── ViOSStr.uni

42 directories, 127 files
```

___________
<a id="build-requirements"></a>
🚧 Build Requirements
---------------------

### Linux (Ubuntu/Debian)
```bash
sudo apt install cmake build-essential nasm python3
sudo apt install gcc-x86-64-linux-gnu binutils-x86-64-linux-gnu
```

### macOS (Homebrew)
```bash
brew install cmake nasm python3
brew install x86_64-elf-gcc x86_64-elf-binutils
```

___________
<a id="building-vios"></a>
🚀 Building ViOS
----------------

### Quick Build

```bash
make
```

### Manual CMake

```bash
mkdir build && cd build
cmake ..
make
```

Output: `build/bin/os.img`

### Options

```bash
# Custom cross-compiler
make CROSS_COMPILE=x86_64-linux-gnu-

# Clean build
make clean
```

### Running

```bash
./run.sh
```

See [BUILD_GUIDE.md](BUILD_GUIDE.md) for detailed instructions.

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
    qemu-system-x86_64 -kernel bin/os.bin
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
