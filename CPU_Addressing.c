
/**
 * @file CPU_Addressing.c
 * @brief Addressing Modes and Branching Operations for XM23
 *
 * This module contains functions for demonstrating different addressing
 * modes (indexed and relative), register move operations and branching
 * strategies (conditional and linking) within the context of the XM23
 * processor. This forms a critical part of the emulation process,
 * helping to control the flow of execution and manage memory effectively.
 *
 * This is part of the coursework for ECED3403 - Computer Architecture, Assignment 1.
 *
 * @author Omar Hameed
 * @date Last updated on June 18, 2023
 *
 * ==============================================================================
 */
#include "emulator.h"
#include "Cache.h"
//#define ReltiveAdressDebug
//#define Branch_DEBUG

/**
 * Purpose: Handles the Indexed Addressing Mode for Load (LD) and Store (ST) operations.
 * It calculates the effective address based on multiple bits (PREPOS, DEC, INC)
 * 
 * @param load_store: An enum indicating the operation to be performed. LD for a load operation, ST for a store operation.
 */
void IndexedAddressing(extern enum IndexedAddressings load_store) {
    CPU_CLOCK += 1;
    unsigned short  dst = DST(instr_reg);
    unsigned short  src = SRC(instr_reg);
    unsigned char   word_byte = Hex_2_Bit(instr_reg, 6);
    unsigned char   PREPOS = Hex_2_Bit(instr_reg, 9);
    unsigned char   DEC = Hex_2_Bit(instr_reg, 8);
    unsigned char   INC = Hex_2_Bit(instr_reg, 7);

    unsigned char WORD_BYTE = Hex_2_Bit(instr_reg, 6);
    unsigned short EffectiveAddress;
    unsigned short  address_modifiers;
    if (load_store == LD)
        address_modifiers = RegFile[0][src];
    else
        address_modifiers = RegFile[0][dst];


    switch (PREPOS << 2 | DEC << 1 | INC) {
    case Normal:  // '000'
        EffectiveAddress = address_modifiers;
        break;

    case POS_INC:  // '001'
        //Post increment REG
        EffectiveAddress = address_modifiers;
        address_modifiers = address_modifiers + 2 - WORD_BYTE;
        break;
    case POS_DEC:  // '010'
        //Post-decrement the register
        EffectiveAddress = address_modifiers;
        // Decrement by - 1 or - 2 depending if WORD -2 OR BYTE - 1
        address_modifiers = address_modifiers - 2 + WORD_BYTE;
        break;

    case PRE_INC:  // '101'
        // PRE increment REG
        // Access memory and add 2 if word or add 1 if byte (WORD_BYTE = 1)
        EffectiveAddress = address_modifiers + 2 - WORD_BYTE;
        address_modifiers = EffectiveAddress;
        break;


    case PRE_DEC:  // '110'
        //Pre-decrement the register
        EffectiveAddress = address_modifiers - 2 + WORD_BYTE;
        address_modifiers = EffectiveAddress;
        break;


    default:
        // Handle invalid case
        printf("Error: PRPO | DEC | INC | has an undefined combination\n");
        break;
    }

    if (load_store == LD) {

#ifdef DEBUG
        printf(" EffectiveAddress= %4X\n", EffectiveAddress);
#endif

        //Bus(EffectiveAddress, &RegFile[0][dst], R, WORD_BYTE);
        Cache(EffectiveAddress, &RegFile[0][dst], R, WORD_BYTE);
        if (src!=dst) RegFile[0][src] = address_modifiers;
    }
    else {

#ifdef DEBUG
        printf(" EffectiveAddress= %4X\n", EffectiveAddress);
#endif
        // Cache(unsigned short address, unsigned short* content,
        // unsigned char read_write, unsigned char word_byte, unsigned char wrt_back_thro
        Cache(EffectiveAddress, &RegFile[0][src], WR, WORD_BYTE);

        // Bus(EffectiveAddress, &RegFile[0][src], WR, WORD_BYTE);
        if (src != dst) RegFile[0][dst] = address_modifiers;
    }

}

/**
 * Purpose: Handles the Relative Addressing Mode for Load (LDR) and Store (STR) operations.
 *          It calculates the effective relative address based on the source or destination register and a postive 
 *          or negative offset.
 *
 */

void RelativeAddressing() {
    CPU_CLOCK += 1;
    unsigned short dst = DST(instr_reg);
    unsigned short src = SRC(instr_reg);
    unsigned char word_byte = Hex_2_Bit(instr_reg, 6);
    // Checking the signed bit from OG intput this is bit 13
    unsigned char signed_bit = Hex_2_Bit(instr_reg, 13);
    unsigned short offset = REL_AD_MASK(instr_reg);
    unsigned short RelativeAddress;
    // IF signed bit set sign extend 
    offset = (signed_bit) ? offset | 0XFF80 : offset;
    RelativeAddress = (Hex_2_Bit(instr_reg, 14) == 1) ? (RegFile[0][dst] + offset) : (RegFile[0][src] + offset);
#ifdef ReltiveAdressDebug
    printf("OFFSET: %4X\n", offset);
    printf("Reltive Adress: %4X \n", RelativeAddress);
#endif // !ReltiveAdressDebug

    // STR
    if (Hex_2_Bit(instr_reg, 14)) Bus(RelativeAddress, &RegFile[0][src], WR, word_byte);
    // LDR
    else Bus(RelativeAddress, &RegFile[0][dst], R, word_byte);

}

