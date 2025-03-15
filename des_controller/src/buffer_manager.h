/*
 * buffer_manager.h
 *
 *  Created on: Mar. 14, 2025
 *      Author: amydi
 */
#ifndef SRC_BUFFER_MANAGER_H_
#define SRC_BUFFER_MANAGER_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define BUFFER_SIZE 256

extern char global_buffer[BUFFER_SIZE];
extern const char* pGlobalBuffer; // Should be used as read-only pointer.

/**
 * Function to write formatted data into the buffer.
 */
void write_to_buffer(const char *format, ...);

#endif /* SRC_BUFFER_MANAGER_H_ */
