
/**
 * @file  : CPU_operations.c
 * @brief : CPU Emulation Operations for the XM-23 machine 
 * 
 *
 * This module contains functions for emulating Coping/Swamping register contents, sign extending,
 * Shifting/Rotating right with carry, swapping Bytes of a register and arithmetic operations 
 * in the XM23 processor architecture. Functions included
 * are capable of executing different operation codes emulating a CPU, with
 * capabilities ranging from memory handling to complex arithmetic calculations.
 *
 * This is part of the coursework for ECED3403 - Computer Architecture, Assignment 1.
 *
 * @author Omar Hameed
 * @date Last updated on July 15, 2023
 *
 * 
 */

#include "emulator.h"
//#define DEBUG
//#define SwapDebug
//#define ARITH_DEBUG
//#define ADDC

/**
 * Purpose: Simulates the MOV and SWAP operations of a processor.
 *         MOV copies a value from the source register to the destination register.
 *         SWAP exchanges the values between the source and destination registers.
 *
 * @param operation: An integer that indicates the operation to be performed. MOV for a move operation, SWAP for a swap operation.
 */
void Mov_SWAP(int opration) {
    CPU_CLOCK += 1;
    // Src register points to an adress in memory 
    unsigned char word_byte = Hex_2_Bit(instr_reg, 6);

    unsigned short dst_reg = DST(instr_reg);
    unsigned short src_reg = SRC(instr_reg);
    unsigned short src_val;
    unsigned short result;

    src_val = RegFile[REG][src_reg];

    
    switch (opration)
    {
    case MOV:
        result = (word_byte == WORD) ? src_val :  (unsigned char)src_val;
        RegFile[REG][dst_reg] = result;
        break;
    
    case SWAP:
        RegFile[REG][src_reg] = RegFile[REG][dst_reg];
        RegFile[REG][dst_reg] = src_val;
        break;

    default:
        break;
    }
}

/**
 * Purpose: Simulates the SXT (Sign Extend) and COMP (One's Complement) operations in a processor.
 *
 * @param operation: An enum indicating the operation to be performed. 
 */

void SignChange(extern enum OneOprands opration) {
    CPU_CLOCK += 1;
    unsigned short dst_reg = DST(instr_reg);
    unsigned short dst_val;
    unsigned short result;
    unsigned char sxt_bit;
    unsigned char word_byte;
    word_byte = Hex_2_Bit(instr_reg, 6);
    dst_val = RegFile[REG][dst_reg];
    unsigned short hi_byte = dst_reg & 0xFF00;

    switch (opration)
    {
  
    case SXT:
        result = dst_val;
        sxt_bit = Hex_2_Bit(RegFile[REG][DST(instr_reg)], 7);
        result = (sxt_bit == 0) ? result & SET_LOW : result | SET_HI;

        break;

    case COMP:
        result = (word_byte == WORD) ? ~ dst_val : ~(unsigned char)dst_val;
        break;
    default:
        printf("Error: Unkown OPCODE\n");
        break;
    }
    if (word_byte == BYTE) result = hi_byte |result;
    RegFile[REG][dst_reg] = result ; 
}

/**
 * Purpose: Simulates the DADD (Decimal Add) operation in a processor.
 * DADD adds the source and destination values in binary-coded decimal (BCD) representation.
 *
 * @param srcValue: The source value for the operation.
 * @param dstValue: The destination value for the operation.
 * @param word_byte: Flag to indicate if the operation is byte-wise or word-wise.
 * @return: The result of the DADD operation. 
 */
unsigned short Dadd(unsigned srcValue, unsigned dstValue, unsigned char word_byte) {
    CPU_CLOCK += 1;
    // This function uses two unions of bcd_digits one for source and the other for dst 
    unsigned short temp_carry = psw.c;
    union bcd_digits src_ip, dst_ip;
    // SrcValue and dstValue is loaded in the memory which is then accessed using a union
    src_ip.data_val = srcValue;
    dst_ip.data_val = dstValue;

    // 4 bits accessed in the struct using fields in the struct dadd_bits which is inside the uinion bcd_digits
    dst_ip.nibble.n_0 = BcdAdd(src_ip.nibble.n_0, dst_ip.nibble.n_0, &temp_carry);
    dst_ip.nibble.n_1 = BcdAdd(src_ip.nibble.n_1, dst_ip.nibble.n_1, &temp_carry);
    // If it's a word then 4 bytes must be accessed 
    if (!word_byte) {
        dst_ip.nibble.n_2 = BcdAdd(src_ip.nibble.n_2, dst_ip.nibble.n_2, &temp_carry);
        dst_ip.nibble.n_3 = BcdAdd(src_ip.nibble.n_3, dst_ip.nibble.n_3, &temp_carry);
    }
    psw.c = temp_carry;
    return dst_ip.data_val;


}

