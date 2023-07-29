
/*
 * CACHE MEMORY SYSTEM IMPLEMENTATION
 *
 * This file provides a comprehensive emulation of cache memory operations, offering
 * both write-back and write-through caching strategies. The file contains functions for
 * cache initialization, address location, cache updating, cache printing, and cache line aging.
 * It emulates associative as well as direct mapping cache strategies.
 *
 *
 * Author: Omar
 */




// Macro to choose Write-Back or Write-Through caching strategy, 
// uncomment to select this strategy and comment out the Write-Through macro

#define WRT_BACK // "Achraf Hakimi is the best"
// #define WRT_THRO

// Macro to choose Associative OR DIRCT caching please dont uncomment both
#define Associative 
#define DIRCT

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "emulator.h"

// #define CacheUpdate
// #define CacheDebug

CacheLine cache[CACHE_SIZE];

/*
*  purpose   :   Function to decrement the age of all cache lines, except the one at the provided index
*  parameters : Index to be excluded from decrement
*  return    :  none
*/
void DecrementAllExcept(int index) {
    for (int i = 0; i < CACHE_SIZE; i++) {

        if (cache[i].age > 0 && cache[i].age > cache[index].age) {
            cache[i].age--;
        }

    }
    cache[index].age = MAX_AGE;
}
/*
*  purpose   : Function to initialize the cache
*  parameters: None
*  return    : None
*/
void InitializeCache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i].address = 0x0000;
        cache[i].cache_line.word = 0x0000;
        cache[i].age = 0;
        cache[i].dirty_lo = false;
        cache[i].dirty_hi = false;
        cache[i].valid = false;
    }
}

/*
*  Purpose   : Searches through all cache lines. If the current cache line's address matches the requested address, the index is returned.
*              If the address is not found, the function returns -1.
*
*  Parameters:
*              address - The address to be located in the cache.
*
*  Return    : Returns the index of the found address in cache. If the address is not found, returns -1.
*/
int FindInCache(unsigned short address) {

    for (int i = 0; i < CACHE_SIZE; i++) {
        // Check if address matches and cache line is valid
        if (cache[i].address == address && cache[i].valid) {
            return i;
        }
    }
    return -1;  // Return -1 if address is not found in cache
}

/*
*  purpose    : Function to update the cache with a new address and content
*                   Searches through all cache lines to locate the oldest one, and updates it with the new address and content.
*                   This function follows an associative addressing approach. If Write-Back is enabled, it also checks dirty bits
*                   to decide if a memory write is needed before updating the cache line.
*
*  parameters : address - The new address to be added to the cache
*                   content - The content associated with the new address
*                   word_byte - Flag to determine if the operation is on a word or byte (WORD/BYTE)
*
*  return     :  Returns the index of the oldest cache line that was updated
*/

int UpdateCache(unsigned short address, unsigned short content, unsigned int word_byte) {
#ifdef CacheUpdate
    printf("Cache Update Request: Address = 0x%04X, Content = 0x%02X\n", address, content);
#endif
    int oldest_index = 0;


#ifdef Associative 
    for (int i = 1; i < CACHE_SIZE; i++) {
        
        if (cache[i].age <= cache[oldest_index].age) {

            oldest_index = i;
        }
    }
#endif
#ifdef  DIRCT
    oldest_index = address % CACHE_SIZE;
#endif 

#ifdef CacheUpdate
    printf(RED "Cache Evicted: Address = 0x%04X, Content = 0x%02X\n" RESET, cache[oldest_index].address, cache[oldest_index].contents);
#endif

#ifdef WRT_BACK

     // If either the high byte or low byte of the dirty bit is set then we must write to memory to avoid brain damage 
    if ( (cache[oldest_index].dirty_lo || cache[oldest_index].dirty_hi) && cache[oldest_index].valid) {
        if (word_byte == WORD) {
            Bus(cache[oldest_index].address, &cache[oldest_index].cache_line.word, WR, WORD);
            cache[oldest_index].dirty_lo = false;
            cache[oldest_index].dirty_hi = false;

        }
        else{

           
            if (address % 2 == 0) {
                // If address is even, load high byte
                Bus(address, &cache[oldest_index].cache_line.byte[1], WR, BYTE);
                cache[oldest_index].dirty_hi = true;

            }
            else {
                // If address is odd, load low byte
                Bus(address, &cache[oldest_index].cache_line.byte[0], WR, BYTE);
                cache[oldest_index].dirty_lo = true;

            }
            cache[oldest_index].dirty_lo = false;
            cache[oldest_index].dirty_hi = false;
        }
    }


#endif


        cache[oldest_index].address = address;
        cache[oldest_index].valid = true; 
    if (word_byte == WORD) {
        cache[oldest_index].cache_line.word = content;



    }
    else {
        if (address % 2 == 0) {
            // If address is even, load high byte
            cache[oldest_index].cache_line.byte[1] = content >> 8;
        }
        else {
            // If address is odd, load low byte
            cache[oldest_index].cache_line.byte[0] = content & 0xFF; 
        }
    }
#ifdef CacheUpdate
    printf("Cache Updated Completed: Address = 0x%04X, Content = 0x%02X\n", cache[oldest_index].address, cache[oldest_index].contents);
#endif

    DecrementAllExcept(oldest_index);

    return oldest_index;
}


