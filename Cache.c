
/*
* This file contains the implementation of the cache memory system.
* It provides functions for initializing the cache, finding an address in the cache,
* updating the cache with a new address, printing the current state of the cache,
* and updating the age of a cache line at a particular index to 7 (most recently used).
*/

// Emulating both a write-back cache and write through cache in order to use one uncomment one 

//#ifdef SHIT


#define WRT_BACK
//#define WRT_THRO
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
        cache[i].contents = 0x00;
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

int UpdateCache(unsigned short address, unsigned char content) {
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
        Bus(cache[oldest_index].address, &(unsigned char)cache[oldest_index].contents, WR, BYTE);

        printf(RED"ADDRESS %04X EVICTED CONTENT %02X "RESET, cache[oldest_index].address, cache[oldest_index].contents);
        cache[oldest_index].dirty = false;
    }

#endif




#ifdef CacheUpdate
    printf(RED "CACHE evicted at address: 0x%04X with the contents: 0x%02x\n" RESET, cache[oldest_index].address, cache[oldest_index].contents);
#endif
    cache[oldest_index].address = address;
    cache[oldest_index].contents = content;
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
    unsigned char high_byte_addr = address >> 8;
    unsigned char low_byte_addr= address & 0xFF;

    int new_index;
    int found_index;
    int found_index_w = 0;
    //mar>>1
    address = address ;
    unsigned short high_byte_adr = address +1;

    unsigned char high_byte = 0;
    unsigned char  low_byte = 0;


    found_index = FindInCache(address);
    

    // If its a Read 
    if (read_write == R) {
        if (found_index == -1) {

            // if we need to read a word
            if (word_byte == WORD) {
                unsigned short word = 0;
                // Read  A Word from bus 
                Bus(address, &word, R, WORD);

                // Split up word to a high and low byte
                high_byte = word >> 8;
                low_byte = word & 0xFF;


                UpdateCache(address, low_byte);
                //DecrementAllExcept(oldest_index);
                found_index_w = FindInCache(high_byte_adr);

                if (found_index_w == -1) {
                    UpdateCache(high_byte_adr, high_byte);
                    found_index_w = FindInCache(high_byte_adr);
                    // word = (high << 8) | low;
                    //DecrementAllExcept(oldest_index_w);
                   // *content = (cache[oldest_index_w].contents << 8) | (cache[oldest_index].contents);
                }

                found_index = FindInCache(address);
                // Maybe just read word into content if it fails 
                if (found_index == -1 || found_index_w == -1) {
                    printf(RED"\nError: CACHE LINE INCORRECTLY SET\n"RESET);
                    return 0;
                }
                else *content = (cache[found_index_w].contents << 8) | (cache[found_index].contents);
                cache[found_index_w].valid = true;
                cache[found_index].valid = true;


#ifdef CACHE_DEBUG
                printf("WORD READ TO CACHE\n");
                PrintCache();
#endif

            }
            else { // if we need to read a byte
                unsigned short temp_holder = 0;
                Bus(address, &temp_holder, R, BYTE);
                *content = (unsigned char)temp_holder;
                oldest_index = UpdateCache(address, *content);
                //DecrementAllExcept(oldest_index);
                found_index = FindInCache(address);
                if (found_index == -1) {
                    printf(RED"\nError: CACHE LINE INCORRECTLY SET 2\n"RESET);
                    return 0;

                }
                cache[found_index_w].valid = true;
                cache[found_index].valid = true;
#ifdef CACHE_DEBUG
                printf("Byte READ TO CACHE\n");
                PrintCache();
#endif

            }
        }
        else {
            if (word_byte == WORD) {
                DecrementAllExcept(found_index);
               // high_byte = word >> 8;
                found_index_w = FindInCache(high_byte_adr);

                if (found_index_w == -1) {
                    // Read  A BYTE From bus 
                        Bus(address, &high_byte, R, BYTE);
                    
                        oldest_index_w = UpdateCache(high_byte_adr, high_byte);
                        //unsigned short word = (high << 8) | low;
                        //ecrementAllExcept(oldest_index_w);
                        found_index_w = FindInCache(high_byte_adr);
                        found_index = FindInCache(address);
                        *content = (cache[found_index_w].contents << 8) | (cache[found_index].contents);
                        cache[found_index_w].valid = true;

                    
                }
                //unsigned short word = (high << 8) | low;
                else {
                    DecrementAllExcept(found_index_w);

                    found_index_w = FindInCache(high_byte_adr);
                    found_index = FindInCache(address);
                    *content = (cache[found_index_w].contents << 8) | (cache[found_index].contents);
 
                }


            }
            else {

                    DecrementAllExcept(found_index);
                
            }
            // printf("CACHE CONTENT %04X\n", *content);
        }
    }
    else {
        high_byte = *content >> 8;

        low_byte = *content & 0xFF;

        //low_byte = *content >> 8;
        //high_byte = *content & 0xFF;
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
            if (address == 0x0080)printf("SHIT low %2X\n", low_byte);
            if (address == 0x0080)printf("SHIT high %2X\n", high_byte);

            Bus(address, content, WR, WORD);

            oldest_index = UpdateCache(address, low_byte);
            //DecrementAllExcept(oldest_index);

            oldest_index_w = UpdateCache(high_byte_adr, high_byte);
            //DecrementAllExcept(oldest_index_w);

            found_index = FindInCache(address);
            found_index_w = FindInCache(high_byte_adr);
            if (found_index == -1 || found_index_w == -1) {
                printf(RED"\nError: CACHE LINE INCORRECTLY SET 3\n"RESET);
                return 0;
            }
            
            // Bus(address + 1, &high_byte, WR, BYTE);
            // Bus(address, &low_byte, WR, BYTE);

        }
        else { // if we need to write a byte
            Bus(address, content, WR, BYTE);
            low_byte = *content & 0xFF;

            oldest_index = UpdateCache(address, low_byte);
            //DecrementAllExcept(oldest_index);
        }
        // Write value immediately to PM
            //Bus(address, content, read_write, word_byte);

#endif

#ifdef WRT_BACK

            // For write-back, mark the cache entry as dirty but don't write to memory
        if (word_byte == WORD) { // if we need to write a word




            //cache[found_index].address = address;
            //cache[found_index_w].address = high_byte_adr;
            //cache[found_index].contents = low_byte;
            //cache[found_index_w].contents = high_byte;

            if (address == 0x0080)printf("SHIT low %2X\n", low_byte);
            if (address == 0x0080)printf("SHIT high %2X\n", high_byte);

            oldest_index = UpdateCache(address, low_byte);
            //DecrementAllExcept(oldest_index);
            found_index = FindInCache(address);
            cache[found_index].dirty = true;
            cache[found_index].valid = true;

            oldest_index_w = UpdateCache(high_byte_adr, high_byte);
            //DecrementAllExcept(oldest_index_w);
            found_index_w = FindInCache(high_byte_adr);
            cache[found_index_w].dirty = true;
            cache[found_index_w].valid = true;




            //Bus(address, content, WR, word_byte);

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
}


//#endif



