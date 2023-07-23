/*
* This file contains the implementation of the cache memory system.
* It provides functions for initializing the cache, finding an address in the cache,
* updating the cache with a new address, printing the current state of the cache,
* and updating the age of a cache line at a particular index to 7 (most recently used).
*/

// Emulating both a write-back cache and write through cache in order to use one uncomment one 

//#define WRT_BACK
#define WRT_THRO


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "emulator.h"

#define CacheUpdate
#define CacheDebug

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
        cache[i].contents = 0x00;
        cache[i].age = 0;
        cache[i].word_byte = BYTE;
        cache[i].dirty = false;
    }
}

/*
*  purpose   : Loop over all cache lines, If the current cache line's address matches the requested address, return index
*  parameters: Address to find in the cache
*  return    : Index of the found address in cache, if not found return -1
*/
int FindInCache(unsigned short address) {

    int i = 0;
    int oldest_index = 0;
    do {
        if (cache[i].address == address) {
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
    for (int i = 1; i < CACHE_SIZE; i++) {
        if (cache[i].age < cache[oldest_index].age) {
            oldest_index = i;
        }
    }
    // if dirty bit set then 
    if (cache[oldest_index].dirty) {
        Bus(address, &content, WR, BYTE);
        cache[oldest_index].dirty = false;
    }
#ifdef CacheUpdate
    printf(RED "CACHE evicted at address: 0x%04X with the contents: 0x%02x\n" RESET, cache[oldest_index].address, cache[oldest_index].contents);
#endif
    cache[oldest_index].address = address;
    cache[oldest_index].contents = content;
#ifdef CacheUpdate
    printf("CACHE Updated at address: 0x%04X with the contents: 0x%02x\n", cache[oldest_index].address, cache[oldest_index].contents);
#endif

    // DecrementAllExcept(oldest_index);

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
        printf(" | Contents: 0x%02X ", cache[i].contents);
        printf(" | Age: %02d ", cache[i].age);
        printf(" | Word/Byte: %02d ", cache[i].word_byte);
        printf(" | Dirty: %s |\n", cache[i].dirty ? "1" : "0");
    }
}
void Cache(unsigned short address, unsigned short* content,
    unsigned char read_write, unsigned char word_byte) {


    int oldest_index;
    int oldest_index_w;

    int new_index;
    int found_index;
    int found_index_w;
    unsigned short high_byte_adr = address + 1;

    found_index = FindInCache(address);
    if (word_byte == WORD) found_index_w = FindInCache(high_byte_adr);

    // If its a Read 
    if (read_write == 0) {
        if (found_index == -1) {

            // if we need to read a word
            if (word_byte == 0) {
                found_index_w = FindInCache(address + 1);
                // Call bus twice and add word to two cache lines.
                unsigned short high_byte = *content >> 8;
                unsigned short low_byte = *content & 0x00FF;

                unsigned short high_byte_adr = address + 1;

                Bus(address, &low_byte, read_write, BYTE);

                oldest_index = UpdateCache(address, low_byte);
                DecrementAllExcept(oldest_index);

                Bus(high_byte_adr, &high_byte, read_write, BYTE);
                oldest_index = UpdateCache(high_byte_adr, high_byte);
                DecrementAllExcept(oldest_index);

#ifdef CACHE_DEBUG
                printf("WORD READ TO CACHE\n");
                PrintCache();
#endif

            }
            else { // if we need to read a byte
                Bus(address, &content, read_write, word_byte);
                oldest_index = UpdateCache(address, *content);
                DecrementAllExcept(oldest_index);
#ifdef CACHE_DEBUG
                printf("Byte READ TO CACHE\n");
                PrintCache();
#endif
            }
        }
        else {
            if (word_byte == WORD) {
                DecrementAllExcept(found_index);
                DecrementAllExcept(found_index_w);
                //unsigned short word = (high << 8) | low;
                *content = (cache[found_index_w].contents << 8) | (cache[found_index].contents);


            }
            else {
                (unsigned char) *content = cache[found_index].contents;
                DecrementAllExcept(found_index);
            }
            printf("CACHE CONTENT %04X", *content);
        }
    }
    else {
        /*
        *  write operation:
        *   -> The dirty bit is set when the processor writes to (modifies) this memory
        *   -> write-back cache (0): when data is written into the cache, the write operation to the main memory is delayed or postponed.
        *   -> write-through (1) : every time data is written into the cache, it's also written into the main memory.
        */
        // Write & MISS

        

        // If we're using write-through caching
#ifdef WRT_THRO
            if (word_byte == 0) { // if we need to write a word

                unsigned char high_byte = *content >> 8;
                unsigned char  low_byte = *content & 0xFF;

                unsigned short high_byte_adr = address + 1;

                oldest_index = UpdateCache(address, low_byte);
                DecrementAllExcept(oldest_index);
                oldest_index_w = UpdateCache(high_byte_adr, high_byte);
                DecrementAllExcept(oldest_index_w);

                
            }
            else { // if we need to write a byte

                oldest_index = UpdateCache(address, *content);
                DecrementAllExcept(oldest_index);

            }
            // Write value immediately to PM
                Bus(address, content, read_write, word_byte);
#endif
        
#ifdef WRT_BACK

            // For write-back, mark the cache entry as dirty but don't write to memory
            if (word_byte == 0) { // if we need to write a word

                unsigned char high_byte = *content >> 8;
                unsigned char  low_byte = *content & 0xFF;

                unsigned short high_byte_adr = address + 1;

                //cache[found_index].address = address;
                //cache[found_index_w].address = high_byte_adr;
                //cache[found_index].contents = low_byte;
                //cache[found_index_w].contents = high_byte;


                oldest_index = UpdateCache(high_byte_adr, high_byte);
                cache[oldest_index].dirty = true;
                DecrementAllExcept(oldest_index);

                oldest_index_w = UpdateCache(address, low_byte);
                cache[oldest_index_w].dirty = true;
                DecrementAllExcept(oldest_index_w);

                printf("THE ADDIE = %04X | THE CONTENT lsb = %02X | THE CONTENT msb = %02X\n", address, low_byte, high_byte);

            }
            else { // if we need to write a byte
                oldest_index = UpdateCache(address, *content);
                cache[oldest_index].dirty = true;
                DecrementAllExcept(oldest_index);
            }
#endif
        
    }
}