/*
*  purpose   : Function to print the current state of the cache
*              Loop over all cache lines, Print the cache line, address, and age of each cache line
*  parameters: None
*  return    : None
*/

void PrintCache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        printf(" |CACHE LINE %2d", i);
        printf(" | Address: 0x%04X ", cache[i].address);
        printf(" | Contents: 0x%4X ", cache[i].cache_line.word);
        printf(" | Age: %02d ", cache[i].age);
        printf(" | Word/Byte: %02d ", cache[i].word_byte);
        printf(" | DL: %s |", cache[i].dirty_lo ? "1" : "0");
        printf(" | DH: %s |\n", cache[i].dirty_hi ? "1" : "0");
    }
}


/*
*  purpose   : Function to manage the cache read and write operations.
*              This function handles the logic of read/write operations in cache, including miss/hit situations,
*              with different behaviors for Write-through (1) and Write-back (0) methods.
*              In Write-through, it simultaneously writes into cache and main memory.
*              In Write-back, it delays the main memory write operation until the CACHE line is evicted.
*              If a cache miss occurs (when the cache does not contain the requested data),
*              the function calls the appropriate UpdateCache().
*              In case of a cache hit (when the requested data is found in cache),
*              it directly manipulates the cache contents (if its a write) and calls DecrementAllExcept() function in both cases
*  
*  parameters: address - The memory address to be read/written
*              content - Pointer to the content to be written or where the read content should be stored
*              read_write - Symbol to determine if the operation is a read (R) or write (WR)
*              word_byte - Symbol to determine if the operation is on a word or byte (WORD/BYTE)
*
*  return    : None
*/



void Cache(unsigned short address, unsigned short* content,
    unsigned char read_write, unsigned char word_byte) {


    int found_index;
    found_index = FindInCache(address);

    if (read_write == R) {
        
        if (found_index == -1) {
            // Read  A Word from bus 
            if (word_byte == WORD ) Bus(address, content, R, WORD);
            else Bus(address, content, R, WORD);
            UpdateCache(address, *content, word_byte);
        }
        
        else DecrementAllExcept(found_index);

        found_index = FindInCache(address);
        if (found_index == -1) {
            printf(RED "\nError oopsi: CACHE LINE INCORRECTLY SET \n"RESET);
            return 0;
        }
        cache[found_index].valid = true;
        DecrementAllExcept(found_index);
        // Trust me Im an Engineer 
        *content = cache[found_index].cache_line.word;

        printf("CACHE READ  %s  AT ADDRESS %04X FILLED WITH %04X \n", word_byte ? "BYTE" : "WORD", cache[found_index].address, cache[found_index].cache_line.word);
    }
    else {
    

#ifdef WRT_THRO

        if (found_index == -1) { // MISS ME
            Bus(address, content, WR, (word_byte == WORD) ? WORD : BYTE);
            UpdateCache(address, *content, word_byte);
        }
       
        else { // HIT me 
            if (word_byte == WORD) { 
                Bus(address, content, WR, WORD);
                cache[found_index].cache_line.word = *content;
            }

            else {

                if (address % 2 == 0) {
                    // If address is even, load high byte
                    Bus(address, &cache[found_index].cache_line.byte[HI], WR, BYTE);
                    cache[found_index].cache_line.byte[HI] = *content;
                }
                else {
                    // If address is odd, load low byte
                    Bus(address, &cache[found_index].cache_line.byte[HI], WR, BYTE);
                    cache[found_index].cache_line.byte[HI] = *content;
                }
            }
        }
        found_index = FindInCache(address);
        if (found_index == -1) {
            printf(RED "\nError: CACHE LINE INCORRECTLY Read at address %04x filled with %04x \n"RESET, address, *content);
            return 0;
        }
        DecrementAllExcept(found_index);
#endif


#ifdef WRT_BACK
        if (found_index == -1) { // MISS ME
            UpdateCache(address, *content, word_byte);
            found_index = FindInCache(address);
            if (word_byte == WORD) { 
                cache[found_index].dirty_hi = true;
                cache[found_index].dirty_lo = true;
            }
            else {
                if (address % 2 == 0) {
                    // If address is even, load high byte
                    cache[found_index].dirty_hi = true;
                }
                else {
                    // If address is odd, load low byte
                    cache[found_index].dirty_lo = true;
                }
            }
        }

        else { // HIT me 
            DecrementAllExcept(found_index);
            found_index = FindInCache(address);

            if (word_byte == WORD) { // if we need to write a word
                cache[found_index].cache_line.word = *content;
                found_index = FindInCache(address);
                cache[found_index].dirty_hi = true;
                cache[found_index].dirty_lo = true;

            }
            else {
                if (address % 2 == 0) {
                    // If address is even, load high byte
                    cache[found_index].cache_line.byte[1] = *content;
                    cache[found_index].dirty_hi = true;
                }
                else {
                    // If address is odd, load low byte
                    cache[found_index].cache_line.byte[0] = *content;
                    cache[found_index].dirty_lo = true;
                }
            }
            

        }
        if (found_index == -1) {
            printf(RED "\nError big oppssie: WE WROTE TO CACHE LINE  -1 !! trying to Write address %04X with content %04X\n"RESET, address, *content);
            return 0;
        }
        
#endif

    }
}




