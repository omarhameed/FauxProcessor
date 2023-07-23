/**
 *  ECED3403 - Computer Architecture Assignment 1
 * @file emulator.h
 * @brief CPU emulator program definitions and declarations
 *
 * This header file contains macro definitions and declarations of external variables
 * and functions for the X-Makina emulator program, including register and memory definitions,
 * bit masking definitions, and functions for branching, arithmetic operations, and
 * various addressing modes.
 *
 * @author Omar Hameed
 * @date July 17, 2023
 */


#include <stdio.h>

#define RED     "\033[1m\033[31m"    
#define YELLOW  "\033[1m\033[33m"      
#define RESET   "\033[0m"

#ifndef EMULATOR_H
#define EMULATOR_H

#define MAXBufSize 256

#define WORD 0
#define BYTE 1
#define ByteLength 7 // 0 to 7 is 8 bits
// Register File Definitions 
// Register's 
#define REG_CONS 2
#define REG 0
#define GP_REG 4
#define BP RegFile[0][4]
#define LR RegFile[0][5]
#define SP RegFile[0][6]
#define PC RegFile[REG][7] 
#define number_reg 8
#define num_consts 8
#define NUM_REG 8
#define LastByte 1


//  Memory Capacity 
#define MEM_SIZE 0x10000
#define MAXBufSize 256
#define WORD_MEM_SIZE 1<<15
#define BYTE_MEM_SIZE 1<<16
#define MAX_FILE_NAME 20
#define DataStart 8

//  Instruction Specific definitions
#define R 0 // Read
#define WR 1 // Write

#define ORs 0
#define BITs 1
#define BICs 2
#define BISs 3

#define MOV 13
#define SWAP 14

//  Byte Shifting Macros
#define MOV_B(x) ((x>>3)&(0xFF))
#define REL_AD_MASK(x)  ((x>>7)&(0x007F))
#define DST(x)   ((x) & (0x07))
#define SRC(x)   ((x>>3)&(0x07))
#define WB(x)   ((x>>6)&(0x01))
#define RC(x)   ((x>>7)&(0x01))


/**********************************************Enums of Instructrions*******************************************/
enum BR { BEQ , BNE , BC, BNC, BN, BGE, BLT, BRA };
enum Arithmetics {ADD, ADDC, SUB, SUBC, DADD, CMP, XOR, AND, OR, BIT , BIC , BIS };
enum Mov{MOVL, MOVLZ, MOVLS, MOVH};
enum IndexedAddressings {LD =0b10, ST = 0b11};
enum OneOprands{SRA, RRC, COMP, SWAPB, SXT };
enum PrePos{Normal, POS_INC, POS_DEC,PRE_INC = 0b101, PRE_DEC = 0b110};


/**********************************************Byte masking definitions *******************************************/

/*
* SEXT_BRA : 111111 0000000000 To set the first 6 bits of a binary value to 1s
* SEXT_BL  : 111000 0000000000 To set the first 3 bits of a binary value to 1s
* SET_BIT  : set bit "pos" in "var". "val" = [0|1]
*/
#define BIT_ARRAY_SIZE 16

#define SEXT_BRA 0xFC00 
#define SEXT_BL 0xE000 
#define SET_HI 0xFF00
#define SET_LOW 0x00FF
#define CLEAR_ALL 0x0000

#define SET_BIT(var, pos, val) ((var) = ((var) & ~(1 << (pos))) | ((val) << (pos))) 
#define Hex_2_Bit(var, pos) (((var) & (1 << pos))>>pos)

extern union Memory {
    unsigned short WordMem[WORD_MEM_SIZE];
    unsigned char ByteMem[BYTE_MEM_SIZE];
}memory_u;


/***************************************** Program Status Word **********************************************/
// Citation: Hughes, L (2023) Using the PSW software (Version 2.0) [Source code].
#define B15(x) (((x)>>15) & 0x01)
#define B7(x) (((x)>>7) & 0x01)
typedef struct 
{
    unsigned short c : 1;
    unsigned short z : 1;
    unsigned short n : 1;
    unsigned short slp : 1;
    unsigned short v : 1;
    unsigned short current : 3; /* Current priority */
    unsigned short faulting : 1; /* 0 - No fault; 1 - Active fault */
    unsigned short reserved : 4;
    unsigned short previous : 3; /* Previous priority */
}psw_bits;
//extern struct psw_bits *psw_ptr;
extern psw_bits psw;
extern psw_bits* pswptr;
extern unsigned carry[2][2][2];
extern unsigned overflow[2][2][2];
/************************************* DADD Implementation *************************************************/

typedef struct {
    unsigned short n_0 : 4;  // LSNibble
    unsigned short n_1 : 4;
    unsigned short n_2 : 4;
    unsigned short n_3 : 4; // MSNibble

}dadd_bits;

extern union bcd_digits {
    unsigned short data_val;
    dadd_bits nibble;
}bcd_digits_u;

extern union word_byte {
    unsigned short word;
    unsigned char byte[2];
}word_byte;

extern dadd_bits dadd;

/* ******************************** Program Flow control ****************************************** */

/*
Activity : CPU clock cycles

Accessing memory : 3
Accessing CPU registers and constants :  0
Fetch cycle   : 1
Decode cycle  : 1
Execute cycle : 1

*/
extern void Controller();
extern void Control();
extern void Fetch();
extern void Decode();
extern void update_psw(unsigned short src, unsigned short dst, unsigned short res, unsigned short wb);
extern void Bus(unsigned short mar, unsigned short* mdr_ptr, int read_write, int word_byte);
int CPU_CLOCK;

/* ******************************** Memory management ****************************************** */
extern unsigned char memory[MEM_SIZE];
extern unsigned short RegFile[REG_CONS][NUM_REG];
extern unsigned short instr_reg;

/* Loader Memory management:
*   testing_input : Input file buffer 
*   origin_address: Address extracted from S9 record
*/ 
extern unsigned char testing_input[MAXBufSize];
unsigned short origin_address; 
extern void ReadFile(FILE* in_file);
int OpenLoadF(int argc, char* argv[]);


/* ******************************** Instruction Implementations ******************************** */
extern void RelativeAddressing();
extern void IndexedAddressing(extern enum IndexedAddressings load_store);

extern void Branching(int branch_type);
extern void BranchLink();
extern void Movs();

extern void OneOprand(extern enum OneOprands);

extern void SignChange(int defined);
extern void Arithmetic(extern enum Arithmetics arithmetics);
extern unsigned short Dadd(unsigned short srcValue, unsigned short dstValue, unsigned char word_byte);
extern unsigned short BcdAdd(unsigned short nibble_x, unsigned short nibble_y, unsigned short* carry);
extern unsigned short Addc(unsigned short src, unsigned short dst, unsigned short temp_carry, char word_byte);

/* ******************************** User Interface ******************************************************/
extern void PrintInstructions();
extern void DisplayRegisters();
extern void printBits(instr_reg);
extern void PrintMemory(unsigned short, int);
extern void PrintRegMem(int);
extern void PrintPswValues();
extern void PrintWholeMemory();
extern void DebugMode();
extern void PrintMemoryRange();
extern void PrintMem(unsigned char* start, unsigned char* end, unsigned short start_address);
extern void AddAssembly();



#endif
