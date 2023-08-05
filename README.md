cat > README.md << EOF
# XM-23 Machine Emulator Documentation

This README provides an overview of the modules and functionalities of the XM-23 machine emulator.

## Key Features:

- Initialization of the CPU clock.
- File loading capabilities for S-Record formatted files.
- Cache initialization.
- Execution of the main controller function to manage the emulation process.

**Note**: To run the program on Visual Studio, it's recommended to set the \`_CRT_SECURE_NO_WARNINGS\` preprocessor definition to avoid certain warnings.

## Main Emulator Program - \`main.c\`

The \`main.c\` file serves as the entry point for the X-Makina Emulator. The program is designed to emulate the central processing unit (CPU) of the XM-23 machine, offering fetch, decode, and execute functionalities. It can read S-Record formatted files, extracting and decoding the embedded information.

## Debugging Tools - \`debug.c\`

This module provides debugging tools for interacting with the emulator. The main features include:

- An interactive command-line interface for debugging.
- Capabilities to view the contents of the registers and memory.
- Adjusting the program counter (PC).
- Controlling the execution flow by adding breakpoints.
- Modifying the Program Status Word (PSW).
- Running new .xme files and more.

## S-Record File Loader - \`Loader.c\`

This module provides functionality to:

- Read S-Record formatted files.
- Extract and decode necessary information such as record type, data length, address, and data.
- Populate memory and set up the initial program counter (PC) for the emulator.

Only S0, S1, and S9 records are supported.

## Central Processing Unit Emulator - \`CPU.c\`

This module emulates the central processing unit (CPU) of the XM-23 machine. The emulator is capable of:

- Instruction fetch, decode, and execute.
- Handling memory operations.
- Managing branching instructions.
- Supporting multiple addressing modes, as detailed in [CPU_addressing.c].
- Performing arithmetic operations, as detailed in [CPU_Arithmetic].

## CPU Addressing Modes and Branching - \`CPU_addressing.c\`

This module provides functionalities related to:

- Different addressing modes, including indexed and relative.
- Register move operations.
- Branching strategies, both conditional and linking.

These functionalities are crucial for controlling the flow of execution and managing memory within the context of the XM23 processor. The code was last updated on June 18, 2023.

## CPU Operations - \`CPU_operations.c\`

This module emulates various CPU operations for the XM-23 machine. It contains functions for:

- Copying and swapping register contents.
- Sign extending.
- Shifting and rotating right with carry.
- Swapping bytes of a register.
- Executing arithmetic operations.

## Program Status Word (PSW) Handling - \`psw.c\`

This module is responsible for managing the Program Status Word (PSW) of the emulator. The PSW is a special-purpose register that stores the status flags, which reflect the outcome of machine language instructions executed by the CPU. The module provides functions to update the PSW based on arithmetic and logic operations. It also offers functionalities to set and clear specific flags in the PSW.

## Cache Memory System - \`Cache.c\`

This module provides a comprehensive emulation of cache memory operations. The main features include:

- Emulation of both write-back and write-through caching strategies.
- Functions for cache initialization, address location, cache updating, cache printing, and cache line aging.
- Support for associative as well as direct mapping cache strategies.

The caching strategy can be chosen by uncommenting the respective macro definitions in the file. For instance, to choose the write-back strategy, uncomment \`#define WRT_BACK\` and comment out \`#define WRT_THRO\`.

## Priority Execution - \`Priority.c\`

This module is responsible for handling conditional execution based on various conditions like equality, carry set, minus, overflow, etc. It evaluates the conditions and sets the \`TRU_FLS\` flag accordingly. The module also provides feedback on whether the condition evaluated to \`TRUE\` or \`FALSE\`.

## Emulator Definitions and Declarations - \`emulator.h\`

The \`emulator.h\` header file provides the necessary definitions and declarations for the X-Makina emulator program. It includes:

- Macro definitions for register and memory configurations, bit masking, and instruction specifics.
- Enumerations for various instruction types.
- Byte masking definitions.
- Program Status Word (PSW) structure and related definitions.
- DADD (Decimal Adjust after Addition) implementation structures.
- Function declarations for program flow control, memory management, instruction implementations, and user interface functionalities.

## Debugging

Both modules contain debugging flags (\`DEBUG\`, \`SwapDebug\`, \`ARITH_DEBUG\`, \`ADDC\`) that can be enabled to print detailed information about the operations being performed.
EOF
## Requirements:

- A C/C++ compiler (e.g., GCC, Clang, or MSVC for Visual Studio).
- Familiarity with S-Record formatted files if you wish to load custom programs.
- Basic understanding of CPU emulation and the XM-23 machine architecture.
- (Optional) Visual Studio for a more streamlined debugging experience. Remember to set the \`_CRT_SECURE_NO_WARNINGS\` preprocessor definition.
