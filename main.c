/*
 *  File: main.c
 *  Purpose:
 *  This is the main file for the X-Makina Emulator. This program emulates the central processing unit (CPU)
 *  of the XM-23 machine, with fetch, decode, and execute capabilities. With functionality to 
 *  read S-Record formatted files, extracting and decoding information.
 */
/*TO RUN PROGRAM ON Visual Studio go to debug properties > c/c++ > preprocessor > _CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h> /* Signal handling software */

#include "emulator.h"


union Memory memory_u;

unsigned short instr_reg;




int main(int argc, char* argv[]) {
    printf("ECED3403 - Computer Architecture Assigment 1\n");
    printf("X-Makina (XM-23) Emulator\n");
    printf("Developed by Omar Hameeed (B00764655)\n");
    printf("\n");

    if (OpenLoadF(argc, argv) != 0) {
        printf("Error opening file. Exiting program.\n");
        return 1;
    }

    CPU_CLOCK = 0;
    Controller();

    return 0;
}