/**
 * Purpose: Adds two binary-coded decimal (BCD) nibbles along with a carry.
 *
 * @param nibble_x: The first BCD nibble.
 * @param nibble_y: The second BCD nibble.
 * @param carry: Pointer to the carry value, which is also updated by this function.
 * @return: The sum of the two nibbles and the input carry. If the sum exceeds 9, it wraps around by 10, and the carry is set to 1.
 */

unsigned short BcdAdd(unsigned short nibble_x, unsigned short nibble_y,unsigned short *carry) 
{
    CPU_CLOCK += 1;
    unsigned short result;

    result = nibble_x + nibble_y + *carry;
    // if result is bigger than 9 in deducts 10 and sets carry bit else carry bit is cleared and result is un-altered 
    if (result > 9) {
        result = result - 10;
        *carry = 1;
    }
    else {
        *carry = 0;

    }

    return result;
}

/**
 * Purpose: Generalized function to preform most arithmetics
 *
 * @param src: The source value for the operation.
 * @param dst: The destination value for the operation.
 * @param temp_carry: Carry value, different for each arithmetic operation
 * @param word_byte: Flag to indicate if the operation is byte-wise or word-wise.
 * @return: The result of the arithmetic operation. This function also updates the PSW.
 */
unsigned short Addc(unsigned short src, unsigned short dst, unsigned short temp_carry, char word_byte) {
    CPU_CLOCK += 1;
    unsigned short dst_high;
    unsigned short result;
    dst_high = dst & 0xFF00;
    if (word_byte == WORD) {
        result = dst + src + temp_carry;
        update_psw((src + temp_carry), dst, result, word_byte);
    }
    else {
        unsigned char src_lsb = src & 0x00FF; // Get the LSB of src
        unsigned char dst_lsb = dst & 0x00FF; // Get the LSB of dst
        (unsigned char)result = src_lsb + dst_lsb + temp_carry;        
        (unsigned char)dst = dst & 0XFF00;
        result = dst | result;
        update_psw((src_lsb + temp_carry), dst_lsb, (unsigned char)result, word_byte);
    }

    if (word_byte == BYTE) result = result | dst_high;
#ifdef ADDC
    printf("DST: %04X SRC: %4X RES: %4X\n", src, dst, result);
#endif
    return result;
}

void SRA_RRC(int oprartion) {
    CPU_CLOCK += 1;
    unsigned int msb_value;
    unsigned int temp_carry;
    unsigned int msb_position;
    unsigned char word_byte = Hex_2_Bit(instr_reg, 6);
    unsigned short dst = DST(instr_reg);
    // -> MSB is eithr bit 7 when its a BYTE or bit 15 when its a WORD
    msb_position = (word_byte) ?  7: 15;
    /*
    * SRA:
    * Arithmetic shift to the right by 1 BIT with Sign Extension (word or byte)
    * MSB Is Unchanged 
    * PSW.C is set through arithmetic shift
    * DST.MSB → … → DST.LSB → C
    */
    if (oprartion == SRA) {
        temp_carry = (word_byte) ? Hex_2_Bit(RegFile[REG][dst], 7) : Hex_2_Bit(RegFile[REG][dst], 15);
    }
    /*
    * RRC
    * Rotate DST right by 1 bit through the carry (word or byte)
    * C → DST.MSB → … → DST.LSB → C 
    */
    else {
        temp_carry = psw.c;
        psw.c = Hex_2_Bit(RegFile[REG][dst], 0);
    }

    // If a byte update MSB so its unchanged 
    if (word_byte) (unsigned char)RegFile[REG][dst] >>= 1;
    else RegFile[REG][dst] >>= 1;
    SET_BIT(RegFile[REG][dst], msb_position, temp_carry);
}

void SwapPB() {
    CPU_CLOCK += 1;
    /*
    * (Word Only)
    * Swaps Bytes in DST
    * TMP <--- DST.MSB
    * DST.MSB <--- DST.LSB
    * DST.LSB <--- TMP
    */
    unsigned short dst_val = RegFile[0][DST(instr_reg)];
    #ifdef SwapDebug

    printf("\n OLD DEST: %4x\n", dst_val);
    #endif
    unsigned char msB = (dst_val >> 8) & 0xFF;  // Extract MSB (Most Significant Byte)
    unsigned char lsB = dst_val & 0xFF;         // Extract LSB (Least Significant Byte)
    dst_val = (lsB << 8) | msB;  // Swap the bytes
    RegFile[0][DST(instr_reg)] = dst_val;
    #ifdef SwapDebug
        printf(" NEW DEST: %4x\n", RegFile[0][DST(instr_reg)]);
    #endif

}

