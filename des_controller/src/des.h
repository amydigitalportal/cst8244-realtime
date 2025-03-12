/*
 * des.h
 *
 *  Created on: Mar. 11, 2025
 *      Author: Amy-PC
 */

#ifndef DES_H
#define DES_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>        // For getpid()
#include <sys/neutrino.h>  // For QNX messaging
#include <errno.h>         // For error handling
#include <string.h>        // For memset

//––– Define our door flag bit masks –––
#define LEFT_UNLOCKED_FLAG   (1 << 3)  // 0x8: left door unlocked
#define LEFT_OPENED_FLAG       (1 << 2)  // 0x4: left door open
#define RIGHT_UNLOCKED_FLAG  (1 << 1)  // 0x2: right door unlocked
#define RIGHT_OPENED_FLAG      (1 << 0)  // 0x1: right door open

// Global persistent door state – always maintained to enforce the airlock rule.
unsigned char doorState = 0x0; // Both doors locked & closed (0b0000)


// Define system states
#define NUM_DES_STATES 8
typedef enum {
	STATE_IDLE
	, STATE_LEFT_UNLOCKED
	, STATE_LEFT_OPENED
	, STATE_LEFT_SECURED
	, STATE_WEIGHT_MEASURED
	, STATE_RIGHT_UNLOCKED
	, STATE_RIGHT_OPENED
	, STATE_RIGHT_SECURED
} DES_State;

// Define event types for messages from devices
#define NUM_EVENT_TYPES 11
typedef enum {
    EVENT_LS   	// Left scan
    , EVENT_RS  // Right scan
    , EVENT_WS  // Weight scale
	, EVENT_LO  // Left open
	, EVENT_RO  // Right open
	, EVENT_LC  // Left closed
	, EVENT_RC  // Right closed
	, EVENT_GRU // Guard right unlock
	, EVENT_GRL // Guard right lock
	, EVENT_GLL // Guard left lock
	, EVENT_GLU	// Guard left unlock
} EventType;

// Define message structure for interprocess communication (IPC)
typedef struct {
    EventType eventType; // Type of event received
    int data;
} DES_Message;

const char *outMessage[NUM_DES_STATES] = {
	"System IDLE ..."
	, "Device: Left Latch UNLOCKED"
	, "Device: Left Door OPENED"
	, "Device: Left Door SECURED"
	, "Device: Scale MEASURED. weight:"
	, "Device: Right Door UNLOCKED"
	, "Device: Right Door OPENED"
	, "Device: Right Door SECURED"
};

const char *inMessage[NUM_EVENT_TYPES] = {
	"LS"
	, "RS"
	, "WS"
	, "LO"
	, "RO"
	, "LC"
	, "RC"
	, "GRU"
	, "GRL"
	, "GLL"
	, "GLU"
};

#endif // DES_H
