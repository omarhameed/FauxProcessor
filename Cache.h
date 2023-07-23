/*
* This is the header file for the cache memory system.
* It defines the constants for memory size, cache size and maximum age.
* It also defines the CacheLine struct and declares the functions used in the cache memory system.
*/
#include <stdbool.h>

#ifndef CACHE_H  
#define CACHE_H






// Defining constants
#define MEM_SIZE 0x10000  // Size of the memory
#define CACHE_SIZE 32  // Size of the cache
#define MAX_AGE 31  // Maximum age for a cache line

// CacheLine struct definition
typedef struct {
    unsigned short address;  // Address of the cache line 0x000 to 0xFFFF
    unsigned char contents;
    unsigned __int8 age;    // Age of the cache line (__int8: 0 to 255)
    unsigned char word_byte;
    bool dirty;
} CacheLine;

extern void InitializeCache();
extern int FindInCache(unsigned short address);
extern int UpdateCache(unsigned short address, unsigned short content);
extern void PrintCache();
extern void DecrementAllExcept(int index);
extern void Cache(unsigned short address, unsigned short* content,
                 unsigned char read_write, unsigned char word_byte);

#endif
