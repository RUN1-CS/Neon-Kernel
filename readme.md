# NK - Neon Kernel

# SSn-DOS - Shinseiki no Disk Operating System

A minimal operating system written in C and Assembly, featuring a bootloader, kernel with device drivers, and a custom shell application.

## Project Structure

```
├── apps/                          # Application layer
│   └── xxsh/                      # Custom shell implementation
│       ├── executor.c/.h          # Command execution engine
│       ├── shell.c/.h             # Shell interface
│       ├── readme.md              # Shell documentation
│       └── LICENSE
├── boot/                          # Bootloader
│   └── boot.asm                   # Assembly bootloader code
├── kernel/                        # Kernel core
│   ├── main.c                     # Kernel entry point
│   ├── linker.ld                  # Linker script
│   └── drivers/                   # Hardware drivers
│       ├── keyboard.c/.h          # Keyboard input driver
│       ├── video.c/.h             # Video/display driver
│       └── string.c/.h            # String utilities
├── build.sh                       # Build script
├── Makefile                       # Build configuration
├── main.c                         # Main entry point
└── LICENSE                        # License file
```

## Components

### Bootloader (`boot/`)

- Assembly-based bootloader responsible for initial system startup and kernel loading

### Kernel (`kernel/`)

- **Core**: Main kernel initialization and control flow
- **Drivers**:
  - **Keyboard**: Input handling from keyboard device
  - **Video**: Display and screen output management
  - **String**: String manipulation utilities

### Applications (`apps/`)

- **xxsh Shell**: Custom command-line shell with command execution capabilities
  - Executor: Processes and executes user commands
  - Shell: Interactive user interface

## Building

Run the build script to compile the entire project:

```bash
./build.sh
```

Or use Make:

```bash
make
```

## License

See LICENSE file in the project root and apps/xxsh/LICENSE for details.

## Features

- Custom bootloader for system initialization
- Kernel with essential device drivers
- Interactive shell application (xxsh)
- Hardware abstraction layer through drivers
