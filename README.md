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

🤝 Contributing
---------------

Contributions are **highly encouraged and deeply appreciated**. ViOS is more than an OS—it's a learning tool and a tribute to hacker culture. Whether you're fixing a bug, improving documentation, or building a whole new feature, your work helps keep the spirit of Vio and low-level computing alive.

### 🧭 How to Contribute

Want to get started? Here’s how:

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
    Add your code, fix bugs, write docs, or improve the build system. Keep commits focused.
    
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

### 🧼 Contribution Guidelines
    
*   Keep commits clean and descriptive.
    
*   If you’re adding new files, place them in a logical subdirectory.
    
*   Contributions can include:
    
    *   🔧 Bug fixes
        
    *   📄 Documentation
        
    *   ⚙️ Drivers or kernel features
        
    *   💬 Shell improvements
        
    *   📦 File system or memory improvements
 
    *   And More!

--- 

## 🧪 Ideas to Get Involved

- 🌐 Implement networking functionality (e.g. TCP/IP stack or USB Ethernet)
- 📦 Add support for system updates or patching mechanism
- 🧠 Add new syscalls or user-mode execution support
- 🛠️ Expand the shell with built-in commands (like `ls`, `cat`, `cd`)
- 🧳 Build a lightweight `init` system or process manager
- 🧾 Add support for EXT4 or exFAT filesystems
- 🎮 Build demo applications or a TUI-based game on top of ViOS

---

## 🧵 Just Starting?

No worries! Open an issue with a question, start a discussion, or contribute to the documentation to get your feet wet. Everyone starts somewhere—and every little bit helps.

> _"The OS belongs to everyone who dares to open the binary."_  
> – You, after your first PR

---

## 🪪 License

MIT License — use it, fork it, build on it.  
Just don’t forget where you came from.

---

## ✍️ Author

Built and maintained by **Hanna Skairipa**  
🔗 [PinkQween on GitHub](https://github.com/PinkQween)

> _"Not all hackers wear masks. Some wear purpose."_  
> — **Vio** (SiegedSec)
