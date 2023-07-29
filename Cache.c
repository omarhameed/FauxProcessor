
/*
* This file contains the implementation of the cache memory system.
* It provides functions for initializing the cache, finding an address in the cache,
* updating the cache with a new address, printing the current state of the cache,
* and updating the age of a cache line at a particular index to 7 (most recently used).
*/

// Emulating both a write-back cache and write through cache in order to use one uncomment one 

//#ifdef SHIT


//#define WRT_BACK
#define WRT_THRO
#define Associative


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
*              Loop over all cache lines, initialize the address to 0 and age to 0
*  parameters: None
*  return    : None
*/
void InitializeCache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i].address = 0x0000;
        cache[i].cache_line.word = 0x0000;
        cache[i].age = 0;
        cache[i].dirty = false;
        cache[i].valid = false;
    }
}

/*
*  purpose   : Loop over all cache lines, If the current cache line's address matches the requested address, return index
*  parameters: Address to find in the cache
*  return    : Index of the found address in cache, if not found return -1
*/
int FindInCache(unsigned short address) {

    int i = 0;
    do {
        // Check if address is populated 
        if (cache[i].address == address && cache[i].valid) {
            return i;
        }

        i++;
    } while (i < CACHE_SIZE);
    return -1;
}

/*
*  purpose   : Function to update the cache with a new address & content
*              Loop over all cache lines to find the oldest, then update the oldest cache line to the new address
*  parameters: Address to add to cache
*  return    : oldest_index of the cache
*/
// Associative Addressing only

int UpdateCache(unsigned short address, unsigned short content) {
#ifdef CacheUpdate
    printf("CACHE Update requst at address: 0x%04X with the contents: 0x%02x\n", address, content);
#endif
    int oldest_index = 0;


#ifdef Associative 
    // printf(RED"ADDRESS %04X EVICTED THAT THIS SHIT %02X\n"RESET, cache[oldest_index].address, cache[oldest_index].contents);
    for (int i = 1; i < CACHE_SIZE; i++) {
        if (cache[i].age <= cache[oldest_index].age) {
            oldest_index = i;
        }
    }
    // printf(YELLOW"NEW ADDRESS %04X EVICTED THAT THIS SHIT %02X\n"RESET, address, content);
#else
    oldest_index = address % CACHE_SIZE;
#endif 
#ifdef WRT_BACK

    // if dirty bit set then 
    // && cache[oldest_index].valid
    if (cache[oldest_index].dirty && cache[oldest_index].valid) {
        Bus(cache[oldest_index].address, &cache[oldest_index].cache_line.word, WR, WORD);
        cache[oldest_index].dirty = false;
    }

#endif
#ifdef CacheUpdate
    printf(RED "CACHE evicted at address: 0x%04X with the contents: 0x%02x\n" RESET, cache[oldest_index].address, cache[oldest_index].contents);
#endif

    cache[oldest_index].address = address;
    cache[oldest_index].cache_line.word = content;
    //else cache[oldest_index].cache_line.byte[low_high]= content;
    cache[oldest_index].valid = true; // Add this line
    //if (read_write == WR)cache[oldest_index].dirty = true;
#ifdef CacheUpdate
    printf("CACHE Updated at address: 0x%04X with the contents: 0x%02x\n", cache[oldest_index].address, cache[oldest_index].contents);
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
        printf(" | Contents: 0x%02X ", cache[i].cache_line.word);
        printf(" | Age: %02d ", cache[i].age);
        printf(" | Word/Byte: %02d ", cache[i].word_byte);
        printf(" | Dirty: %s |\n", cache[i].dirty ? "1" : "0");
    }
}
void Cache(unsigned short address, unsigned short* content,
    unsigned char read_write, unsigned char word_byte) {


    int oldest_index;
    int found_index;
    unsigned char high_byte, low_byte;
    unsigned short cache_content;

    found_index = FindInCache(address);

    if (read_write == R) {
        if (found_index == -1) {
            // Read  A Word from bus 
            Bus(address, &cache[found_index].cache_line.word, R, WORD);

            
            

        }
        else {
            DecrementAllExcept(found_index);
        }
        found_index = FindInCache(address);

        // Store in Cache Line
        if (word_byte == WORD) {
            *content = cache[found_index].cache_line.word;
        }
        else {
            // if address even load high byte else load low byte
            //UpdateCache(address, (unsigned short)cache[found_index].cache_line.byte[0]);

            if (address % 2 == 0) {
                // If address is even, load high byte
                *content = cache[found_index].cache_line.byte[1];
            }
            else {
                // If address is odd, load low byte
                *content = cache[found_index].cache_line.byte[0];
            }

        }
        cache[found_index].valid = true;
        
        
    }
    else {
        high_byte = *content >> 8;

        low_byte = *content & 0xFF;

        /*
        *  write operation:
        *   -> The dirty bit is set when the processor writes to (modifies) this memory
        *   -> write-back cache (0): when data is written into the cache, the write operation to the main memory is delayed or postponed.
        *   -> write-through (1) : every time data is written into the cache, it's also written into the main memory.
        */
        // Write & MISS



        // If we're using write-through caching
#ifdef WRT_THRO

        if (word_byte == WORD) { // if we need to write a word
            cache[found_index].cache_line.word = *content;

            //Bus(address, content, WR, WORD);


            found_index = FindInCache(address);
            Bus(address, &cache[found_index].cache_line.word, WR, WORD);
            if (found_index == -1) {
                printf("\nError: CACHE LINE INCORRECTLY SET 3\n");
                return 0;
            }

        }
        else { // if we need to write a byte

            // This can be fucking working but idk 
            
            //Bus(address, &cache[found_index].cache_line.byte[0], WR, BYTE);
            //*content = cache[found_index].cache_line.byte[0];
            

            // if not this should fucking work 
            if (address % 2 == 0) {
                // If address is even, load high byte
                cache[found_index].cache_line.byte[1] = *content ;
                Bus(address, &cache[found_index].cache_line.byte[1], WR, BYTE);
                UpdateCache(address, *content);
            }
            else {
                // If address is odd, load low byte
                Bus(address, &cache[found_index].cache_line.byte[0], WR, BYTE);
                // cache[found_index].cache_line.byte[0] = *content ;
                UpdateCache(address, (unsigned short)*content);
            }

        }


#endif

#ifdef WRT_BACK

        // For write-back, mark the cache entry as dirty but don't write to memory
        if (word_byte == WORD) { // if we need to write a word

            oldest_index = UpdateCache(address, *content);
            //DecrementAllExcept(oldest_index);
            found_index = FindInCache(address);
            cache[found_index].dirty = true;
            cache[found_index].valid = true;

            /*oldest_index_w = UpdateCache(high_byte_adr, high_byte);
            //DecrementAllExcept(oldest_index_w);
            found_index_w = FindInCache(high_byte_adr);
            cache[found_index_w].dirty = true;
            cache[found_index_w].valid = true;*/


            printf("THE ADDIE = %04X | THE CONTENT lsb = %02X | THE CONTENT msb = %02X\n", address, low_byte, high_byte);

        }
        else { // if we need to write a byte

            oldest_index = UpdateCache(address, (unsigned char)*content);
            found_index = FindInCache(address);
            cache[found_index].dirty = true;
            cache[found_index].valid = true;
            //DecrementAllExcept(oldest_index);
        }

        // Bus(address, content, read_write, word_byte);
#endif

    }
    //PrintCache();
}


//#endif



