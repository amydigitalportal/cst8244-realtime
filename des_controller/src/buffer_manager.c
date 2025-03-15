/*
 * buffer_manager.c
 *
 *  Created on: Mar. 14, 2025
 *      Author: amydi
 */


#include "buffer_manager.h"

// Define the global writable buffer
char global_buffer[BUFFER_SIZE] = {0};

// Define the read-only pointer
const char* pGlobalBuffer = global_buffer;

void write_to_buffer(const char *format, ...) {
    memset(global_buffer, 0, BUFFER_SIZE); // Clear the buffer

    va_list args; // list of variadic arguments
    va_start(args, format);
    vsnprintf(global_buffer, BUFFER_SIZE, format, args); // Write formatted string
    va_end(args);

    pGlobalBuffer = global_buffer; // Update pointer
}
