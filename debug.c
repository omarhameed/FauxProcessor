/**
 * @file debug.c
 * @brief Debugger and User Interface for ECED3403 Assignment 1 Emulator
 *
 * This module provides debugging tools for interacting with the emulator.
 * It allows the user to execute various debugging operations such as viewing
 * the contents of the registers and memory, adjusting the program counter (PC),
 * controlling the execution flow by adding breakpoints, modifying the Program
 * Status Word (PSW), running new .xme files and more.
 *
 * The interface operates in an interactive command-line mode, prompting the user
 * for commands and providing feedback based on their input.
 *
 * @author Omar Hameed
 * @student_id B00764655
 * @date Last updated on July 15, 2023
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "emulator.h"
#include "Cache.h"
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>
#define MAX_LINE_SIZE 16

enum { FALSE, TRUE };

volatile sig_atomic_t ctrl_c_fnd; /* T|F - indicates whether ^C detected */

void sigint_hdlr()
{
    /*
    - Invoked when SIGINT (control-C) is detected
    - changes state of waiting_for_signal
    - signal must be reinitialized
    */
    ctrl_c_fnd = TRUE;
    signal(SIGINT, (_crt_signal_t)sigint_hdlr); /* Reinitialize SIGINT */
}

/*
Function: DebugMode

Purpose:
    This function is part of a break-point debugging mechanism that executes the .XME file
    up to a specified stopping address or until an interruption signal (CTRL+C) is detected.

    If the execution reaches the specified stop address or if an interruption signal is detected,
    a message is printed to indicate this. If execution stops for any other reason, a warning message
    is printed.
*/
void DebugMode() {
    printf("Enter a stop address:");
    unsigned short stop_address;
    scanf("%04hx", &stop_address);
    ctrl_c_fnd = FALSE;
    signal(SIGINT, (_crt_signal_t)sigint_hdlr);

    if (stop_address & 1) {
        stop_address = stop_address - 1;
        printf("The number is odd starting at valid address %04X instead \n", stop_address);
    }

    if (stop_address > MEM_SIZE) {
        printf("Error: memory max value exceeded\n ");
        return -1;
    }

    printf("Running program to address : %04hx\n", stop_address);
    int pri_isa = 0;
    while (PC != stop_address && !ctrl_c_fnd) { //&& !ctrl_c_fnd
        Control();
    }

    if (PC == stop_address && !ctrl_c_fnd) printf("Break point reached at address %04hx \n", PC);
    else if (ctrl_c_fnd) printf("CTRL + C detected stoped at address %04hx\n", PC);
    else(YELLOW "\n Warning: stopped at address %04hx\n" RESET, PC);
}

/*
 *   Purpose: Displays the content of all the registers in the register file and
 *            the location in memory they point to.
 */
void PrintRegMem(int iter) {
    unsigned short data = RegFile[0][iter];
    Bus(RegFile[0][iter], &data, R, WORD);
    printf(" -------------> |_%04X_|\n", data);

}
void DisplayRegisters() {
    for (int j = 0; j < NUM_REG; j++) {
        printf("R%d: %04X", j, RegFile[REG][j]);

        PrintRegMem(j);
    }
}
/*
 *   Prints the binary representation of the provided 16-bit unsigned short value.
 *   Parameters:
 *   value: The 16-bit unsigned short value to be printed in binary.
 */
void PrintBits(unsigned short value) {

    int numBits = 16;
    printf("Bit Print: ");
    printf(" BIT 15 ---> ");
    for (int bit = numBits - 1; bit >= 0; --bit) {
        printf("%d", Hex_2_Bit(value, bit));
    }
    printf(" <--- BIT 0 ");
    printf("\n");
}


/*
 *   Purpose: Displays all PSW bits.
 */
void PrintPswValues() {
    printf("PSW Values:\n");
    printf("c: %u\n", pswptr->c);
    printf("z: %u\n", pswptr->z);
    printf("n: %u\n", pswptr->n);
    printf("v: %u\n", pswptr->v);
}

/*
 *   Purpose: Prints the contents of memory from the start to the end addresses in both hexadecimal and ASCII formats.
 *            Each line of the printout includes a memory address, a series of hex values,
 *            and their corresponding ASCII characters. Unprintable characters are represented by a '.'.
 *   Parameters:
 *   start: Pointer to the start of the memory segment to be printed.
 *   end: Pointer to the end of the memory segment to be printed.
 *   start_address: The memory address corresponding to the start pointer.
 *
 *
 */
void PrintMem(unsigned char* start, unsigned char* end, unsigned short start_address) {

    unsigned char* ptr = start;

    while (ptr < end) {
        unsigned char* line_start = ptr;
        unsigned char* line_end = ptr + MAX_LINE_SIZE;
        if (line_end > end) {
            line_end = end;
        }
        printf(YELLOW "0x%04X: " RESET, start_address);

        // Print hex values
        for (ptr = line_start; ptr < line_end + 1; ptr++) {
            printf("%02X ", *ptr);
        }

        // Print padding spaces
        for (int i = line_end - line_start; i < MAX_LINE_SIZE; i++) {
            printf("   ");  // 3 spaces for each missing hex value
        }

        printf("    ");
        printf("\033[34m");  // Set the text color to blue

        // Print ASCII characters
        for (ptr = line_start; ptr < line_end; ptr++) {
            if (isprint(*ptr)) {
                printf("%c", *ptr);
            }
            else {
                printf(".");
            }
        }


        start_address += MAX_LINE_SIZE;
        printf("\033[0m");  // Reset the text color to the default
        printf("\n");
    }


}

/*
 *   Purpose: Displays a list of commands available to the user in the debugging console.
 */
