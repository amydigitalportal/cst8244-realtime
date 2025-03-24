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
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <errno.h>         // For error handling
#include <string.h>        // For memset
#include "buffer_manager.h"

#define VALID_PERSON_ID_MIN 10000
#define VALID_PERSON_ID_MAX 99999

#define NAMESPACE_INPUTS "des/inputs"
#define NAMESPACE_CONTROLLER "des/controller"
#define NAMESPACE_DISPLAY "des/display"

/**
 * Data structure holding state for a DES door.
 */
typedef struct {
	int isEntrance;
	int isUnlocked;
	int isOpened;
} Door;

/**
 * Enumeration of event types that are recognized by the DES event handler.
 */
typedef enum {
    EVENT_LS   	// Process L_SCANNER
    , EVENT_RS  // Process R_SCANNER
    , EVENT_WS  // Process WEIGHT_SCALE
	, EVENT_LO  // Open L_DOOR
	, EVENT_RO  // Open R_DOOR
	, EVENT_LC  // Close L_DOOR
	, EVENT_RC  // Close R_DOOR
	, EVENT_GRU // Guard unlock R_DOOR
	, EVENT_GRL // Guard lock R_DOOR
	, EVENT_GLL // Guard lock L_DOOR
	, EVENT_GLU	// Guard unlock L_DOOR
	, EVENT_TXT // Process Text Message
	, EVENT_PNC	// Panic! Release all locks and open doors. (too bad we're not doing anything about it XD)
	, EVENT_EXIT // Begin system shutdown.
	, EVENT_UNKNOWN // Unrecognized event
} EventType;
#define NUM_EVENT_TYPES 15

/**
 * Message structure for DES interprocess communication (IPC).
 */
typedef struct {
    EventType eventType; // Type of event received
    int data;
} DES_Message;

typedef enum {
	DISPLAY,
	SHUTDOWN
} MessageType;

typedef struct {
	MessageType type;
	char payload[BUFFER_SIZE];
} DisplayMessage;

const char *getInputCode(EventType eventType);

// Define system state IDs
typedef enum {
	STATE_INITIAL
	, STATE_IDLE
	, STATE_ACCESS_GRANTED
	, STATE_ENTRY_OPENED
	, STATE_ENTRY_CLOSED
	, STATE_ENTRY_UNLOCKED
	, STATE_ENTRY_SECURED
	, STATE_EXIT_OPENED
	, STATE_EXIT_CLOSED
	, STATE_EXIT_UNLOCKED
	, STATE_WEIGHT_MEASURED
	, STATE_CLEANUP
	, STATE_FINAL
} DES_StateID;
#define NUM_DES_STATES 11

typedef struct DES_State {
	DES_StateID id;
	const char* name;
    struct DES_State *(*handler)(void);
} DES_State;

const char *getOutputMessage(DES_StateID enteredState);

#endif // DES_H
