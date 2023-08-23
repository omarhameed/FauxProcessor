
/**
 * @file CPU.c
 * @brief Central Processing Unit Emulator
 *
 * This program emulates the central processing unit (CPU) of the XM-23 machine.
 * The CPU emulator features instruction fetch, decode, and execute capabilities. It is
 * equipped to handle memory operations, branching instructions, and supports
 * multiple addressing modes [CPU_addressing.c] and preforms arithmetic operations [CPU_Arithmetic].
 *
 * This file is part of the Assignment 1 submission for the course ECED3403 - Computer Architecture.
 *
 * @author Omar Hameed
 * @date Last updated on July 15, 2023
 *
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "emulator.h"

//#define PrintInstra
// #define BusDEBUG

// 2-d array to hold registers and constants
unsigned short RegFile[REG_CONS][NUM_REG] = {
    {0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000},
    {0, 1, 2, 4, 8, 16, 32, -1}
};

/**
 * purpose: Simulates a bus that reads from or writes to memory.
 *
 * @param mar: The memory address to be accessed.
 * @param mdr: Pointer to the data to be written or read into/from memory.
 * @param read_write: 0 for read operation, 1 for write.
 * @param word_byte: 0 for a word (2 bytes) operation, 1 for a byte operation.
 * @return: void. Modifies the mdr or memory directly. Prints error if mdr is NULL during write.
 */
void Bus(unsigned short mar, unsigned short* mdr, int read_write, int word_byte) {
    
    CPU_CLOCK += 3;

    assert(word_byte == 0 || word_byte == 1);
    assert(read_write == 0 || read_write == 1);
    if (read_write == 0) {  // read = 0 
        if (word_byte == 0) {
            *mdr = memory_u.WordMem[mar >> 1];

#ifdef BusDEBUG
            printf(" Bus Read Word Function: ADDRESS -> %04X Memory stored -> %04X  \n", mar/2, memory_u.WordMem[mar/2]); //>> 1
#endif // BusDEBUG
        }
        else {
            *mdr = (unsigned char)memory_u.ByteMem[mar];
#ifdef BusDEBUG
            printf(" Bus Read Byte Function: ADDRESS -> %2X Memory stored -> %4x  \n", mar, memory_u.ByteMem[mar]);
#endif // BusDEBUG
        }
    }

    else { // write = 1 
        if (mdr == NULL) {
            printf("\n");
            printf("Error: Segmentation fault, read access violation mdr was accessed while being a nullptr.\n");
            printf("Instruction Skipped\n");
            printf("\n");
            return 0;
        }
        if (word_byte == 0) { // word = 0

            memory_u.WordMem[mar>>1] = *mdr;
#ifdef BusDEBUG
            printf(" Bus Write Word Function: ADDRESS -> %04X Memory stored -> %04x  \n", mar, memory_u.WordMem[mar>>1]);
#endif // BusDEBUG
        }
        else {
            memory_u.ByteMem[mar] = (unsigned char)(*mdr & 0xFF);
#ifdef BusDEBUG
            printf(" Bus Write Byte Function: ADDRESS -> %04X Memory stored -> %04x  \n", mar, memory_u.ByteMem[mar]);
#endif // BusDEBUG
        }
    }


}

/**
 * purpose: Simulates the control flow of a processor.
 *          It sequentially calls the Fetch and Decode operations 
            and increments CPU_CLOCK for each operation.
 */
void Control() {

    Fetch();
    CPU_CLOCK+=1;
    Decode();
    CPU_CLOCK+=1;
    

}

/**
 * Simulates the fetch operation in a processor's instruction cycle.
 * It reads a 16-bit instruction into the instruction register and increments the program counter.
 *
 */
void Fetch() {

    Bus(PC, &instr_reg, R, WORD);
    PC = PC + 2;
    
}

/**
 * Simulates the decode operation in a processor's instruction cycle.
 * It decodes the instruction stored in the instruction register (instr_reg) and calls the corresponding
 * function to handle each type of operation.
 *
 */
