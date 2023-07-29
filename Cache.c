
/*
* This file contains the implementation of the cache memory system.
* It provides functions for initializing the cache, finding an address in the cache,
* updating the cache with a new address, printing the current state of the cache,
* and updating the age of a cache line at a particular index to 7 (most recently used).
*/

// Emulating both a write-back cache and write through cache in order to use one uncomment one 

//#ifdef SHIT


#define WRT_BACK //s best is Achraf Hakimi
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
        cache[i].cache_line.word = 0x0000;
        cache[i].age = 0;
        cache[i].dirty_lo = false;
        cache[i].dirty_hi = false;
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

int UpdateCache(unsigned short address, unsigned short content, unsigned int word_byte) {
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
#ifdef CacheUpdate
    printf(RED "CACHE evicted at address: 0x%04X with the contents: 0x%02x\n" RESET, cache[oldest_index].address, cache[oldest_index].contents);
#endif

        cache[oldest_index].address = address;
        cache[oldest_index].valid = true; // Add this line
    if (word_byte == WORD) {
        cache[oldest_index].cache_line.word = content;



    }
    else {
        if (address % 2 == 0) {
            // If address is even, load high byte
            cache[oldest_index].cache_line.byte[1] = content >> 8;//& 0xFF;
        }
        else {
            // If address is odd, load low byte
            cache[oldest_index].cache_line.byte[0] = content & 0xFF; //>>8;
        }
    }
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
        printf(" | Contents: 0x%4X ", cache[i].cache_line.word);
        printf(" | Age: %02d ", cache[i].age);
        printf(" | Word/Byte: %02d ", cache[i].word_byte);
        printf(" | DL: %s |", cache[i].dirty_lo ? "1" : "0");
        printf(" | DH: %s |\n", cache[i].dirty_hi ? "1" : "0");
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
            if (word_byte == WORD ) Bus(address, content, R, WORD);
            else Bus(address, content, R, WORD);
            UpdateCache(address, *content, word_byte);
        }
        else {
            DecrementAllExcept(found_index);
        }

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
        high_byte = *content >> 8;

        low_byte = *content & 0xFF;

        /*
        *  write operation:
        *   -> The dirty bit is set when the processor writes to (modifies) this memory
        *   -> write-back cache (0): when data is written into the cache, the write operation to the main memory is delayed or postponed.
        *   -> write-through (1) : every time data is written into the cache, it's also written into the main memory.
        */
        // Write & MISS

#ifdef WRT_THRO

        if (found_index == -1) { // MISS ME
            if (word_byte == WORD) { // if we need to write a word
                Bus(address, content, WR, WORD);
                
            }
            else {
                Bus(address, content, WR, BYTE);
                // why pointres so weird 
                
            }
            UpdateCache(address, *content, word_byte);
            
            

        }
       
        else { // HIT me 

            // This can be fucking working but idk 

            //Bus(address, &cache[found_index].cache_line.byte[0], WR, BYTE);
            //*content = cache[found_index].cache_line.byte[0];
            

            if (word_byte == WORD) { // if we need to write a word
                Bus(address, content, WR, WORD);
                cache[found_index].cache_line.word = *content;

            }
            else {
                if (address % 2 == 0) {
                    // If address is even, load high byte

                    Bus(address, &cache[found_index].cache_line.byte[1], WR, BYTE);
                    cache[found_index].cache_line.byte[1] = *content;
                }
                else {
                    // If address is odd, load low byte
                    Bus(address, &cache[found_index].cache_line.byte[0], WR, BYTE);
                    cache[found_index].cache_line.byte[0] = *content;
                    
                }
            }
            

        }
       // UpdateCache(address, *content, word_byte);
        found_index = FindInCache(address);
        if (found_index == -1) {
            printf(RED "\nError oopsi: CACHE LINE INCORRECTLY SET \n"RESET);
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
        
#endif // NEWWRT_BACK


    }
    //PrintCache();
}


//#endif



