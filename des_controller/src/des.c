/*
 * des.c
 *
 *  Created on: Mar. 24, 2025
 *      Author: Amy-PC
 */


#include "des.h"

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
	case EVENT_LC:		return "LC";
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
	case STATE_CLEANUP:			return "System cleanup requested ... Transitioning to 'CLEANUP' state!";
	case STATE_WEIGHT_MEASURED:	return "Weight scale measured ... Transitioning to 'WEIGHT_MEASURED' state!";
	case STATE_FINAL:			return "System finalized ('FINAL' state)! Shutting down.";
	default:
		return "UNKNOWN_OUTPUT";
	}
}
