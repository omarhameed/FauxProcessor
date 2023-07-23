/**
 * @file Loader.c
 * @brief XME File Loader for ECED3403 Assignment 1 Emulator
 *
 * This module contains functionality to read S-Record formatted files,
 * extracting and decoding necessary information such as record type,
 * data length, address and data. This information is used to populate
 * memory and setup the initial program counter (PC) for the emulator.
 *
 * Note: Only S0, S1, and S9 records are supported.
 *
 * This is part of the coursework for ECED3403 - Computer Architecture, Assignment 1.
 *
 * @author Omar Hameed
 * @student_id B00764655
 * @date Last updated on June 16, 2023
 *
 * 
 */

#include <stdio.h>
#include "emulator.h"

unsigned char testing_input[MAXBufSize]; // Define testing_input variable

/*
 *   Purpose:
 *          Opens a .XME file and loads it into memory. If no file is provided via command-line
 *          arguments, it prompts the user for a file name. If the file fails to open, it allows
 *          for retry up to three times. It calls ReadFile() function for reading the file content.
 *  Parameters:
 *          int argc: number of strings pointed to by argv.
 *          char* argv[]: command-line arguments.
 */
int OpenLoadF(int argc, char* argv[]) {
    char* file_name;
    char buffer[MAX_FILE_NAME];
    FILE* in_file = NULL;
    int max_trials = 3;
    int trials = 0;

    while (trials < max_trials) {
        if (argc >= 2) {
            file_name = argv[1];
        }
        else {
            printf("Enter the name of the .XME file to read: ");
            scanf("%s", buffer);
            file_name = buffer;
        }
        size_t name_length = strlen(file_name);

        // Check if the file has the correct extension
        if (name_length < 5 || strcmp(&file_name[name_length - 4], ".xme") != 0) {
            printf("Error: Invalid file extension. The file should have a .xme extension, try again \n");

            trials++;
            continue;  // Continue to the next iteration of the loop
        }
        in_file = fopen(file_name, "r");
        if (in_file != NULL) {
            printf("XME File read successfully\n");
            break;  // Exit the loop since the file was successfully opened
        }
        else {
            printf("Failed to open the file, try again\n");
            trials++;
        }
    }

    if (trials == max_trials) {
        printf("Maximum number of trials reached.\n Exiting the program.\n");
        return 1;  // Return an error code
    }
    ReadFile(in_file);


    return 0;  // Return success
}
/*
 *   Purpose:
 *              Read an S-record file.
 *              It reads the file line by line and depending on the record type,
 *              it performs different operations like writing to memory or setting the PC.
 *   Parameters:
 *              FILE* in_file: Pointer to the file object of the file to be read.
 */
void ReadFile(FILE* in_file) {
    unsigned int number_of_s1s = 0;
    
    while (fgets(testing_input, MAXBufSize, in_file) != NULL) {
        unsigned char CheckSum = 0;
        unsigned int address_lo;
        unsigned int address_hi;
        int length;
        if (testing_input[0] != 'S') {
            printf("ERROR: %s IS Not a valid S-record \n", testing_input);
        }
        else if (testing_input[1] != '0' && testing_input[1] != '1' && testing_input[1] != '9') {
            printf("ERROR: %s IS Not a supported S-record only S0, S1, S9\n", testing_input);
        }

        sscanf(&testing_input[2], "%2x", &length);
        sscanf(&testing_input[4], "%2x%2x", &address_hi, &address_lo);

        origin_address = (address_hi << 8) | address_lo;
        int data_length = (length * 2);
        CheckSum = (unsigned char)address_lo + (unsigned char)address_hi + (unsigned char)length;

#ifdef DEBUG
        printf("Payload Length: %i bytes \n", data_length);
#endif

        if (testing_input[1] == '0') {
            // Display File Nom
            unsigned int filename;
            int test = 0;
            printf("FILE NAME: ");
            for (int i = 8; i < data_length + 2; i += 2) {
                sscanf(&testing_input[i], "%02x", &filename);
                printf("%c", filename);
                CheckSum += filename;
            }
            printf("\n");



        }

        else if (testing_input[1] == '1') {
            /*The program must extract the address of the first byte in the record. Subsequent
            data bytes are stored in contiguous locations. If there is already a value in a memory location, it must
            be overwritten.
            */
            unsigned short byte;
            int loop_end = data_length + 2;
            int iter_adr = origin_address;
#ifdef DEBUG
            int data_starting_adress = origin_address;
                printf("data starting address for S1");
#endif
            for (int i = DataStart; i < loop_end; i += 2) {
                // Writing to address one byte at a time 
                sscanf(&testing_input[i], "%2hx", &byte);
                Bus(iter_adr, &byte, 1, 1);
                CheckSum = (unsigned int)byte + CheckSum;
                iter_adr += 1;
            }


        }
        else if (testing_input[1] == '9') {
            PC = origin_address;
            //sscanf(&origin_address, "%0hhx", &PC);
#ifdef DEBUG
            printf("Adress of PC = %2X\n", PC);
#endif


        }

        int number_of_digits = (length * 2) + 4; // SumOfDigits = (length*2) + header (2) + Count (2)
        int checksum_postion = data_length + 2;
        unsigned int checksum_read = 0;
        sscanf(&testing_input[checksum_postion], "%02x", &checksum_read);

        CheckSum = CheckSum + checksum_read;
        if (CheckSum != 255) printf("Error: Invalid Checksum\n");
    }
    fclose(in_file);


}