/**
 * Performs a all of move operations, which handle different parts of the destination register (high or low byte, or sign).
 * *
 */

void Movs() {
    CPU_CLOCK += 1;
    unsigned short byte_mov = MOV_B(instr_reg);
    unsigned short dst = DST(instr_reg);
    switch (Hex_2_Bit(instr_reg, 12) << 1 | Hex_2_Bit(instr_reg, 11)) {
        case MOVL:
            // MOVL 
            // 1) High Byte and with 11111111 00000000  
            RegFile[REG][dst] = RegFile[REG][dst] & SET_HI;
            // 2) OR Instruction with byte 
        
            break;

        case MOVLZ:
            // MOVLZ
            // 1) High byte  is zero'd by anding with 0x0000 
            RegFile[REG][dst] = RegFile[REG][dst] & CLEAR_ALL;
            // 2) value stored into the LOW byte using OR 
            break;
        case MOVLS:
            // MOVLS
            // MSB Is set to 1's by OR with  11111111 00000000 
            RegFile[REG][dst] = RegFile[REG][dst] | SET_HI;
            // 2) using AND we set LSB to zero's
            RegFile[REG][dst] = RegFile[REG][dst] & SET_HI;
            // 3) using OR we set LSB to Byte
            break;
        case MOVH:
            // MOVH
            // 1) High bit set to zero LSB Unchanged
            RegFile[REG][dst] = RegFile[REG][dst] & SET_LOW;
            // 2) shift Byte so it aligns with high byte then OR 
            byte_mov = byte_mov << 8;
            break;
    }
    RegFile[REG][dst] = RegFile[REG][dst] | byte_mov;
}

/**
 * Purpose: Performs most of branch operations based on the branch type and the PSW, 
 *          and modifies the program counter (PC) depending on the branch operation
 * 
 *
 * @param branch_type: An enum indicating the branch type. Valid values are BEQ, BNE, BC, BNC, BN, BGE, BLT, and BRA.
 */
void Branching(enum BR branch_type) {
    CPU_CLOCK += 1;
#ifdef PrintInstra
    printf("\n");
#endif
 
    unsigned int signed_bit = Hex_2_Bit(instr_reg, 9);
    // shifting 1 bit to the left so that | sign |x| EncodedOffset | 0 |
    instr_reg = instr_reg << 1;
    unsigned short branchedPC;

    if (signed_bit)
    {
        instr_reg = instr_reg | SEXT_BRA; 
        branchedPC = PC + instr_reg;

#ifdef Branch_DEBUG
        printf("Branch PC:  %hx\n", branchedPC);
        printf("\n Binary OFFSET AFTER SHIFT + Set (SIGNED Bit) \n ");
        printBits(instr_reg);
#endif

    }
    else
    {
        instr_reg = instr_reg & 0X1FF;
        branchedPC = instr_reg + PC;

#ifdef Branch_DEBUG
        printf("Branch PC   :  %hx\n", branchedPC);
        printf("\n Binary OFFSET AFTER SHIFT + Set (Non-SIGNED Bit) \n");
        printBits(instr_reg);
#endif
    }
#ifdef Branch_DEBUG
    printf("\n Branched PC:  %hx\n", branchedPC);
    printf("Current PC: %hx\n", PC);

#endif //
    switch (branch_type)
    {

    case BEQ:
        PC = psw.z == 1 ? branchedPC : PC; break;
    case BNE:
        PC = psw.z == 0 ? branchedPC : PC; break;
    case BC:
        PC = psw.c == 1 ? branchedPC : PC; break;
    case BNC:
        PC = psw.c == 0 ? branchedPC : PC; break;
    case BN:
        PC = psw.n == 1 ? branchedPC : PC; break;
    case BGE:
        PC = (psw.n ^ psw.v) == 0 ? branchedPC : PC; break;
    case BLT:
        PC = (psw.n ^ psw.v) == 1 ? branchedPC : PC; break;
    case BRA:
        PC = branchedPC; break;
    default: break;
    }

}

/**
 * Purpose: Handles the Branch and Link (BL) operation only. It stores the current program counter (PC) in the link register (LR)
 *          and then branches to a new location by adding a signed offset to the PC.
 *
 */
void BranchLink() {
    CPU_CLOCK += 1;
#ifdef DEBUG
    printf("\n");
    printf("\nEncoded offset:  %hx\n", instr_reg);
    printf("Current PC: %hx\n", PC);
#endif
    int signed_bit = Hex_2_Bit(instr_reg, 12);
    // shifting 1 bit to the left so that | sign |x| EncodedOffset | 0 |
    instr_reg = instr_reg << 1;
    unsigned short branchedPC;
#ifdef DEBUG
    printf("\n Binary OFFSET AFTER SHIFT\n ");
    printBits(instr_reg);
#endif
    if (signed_bit)
    {
        instr_reg = instr_reg | SEXT_BL; 
    }
    else
    {
        instr_reg = SET_BIT(instr_reg, 12, 0);
    }
    // Storing PC into Link Register or R5
    LR = PC;
    PC = PC + instr_reg;

#ifdef DEBUG
    printf("\n Binary OFFSET AFTER SHIFT + Set \n");
    printBits(instr_reg);
    printf("Branch PC:  %hx\n", PC);
#endif
}