void PrintInstructions() {
    printf("\033[1;36m");
    printf("\n\n");
    printf("################################################################################\n");
    printf("###################### Welcome to the X-Makina Emulator! #####################\n");
    printf("################################################################################\n");
    printf("\033[0m");
    printf("\n\n");

    printf("\033[1;31m----- Program Flow Commands -----\033[0m\n");
    printf("    C   : Continue to the next instruction\n");
    printf("    PC  : Change the program-counter\n");
    printf("    BK  : Add a break point to a specific Address\n");
    printf("    PW  : Update PSW (Warning this will affect program flow\n");
    printf("    L  : Print CPU Clock\n");
    printf("\n");

    printf("\033[1;32m----- Printing Commands -----\033[0m\n");
    printf("    PR  : Display all Registers and Constants\n");
    printf("    PM  : Print one byte/word stored in Memory\n");
    printf("    PB  : Print Instruction Register value in bits\n");
    printf("    PH  : Print Cache table\n");
    printf("    PS  : Print Program Status Word\n");
    printf("\n");

    printf("\033[1;33m----- File Control Commands -----\033[0m\n");
    printf("   E   : End the program\n");
    printf("   NF  : Load New .xme File in memory");
    printf(YELLOW " (Warning: This might overwrite the contents loaded to memory)\n" RESET);
    printf("\n");

    printf("\033[1;34m----- Other Commands -----\033[0m\n");
    printf("\n");
    printf("    H   : Display All instructions\n");

    printf("\n");
    printf("\033[1;36m");
    printf("################################################################################\n");
    printf("################################################################################\n");
    printf("\033[0m");
    printf("\n");
}

/*
 *   Purpose: Interacts with the user, allowing them to inspect and control the XM-23 emulator.
 *            The function provides multiple user commands to inspect and manipulate the state of
 *            the XM-23 emulator.
 */
void Controller() {
    /************ Debugger startup software ************/
    unsigned short pc_input;
    unsigned short start, end;

    int input_choice;
    int reg_num;
    int update_psw;

    char* primitive[] = { "c", "e", "pc", "pr", "pm", "pb", "ps", "bk", "nf", "a", "pw","l","h" };

    char input[3]; // Increase the size to accommodate the null terminator
    bool debug = true;
    PrintInstructions();
    while (debug) {//&& !ctrl_c_fnd
        scanf("%2s", input);
        // Convert the input to lowercase
        for (int i = 0; input[i]; i++) {
            input[i] = tolower(input[i]);
        }
        switch (input[0]) {
        case 'c':
            Control();
            break;

        case 'p':
            switch (input[1]) {
            case 'c':
                printf("Current PC  : %04X\n", PC);
                printf("Enter value for PC (in hex form): ");
                fscanf(stdin, "%04hX", &pc_input);

                // Ensure Address is Even:
                if (pc_input & 1) {
                    pc_input = pc_input - 1;
                    printf("The number is odd starting at valid address %04X instead \n", pc_input);
                }
                // Assign pc_value to input if necessary
                PC = pc_input;
                Control();
                printf("NEW PC : %2X \n", PC);
                break;
            case 'r':
                printf("Displaying Register's  (In HEX format)\n");
                DisplayRegisters();
                break;
            case 'm':

                printf("Enter start index (IN HEX): ");
                fscanf(stdin, "%04hX", &start);

                printf("Enter end index (IN HEX): ");
                fscanf(stdin, "%04hX", &end);

                // Check if the indices are within bounds
                if (start < 0 || start >= BYTE_MEM_SIZE || end < 0 || end >= BYTE_MEM_SIZE || start > end) {
                    printf("Error: Invalid indices. Think & Try again\n");
                }
                else {
                    // Print the specified range of memory
                    PrintMem(&memory_u.ByteMem[start], &memory_u.ByteMem[end], start);
                }
                break;
            case 'b':

                printf("Enter 1 to print a register's content or 2 to print Instruction register as a binary array: ");
                fscanf(stdin, "%d", &input_choice);
                if (input_choice == 1) {
                    printf("\nEnter Register number:");
                    fscanf(stdin, "%d", &reg_num);
                    if (reg_num > 7) printf("Error: only registers 0 to 7 possible \n");
                    PrintBits(RegFile[0][reg_num]);
                }
                else if (input_choice == 2) PrintBits(instr_reg);
                else  printf(RED "Error: nope either 1 or 2 only \n" RESET);

                break;
            case 's':
                PrintPswValues();
                break;
            case 'w':

                printf(" To change Z (1) C (2) V (3) N (4): ");
                fscanf(stdin, "%d", &update_psw);
                switch (update_psw)
                {
                case 1:
                    psw.z = psw.z ^ 1;
                    break;
                case 2:
                    psw.c = psw.c ^ 1;
                    break;
                case 3:
                    psw.v = psw.v ^ 1;
                    break;
                case 4:
                    psw.n = psw.n ^ 1;
                    break;

                default:
                    printf(RED "Human Error: That is not an option\n" RESET);
                    printf("H   - Display All instructions\n");
                    break;
                }
            case 'h':
                PrintCache();
                break;
            defult:
                printf(RED "Human Error: That is not an option\n" RESET);
                break;


            }
            break;
        case 'b':
            DebugMode();
            break;
        case 'n':
            OpenLoadF(0, NULL);
            break;

        case 'h':
            PrintInstructions();
            break;
        case 'e':
            printf("Halting program goodbye :)\n");
            goto exit_loop;

        case 'l':
            printf("Current CPU Clock %010d\n", CPU_CLOCK);
            break;
        default:
            printf(RED "Human Error: That is not an option\n" RESET);
            printf("H   - Display All instructions\n");
            continue;

        }
    }
    printf("Halting program goodbye :)\n");
exit_loop:
    debug = false;
}