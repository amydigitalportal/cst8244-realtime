/*
 * des.h
 *
 *  Created on: Mar. 11, 2025
 *      Author: Amy-PC
 */

#ifndef DES_H
#define DES_H

// Define system states
typedef enum {
    START_STATE = 0, // Booting up and initializing
    READY_STATE = 1  // Ready to receive messages (from des_inputs)
} State;

#endif // DES_H
