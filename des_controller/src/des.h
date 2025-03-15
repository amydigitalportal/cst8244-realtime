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
#include "buffer_manager.h"

#define VALID_PERSON_ID_MIN 10000
#define VALID_PERSON_ID_MAX 99999

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

typedef struct {
	char payload[BUFFER_SIZE];
} DisplayMessage;

/**
 * Function for retrieving an input code for a specific event type,
 * representing an input signal from an external device.
 */
const char *getInputCode(EventType eventType) {
	switch (eventType) {
	case EVENT_LS:		return "LS";
	case EVENT_GLU:		return "GLU";
	case EVENT_GLL:		return "GLL";
	case EVENT_LO:		return "LO";
	case EVENT_LC:		return "RO";
	case EVENT_RS:		return "RS";
	case EVENT_GRU:		return "GRU";
	case EVENT_GRL:		return "GRL";
	case EVENT_RO:		return "RO";
	case EVENT_RC:		return "RC";
	case EVENT_WS:		return "WS";
	case EVENT_EXIT:	return "EXIT";
	default:
		return "UNKNOWN_INPUT";
	}
}

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
    struct DES_State *(*handler)(void);
} DES_State;

/**
 * Function for retrieving an output message for when a state is being entered,
 * representing a status update.
 */
const char *getOutputMessage(DES_StateID enteredState) {
	switch (enteredState) {
	case STATE_INITIAL:			return "System initializing ...";
	case STATE_IDLE:			return "... Transitioning to 'IDLE' state!";
	case STATE_ACCESS_GRANTED:	return "System granting access ... Transitioning to 'ACCESS_GRANTED' state!";
	case STATE_ENTRY_OPENED:	return "Entry door opened ... Transitioning to 'ENTRY_OPENED' state!";
	case STATE_ENTRY_CLOSED:	return "Entry door closed ... Transitioning to 'ENTRY_CLOSED' state!";
	case STATE_ENTRY_UNLOCKED:	return "Entry latch released ... Transitioning to 'ENTRY_UNLOCKED' state!";
	case STATE_ENTRY_SECURED:	return "Entry latch secured ... Transitioning to 'ENTRY_LOCKED' state!";
	case STATE_EXIT_OPENED:		return "Exit door opened ... Transitioning to 'EXIT_OPENED' state!";
	case STATE_EXIT_CLOSED:		return "Exit door closed ... Transitioning to 'EXIT_CLOSED' state!";
	case STATE_EXIT_UNLOCKED:	return "Exit latch released ... Transitioning to 'EXIT_UNLOCKED' state!";
	case STATE_CLEANUP:			return "System cleaning up ... Transitioning to 'CLEANUP' state!";
	case STATE_WEIGHT_MEASURED:	return "Weight scale measured ... Transitioning to 'WEIGHT_MEASURED' state!";
	case STATE_FINAL:			return "System finalized ('FINAL' state)! Shutting down.";
	default:
		return "UNKNOWN_OUTPUT";
	}
}

#endif // DES_H