void Decode() {
#ifdef DEBUG
    printf("\n Binary being decoded \n");
    printBits(instr_reg);
#endif


    if (Hex_2_Bit(instr_reg, 15)) {
        // LDR/STR instruction
            if (Hex_2_Bit(instr_reg, 14)) {
#ifdef PrintInstra
                printf("STR\n");
#endif
                RelativeAddressing();
            }
            else {
#ifdef PrintInstra
                printf("LDR\n");
#endif
                RelativeAddressing();
            }

        
    }
    
    else {
            // Branch instruction If bit 14 is clear
        if (Hex_2_Bit(instr_reg, 14) == 0) {
            if (Hex_2_Bit(instr_reg, 13) == 0) {
#ifdef PrintInstra
                printf("BL\n");

#endif // !PrintInstra
                BranchLink();
            }
            else {
            int BR;
                
                    switch (Hex_2_Bit(instr_reg, 12) << 2 | Hex_2_Bit(instr_reg, 11) << 1 | Hex_2_Bit(instr_reg, 10)) {
                    case BEQ:
                        BR = BEQ;
#ifdef PrintInstra
                        printf("BEQ");
#endif
                        break;
                    case BNE:
                        BR = BNE;
#ifdef PrintInstra
                        printf(" BNE");
#endif
                        break;
                    case BC:
                        BR = BC;
#ifdef PrintInstra
                        printf("BC");
#endif
                        break;
                    case BNC:
                        BR = BNC;
#ifdef PrintInstra
                        printf("BNC");
#endif
                        break;
                    case BN:
#ifdef PrintInstra
                        printf("BN\n");
#endif
                        BR = BN;
                        break;
                    case BGE:
#ifdef PrintInstra
                        printf("BGE\n");
#endif
                        BR = BGE;
                        break;
                    case BLT:
#ifdef PrintInstra
                        printf("BLT\n");
#endif
                        BR = BLT;
                        break;
                    case BRA:

                        

#ifdef PrintInstra
                        printf("BRA\n");
#endif       
                        BR = BRA;
                        break;
                    }
                    Branching(BR);
            }
        }
        else {
            // if bits 13 and 12 clear its ADD TO CEX 
            if ( (Hex_2_Bit(instr_reg, 13)) == 0 && (Hex_2_Bit(instr_reg, 12)) == 0) {
                // IF TRUE then ADD TO AND 
                if (Hex_2_Bit(instr_reg, 11) == 0) {
                    switch (Hex_2_Bit(instr_reg, 10) << 2 | Hex_2_Bit(instr_reg, 9) << 1 | Hex_2_Bit(instr_reg, 8))
                    {
                    case ADD:
#ifdef PrintInstra
                        printf("ADD");
#endif
                        Arithmetic(ADD);
                        break;
                    case ADDC:
#ifdef PrintInstra
                        printf("ADDC");
#endif
                        Arithmetic(ADDC);
                        break;
                    case SUB:
                        
#ifdef PrintInstra
                        printf("SUB");
#endif
                        Arithmetic(SUB);
                        break;
                    case SUBC:
#ifdef PrintInstra
                        printf("SUBC");
#endif
                        Arithmetic(SUBC);
                        break;
                    case DADD:
#ifdef PrintInstra
                        printf("DADD");
#endif
                        Arithmetic(DADD);
                        break;
                    case CMP:
#ifdef PrintInstra
                        printf("CMP");
#endif
                        Arithmetic(CMP);
                        break;
                    case XOR:
#ifdef PrintInstra
                        printf("XOR");
#endif
                        Arithmetic(XOR);
                        break;
                    case AND:
#ifdef PrintInstra
                        printf("AND");
#endif
                        Arithmetic(AND);
                        break;
                    }
                    
                }
                // ELSE ITS OR to CLRCC
                else {
                    // IF bit 10 is set then MOV TO CLRCC
                    if (Hex_2_Bit(instr_reg, 10)) {
                        // if bit 8 is clear then its MOV or SWAP
                        if (Hex_2_Bit(instr_reg, 8) == 0) {
                            if (Hex_2_Bit(instr_reg, 7) == 0) {
#ifdef PrintInstra
                                printf("MOV");
#endif
                                Mov_SWAP(MOV);
                            }
                            else {
#ifdef PrintInstra
                                printf("SWAP");
#endif
                                Mov_SWAP(SWAP);
                            }
                        }
                        // else SRA TO CLRCC 
                        else {
                            //IF BIT 8 AND 7 11 THEN SETPRI TO CLRCC
                            if (Hex_2_Bit(instr_reg, 8) && Hex_2_Bit(instr_reg, 7)) {
                                printf("Error 404: I am working very hard to get this part done, thank you for your patience  \n");
                            }
                            else{
                                switch (Hex_2_Bit(instr_reg, 5) << 2 | Hex_2_Bit(instr_reg, 4) << 1 | Hex_2_Bit(instr_reg, 3))
                                {
                                case SRA:
#ifdef PrintInstra
                                    printf("SRA");
#endif
                                    SRA_RRC(SRA);
                                    break;
                                case RRC:
#ifdef PrintInstra
                                    printf("RRC");
#endif
                                    SRA_RRC(RRC);
                                    break;
                                case 0b010:
#ifdef PrintInstra
                                    printf("COMP");
#endif
                                    SignChange(COMP);
                                    break;
                                case SWAPB:
#ifdef PrintInstra
                                    printf("SWAPB");
#endif
                                    SwapPB();
                                    break;
                                case SXT:
#ifdef PrintInstra
                                    printf("SXT");
#endif                              
                                    SignChange(SXT);
                                    break;
                                defult:
#ifdef PrintInstra
                                    printf("Program Error: Invalid Opcode\n");
#endif
                                    break;

                                }

                            }
                        }
                    }
                    else {
                        switch (Hex_2_Bit(instr_reg, 9) << 1 | Hex_2_Bit(instr_reg, 8)) {
                        case ORs:
#ifdef PrintInstra
                            printf("OR");
#endif
                            Arithmetic(OR);
                            break;
                        case BITs:
#ifdef PrintInstra
                            printf("BIT");
#endif
                            Arithmetic(BIT);
                            break;
                        case BICs:
#ifdef PrintInstra
                            printf("BIC");
#endif
                            Arithmetic(BIC);
                            break;
                        case BISs:
#ifdef PrintInstra
                            printf("BIS");
#endif
                            Arithmetic(BIS);
                            break;
                        }

                    
                    }
                }
           
            }
            
            // CEX TO MOV H
            else {

                if ( (Hex_2_Bit(instr_reg, 13))== 0) {
                    // CEX LD OR ST
                    switch (Hex_2_Bit(instr_reg, 11) << 1 | Hex_2_Bit(instr_reg, 10)) {
                    case 0b00:
#ifdef PrintInstra
                        printf("CEX");
#endif
                        printf("Error 404: Something great will be here soon (not due for Assigment - 1)\n");
                        break;
                    case LD:
#ifdef PrintInstra
                        printf("LD");
#endif
                        IndexedAddressing(LD);
                        break;
                    case ST:
#ifdef PrintInstra
                        printf("ST");
#endif
                        IndexedAddressing(ST);
                        break;
                    default:
                        printf("Program Error Invalid Opcode\n");
                        break;

                    }
                }
                else { // MOVx instruction 
                    switch (Hex_2_Bit(instr_reg, 12) << 1 | Hex_2_Bit(instr_reg, 11)) {
                    case MOVL:
#ifdef PrintInstra
                        printf("MOVL");
#endif

                        break;
                    case MOVLZ:
#ifdef PrintInstra
                        printf("MOVLZ");
#endif
                        break;
                    case MOVLS:
#ifdef PrintInstra
                        printf("MOVLS");
#endif
                        break;
                    case MOVH:
#ifdef PrintInstra
                        printf("MOVH");
#endif
                        break;
                    }
                    Movs();

                }

            }
        }
    }
#ifdef PrintInstra
    printf("\n");
#endif
}

