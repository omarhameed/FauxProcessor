
#include <stdio.h>
#include "emulator.h"
//#define PSW_DEBUG
unsigned carry[2][2][2] = { 0, 0, 1, 0, 1, 0, 1, 1 };
unsigned overflow[2][2][2] = { 0, 1, 0, 0, 0, 0, 1, 0 };
psw_bits psw = { 0 };
psw_bits* pswptr = &psw;

struct psw_bits* psw_ptr; /* PSW */

/**
* Reference: Hughes, L (2023) Using the PSW software (Version 2.0) [Source code].

 * Purpose: Updates the PSW based on the results of arithmetic operations.
 *
 * @param src: The source of operation.
 * @param dst: The destination of operation.
 * @param res: The result of  operation.
 * @param wb: Indicator for the size of the data operation. 
 *
 * 
 */
void update_psw (unsigned short src, unsigned short dst, unsigned short res,
    unsigned short wb)
{
    /*
     - Update the PSW bits (V, N, Z, C)
     - Using src, dst, and res values and whether word or byte
     - ADD, ADDC, SUB, and SUBC
    */
    unsigned short mss, msd, msr; /* Most significant src, dst, and res bits */

    if (wb == 0)
    {
        mss = B15(src);
        msd = B15(dst);
        msr = B15(res);
    }
    else /* Byte */
    {
        mss = B7(src);
        msd = B7(dst);
        msr = B7(res);
        res &= 0x00FF;	/* Mask high byte for 'z' check */
    }

    /* Carry */
    pswptr->c = carry[mss][msd][msr];
    /* Zero */
    pswptr->z = (res == 0);
    /* Negative */
    pswptr->n = (msr == 1);
    /* oVerflow */
    pswptr->v = overflow[mss][msd][msr];
#ifdef PSW_DEBUG

    printf("mss: %d msd: %d msr: %d\n", mss, msd, msr);
#endif // PSW_DEBUG



}

/**
 * Purpose: Updates the PSW based on the result of a logic operation.
 * Only the Negative (N) and Zero (Z) flags of the PSW are updated.
 * @param result: The result of the operation.
 * @param word_byte: Indicator for the size.
 */

void update_psw_2(unsigned short result, unsigned short word_byte) 
{
    /*
     - Update the PSW bits (N & Z) only 
     - OR, XOR and AND. 
    */
    unsigned short msr;
    if (word_byte == 0)msr = B15(result);
    else {
        msr = B7(result);
        result &= 0x00FF;	/* Mask high byte for 'z' check */
    }
    /* Zero */
    pswptr->z = (result == 0);
    /* Negative */
    pswptr->n = (msr == 1);

}