/**
 * Purpose: Handles all of arithmetic operations either directly or by calling Addc and setting the temp_carry according
 *          to the operation.
 *
 * @param operation: An enum indicating the operation to be performed.
 */
void Arithmetic(extern enum Arithmetics opration) {
    CPU_CLOCK += 1;
#ifdef PrintInstra
    printf("\n");
#endif
    unsigned char word_byte = Hex_2_Bit(instr_reg, 6);
    unsigned char reg_const = Hex_2_Bit(instr_reg, 7);

    unsigned short dst = DST(instr_reg);
    unsigned short src = SRC(instr_reg);
    unsigned short result;
    unsigned short dstValue;
    unsigned short srcValue;

#ifdef ARITH_DEBUG
    printf("R/C: %hx\n", reg_const);
    printf("W/B: %hx\n", word_byte);
#endif


    dstValue = RegFile[REG][dst];
    srcValue = RegFile[reg_const][src];
#ifdef ARITH_DEBUG

    printf("Source: %hx\n", src);
    printf("Destination: %hx\n", dst);
    printf("DEST & SRC VALS\n");
    printBits(dstValue);
    printBits(srcValue);

#endif // DEBUG


#ifdef Debug

#endif
    if (opration == BIT || opration == CMP) {
        if (opration == CMP) {
            (void)Addc(~srcValue, dstValue, 1, word_byte);
#ifdef ARITH_DEBUG
            printf("CMP\n");
#endif
        }
        else {
            // BIT  
            if (word_byte == BYTE && srcValue > ByteLength) printf(YELLOW "Warning: Byte Operation but Source value is bigger than 8 \n" RESET);
            result = (word_byte == BYTE) ? (dstValue & (1 << srcValue)) : (unsigned char)((dstValue & (1 << srcValue)));
            update_psw_2(result, word_byte);
#ifdef ARITH_DEBUG
            printf("BIT\n");
#endif
        }
    }
    else {
        switch (opration)
        {
        case ADD:
            result = Addc(srcValue, dstValue, 0, word_byte);
#ifdef ARITH_DEBUG
            printf("ADD\n");
#endif
            break;

        case ADDC:
            result = Addc(srcValue, dstValue, psw.c, word_byte);
#ifdef ARITH_DEBUG
            printf("ADDC\n");
#endif
            break;

        case SUB:
            result = Addc(~srcValue, dstValue, 1, word_byte);
#ifdef ARITH_DEBUG
            printf("SUB\n");
#endif
            break;

        case SUBC:
            result = Addc(~srcValue, dstValue, psw.c, word_byte);
#ifdef ARITH_DEBUG
            printf("SUBC\n");
#endif
            break;

        case XOR:
            result = (word_byte == WORD) ? ((dstValue ^ srcValue) + psw.c) : ((unsigned char)(dstValue ^ srcValue)) + psw.c;
            update_psw_2(result, word_byte);
#ifdef ARITH_DEBUG
            printf("XOR\n");
#endif
            break;
        case AND:
            result = (word_byte == WORD) ? ((dstValue & srcValue) + psw.c) : ((unsigned char)dstValue & (unsigned char)srcValue) + psw.c;
            update_psw_2(result, word_byte);
#ifdef ARITH_DEBUG
            printf("AND\n");
#endif
            break;
        case OR:
            result = (word_byte == WORD) ? (dstValue | srcValue) : (unsigned char)(dstValue | srcValue);
            update_psw_2(result, word_byte);
#ifdef ARITH_DEBUG
            printf("OR\n");
#endif
            break;
        case BIC:
            if (word_byte == 1 && srcValue > ByteLength) printf(YELLOW "Warning: Byte Operation but Source value is bigger than 8 \n" RESET);
            result = (word_byte == WORD) ? (dstValue & ~(1 << srcValue)) : (unsigned char)(dstValue & ~(1 << srcValue));
            update_psw_2(result, word_byte);
#ifdef ARITH_DEBUG
            printf("BIC\n");
#endif
            break;
        case BIS:
            if (word_byte == 1 && srcValue > ByteLength) printf(YELLOW "Warning: Byte Operation but Source value is bigger than 8 \n" RESET);
            result = (word_byte == WORD) ? (dstValue | (1 << srcValue)) : (unsigned char)(dstValue | (1 << srcValue));
            update_psw_2(result, word_byte);
#ifdef ARITH_DEBUG
            printf("BIS\n");
#endif
            break;
        case DADD:
            result = Dadd(srcValue, dstValue, word_byte);
            update_psw_2(result, word_byte);
#ifdef ARITH_DEBUG
            printf("DADD\n");
#endif
            break;

        default:
            printf("Program Error: Unkown Arithmetic operation %d\n", opration);
            return -1;
            break;
        }
        
        RegFile[0][dst] = result;
    }
}

