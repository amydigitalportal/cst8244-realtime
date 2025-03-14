#include "des.h"
#include "buffer_manager.h"
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int chid = -1;  // QNX Channel ID

DES_Message device_request;
DES_State *currentState;
int isAccessAuthorized = 0;

Door leftDoor;
Door rightDoor;

// Forward declarations of state handler functions
DES_State *handle_initial_state(void);
DES_State *handle_final_state(void);
DES_State *handle_idle_state(void);
DES_State *handle_access_granted_state(void);
DES_State *handle_entry_opened_state(void);
DES_State *handle_entry_closed_state(void);
DES_State *handle_entry_unlocked_state(void);
DES_State *handle_entry_secured_state(void);
DES_State *handle_exit_opened_state(void);
DES_State *handle_exit_closed_state(void);
DES_State *handle_exit_unlocked_state(void);
DES_State *handle_exit_secured_state(void);
DES_State *handle_cleanup_state(void);
DES_State *handle_weight_measured_state(void);

// Define DES States
DES_State initial_state 			= { STATE_INITIAL, 			handle_initial_state 			};
DES_State final_state 				= { STATE_FINAL, 			handle_final_state 				};
DES_State idle_state 				= { STATE_IDLE, 			handle_idle_state 				};
DES_State access_granted_state 		= { STATE_ACCESS_GRANTED, 	handle_access_granted_state 	};
DES_State entry_opened_state 		= { STATE_ENTRY_OPENED, 	handle_entry_opened_state 		};
DES_State entry_closed_state 		= { STATE_ENTRY_CLOSED, 	handle_entry_closed_state 		};
DES_State entry_unlocked_state 		= { STATE_ENTRY_UNLOCKED, 	handle_entry_unlocked_state 	};
DES_State entry_secured_state 		= { STATE_ENTRY_SECURED, 	handle_entry_secured_state 		};
DES_State exit_opened_state 		= { STATE_EXIT_OPENED, 		handle_exit_opened_state 		};
DES_State exit_closed_state 		= { STATE_EXIT_CLOSED, 		handle_exit_closed_state 		};
DES_State exit_unlocked_state 		= { STATE_EXIT_UNLOCKED, 	handle_exit_unlocked_state 		};
DES_State exit_secured_state 		= { STATE_EXIT_SECURED, 	handle_exit_secured_state 		};
DES_State cleanup_state 			= { STATE_CLEANUP, 			handle_cleanup_state 			};
DES_State weight_measured_state 	= { STATE_WEIGHT_MEASURED, 	handle_weight_measured_state 	};



// Function to send a message to des_display to have it updated.
//void updateDisplay() {
//	const char* outputMessage = getOutputMessage(device_request.eventType);
//	if (device_request.data >= 0) {
//    	printf("des_controller: {%s, '%d'}\n", outputMessage, device_request.data);
//	} else {
//    	printf("des_controller: {%s}\n", outputMessage);
//	}
//    // TODO: implement SendMsg
//}
void updateDisplay(const char *format, ...) {
    va_list args;
    va_start(args, format);

    // Properly forward variadic arguments
    char temp_buffer[BUFFER_SIZE];
    vsnprintf(temp_buffer, BUFFER_SIZE, format, args);
    va_end(args);

    write_to_buffer("%s", temp_buffer);
    printf("%s\n", pGlobalBuffer); // Immediately print the buffer
     // TODO: implement SendMsg
}

// Blocking function to receive a DES_Message from des_inputs.
void receiveMessage() {
    int rcvid = MsgReceive(chid, &device_request, sizeof(device_request), NULL);
    if (rcvid == -1) {
        perror("MsgReceive failed");
    } else {
    	const char* inputCode = getInputCode(device_request.eventType);
    	if (device_request.data >= 0) {
        	printf("\ndes_controller: {%s, '%d'}\n\n", inputCode, device_request.data);
    	} else {
        	printf("\ndes_controller: {%s}\n\n", inputCode);
    	}

        // Acknowledge receipt.
        MsgReply(rcvid, EOK, NULL, 0);
    }
}

void updateStateMachine() {
	if (device_request.eventType < 0) {
		return;
	}

	// perform the state's function and retrieve next state
	DES_State *nextState = currentState->handler();
	// Capture the next state.
	currentState = nextState;
}

void resetDoor(Door *door) {
	door->isEntrance = 0;
	door->isOpened = 0;
	door->isUnlocked = 0;
}

int validateCredentials(int personId, Door *entranceDoor) {
	// Perform validation checks...

	// ID has correct range
	if (personId >= VALID_PERSON_ID_MIN && personId <= VALID_PERSON_ID_MAX)
	{
		entranceDoor->isEntrance = 1;
		return 1;
	}

	return 0;
}

void handleInvalidInputRequest() {
	updateDisplay("Error: Unrecognized input '%s'! Maintaining 'ACCESS_GRANTED' state.", getInputCode(device_request.eventType));
//	printf("ERROR! Received invalid request type: '%s'.\n", getInputCode(device_request.eventType));
}

//–––––– STATE HANDLER FUNCTIONS ––––––

DES_State *handle_initial_state() {
	printf("%s\n", getOutputMessage(initial_state.id));
	double delay = 0.5;
	usleep((useconds_t)(delay * 1000000)); printf("."); fflush(stdout);
	usleep((useconds_t)(delay * 1000000)); printf("."); fflush(stdout);
	usleep((useconds_t)(delay * 1000000)); printf("."); fflush(stdout);
	sleep(1);

	printf("\n\n%s\n", getOutputMessage(idle_state.id));
	return &idle_state;
}

DES_State *handle_idle_state() {
	DES_State *next_state = &idle_state;
	int person_id = device_request.data;
	int authSuccess = 0;
	resetDoor(&leftDoor); resetDoor(&rightDoor);

	switch (device_request.eventType) {
	case EVENT_EXIT:
		updateDisplay("%s ... ", getOutputMessage(STATE_CLEANUP));
		return &cleanup_state;
	case EVENT_LS: 	authSuccess = validateCredentials(person_id, &leftDoor); break;
	case EVENT_RS:	authSuccess = validateCredentials(person_id, &rightDoor); break;
	default: break;
	}

	if (device_request.eventType == EVENT_LS || device_request.eventType == EVENT_RS) {
		if (authSuccess) {
//			printf("(person_id: %d) AUTHORIZED. %s ...\n", person_id, getOutputMessage(access_granted_state.id));
			updateDisplay("(person_id: %d) AUTHORIZED. %s ...\n", person_id, getOutputMessage(access_granted_state.id));
			next_state = &access_granted_state;
		} else {
			updateDisplay("(person_id: %d) DENIED. Maintaining idle state ...\n");
			next_state = &idle_state;
		}
	} else {
		handleInvalidInputRequest();
	}

	return next_state;
}

DES_State *handle_access_granted_state() {
	DES_State *next_state = &access_granted_state;

	switch (device_request.eventType) {
	case EVENT_GLU:
//		if (leftDoor.isEntrance)
//		{
//			leftDoor.isUnlocked = 1;
//			updateDisplay("%s ...\n", getOutputMessage(STATE_ENTRY_UNLOCKED));
//			next_state = &entry_unlocked_state;
//		} else {
//			updateDisplay("Error: another access point is awaiting unlock.");
//		}
//		break;
//	case EVENT_GRU:
	case EVENT_GRU: {
//		if (rightDoor.isEntrance)
//		{
//			rightDoor.isUnlocked = 1;
//			updateDisplay("%s ...\n", getOutputMessage(STATE_ENTRY_UNLOCKED));
//			next_state = &entry_unlocked_state;
//		} else {
//			updateDisplay("Error: another access point is awaiting unlock.");
//		}
		Door *targetDoor = (device_request.eventType == EVENT_GLU) ? &leftDoor : &rightDoor;
		if (targetDoor->isEntrance) {
			targetDoor->isUnlocked = 1;
			updateDisplay("%s\n", getOutputMessage(STATE_ENTRY_UNLOCKED));
			next_state = &entry_unlocked_state;
		} else {
			updateDisplay("Error: another access point is awaiting unlock.");
		}
		break;
	}
	default:
		handleInvalidInputRequest();
		break;
	}

	return next_state;
}


DES_State *handle_entry_closed_state() {
	DES_State *next_state;
	return next_state;
}

DES_State *handle_entry_opened_state() {
	DES_State *next_state;
	return next_state;
}

DES_State *handle_entry_secured_state() {
	DES_State *next_state;
	return next_state;
}

DES_State *handle_entry_unlocked_state() {
	DES_State *next_state = &entry_unlocked_state;
	switch (device_request.eventType) {
	case EVENT_LO:
	case EVENT_RO: {
		Door *targetDoor = (device_request.eventType == EVENT_LO) ? &leftDoor : &rightDoor;
		if (targetDoor->isEntrance) {
			// Check if the door has been unlocked
			if (targetDoor->isUnlocked) {
				targetDoor->isOpened = 1;
				updateDisplay("%s\n", getOutputMessage(STATE_ENTRY_OPENED));
				next_state = &entry_opened_state;
			} else {
				updateDisplay("Error: Door not yet unlocked! Maintaining 'ENTRY_UNLOCKED' state.");
			}
		} else {
			updateDisplay("Error: another door is currently unlocked! Maintaining 'ENTRY_UNLOCKED' state.");
		}
		break;
	}
	default:
		updateDisplay("Error: '%s' for input {%s}! Maintaining 'ACCESS_GRANTED' state.", getOutputMessage(-1), getInputCode(device_request.eventType));
		break;
	}

	return next_state;
}


DES_State *handle_exit_closed_state() {
	DES_State *next_state;
	return next_state;
}

DES_State *handle_exit_opened_state() {
	DES_State *next_state;
	return next_state;
}

DES_State *handle_exit_secured_state() {
	DES_State *next_state;
	return next_state;
}

DES_State *handle_exit_unlocked_state() {
	DES_State *next_state;
	return next_state;
}

DES_State *handle_weight_measured_state() {
	DES_State *next_state;
	return next_state;
}

DES_State *handle_cleanup_state() {
	return &idle_state;
}

DES_State *handle_final_state() {
	return &final_state;
}


//// STATE_IDLE: Wait for a scan event. If EVENT_LS, transition to left-unlock; if EVENT_RS, to right-unlock.
//DES_State idle_state_handler(void) {
//    switch (device_request.eventType) {
//        case EVENT_LS:
//            printf("[IDLE] EVENT_LS received. (person_id: %d)\n", device_request.data);
//            authenticatedPersonId = device_request.data;
//            break;
//        case EVENT_GLU:
//        	printf("[IDLE] EVENT_GLU received. Left-door unlocked!\n");
//			doorState |= LEFT_UNLOCKED_FLAG;
//			return STATE_LEFT_UNLOCKED;
//			if (authenticatedPersonId <= 0)
//				printf("-- WARNING! LOCK OVERRIDE -- No `person_id` provided.");
//        	break;
//        case EVENT_RS:
//            printf("[IDLE] EVENT_RS received. (person_id: %d)\n", device_request.data);
//            authenticatedPersonId = device_request.data;
//            break;
//        case EVENT_GRU:
//			printf("[IDLE] EVENT_GRU received. Right-door unlocked!\n");
//			doorState |= RIGHT_UNLOCKED_FLAG;
//			return STATE_RIGHT_UNLOCKED;
//			if (authenticatedPersonId <= 0)
//				printf("-- WARNING! LOCK OVERRIDE -- No `person_id` provided.");
//        	break;
//        default:
//        	break;
//    }
//
////    printf("[IDLE] Current door state: 0x%X\n", doorState);
//    return STATE_IDLE;
//}
//
//// STATE_LEFT_UNLOCKED: Left scan has been processed and the left door is to be unlocked.
//DES_State left_unlocked_state_handler(void) {
//    switch (device_request.eventType) {
//        case EVENT_LO:
//			printf("[LEFT_UNLOCKED] EVENT_LO received. ");
//			// Open left door
//			doorState |= LEFT_OPENED_FLAG;
//			printf("Left door opened; transitioning to STATE_LEFT_OPENED\n");
//			return STATE_LEFT_OPENED;
//        case EVENT_GLL:
//            printf("[LEFT_UNLOCKED] EVENT_GLL received. ");
//            // Cancel unlocking (lock left door)
//            doorState &= ~LEFT_UNLOCKED_FLAG;
//            printf("Left door locked; returning to STATE_IDLE\n");
//            return STATE_IDLE;
//        default:
//        	break;
//    }
//
//    // No change in state.
//    return STATE_LEFT_UNLOCKED;
//}
//
//// STATE_LEFT_OPENED: Left door is open. Await weight measurement or door closure.
//DES_State left_opened_state_handler(void) {
//    switch (device_request.eventType) {
//        case EVENT_WS:
//            printf("[LEFT_OPENED] EVENT_WS received. ");
//
//            // Register weight measured.
//            printf("Weight measured; transitioning to STATE_WEIGHT_MEASURED\n");
//            return STATE_WEIGHT_MEASURED;
//        case EVENT_LC:
//            printf("[LEFT_OPENED] EVENT_LC received. \n");
//            // Close Left Door.
//            doorState &= ~LEFT_OPENED_FLAG;
//            printf("Left door closed; transitioning to STATE_LEFT_SECURED");
//            return STATE_LEFT_SECURED;
//        default:
//        	break;
//    }
//
//	// No change in state.
//    return STATE_LEFT_OPENED;
//}
//
//// STATE_WEIGHT_MEASURED: Weight has been recorded. Determine which door was active.
//DES_State weight_measured_state_handler(void) {
//
//	switch (device_request.eventType) {
//		case EVENT_LC:
//			printf("[WEIGHT_MEASURED] EVENT_LC received. ");
//
//			// Check if Left-door is open.
//			if (doorState & LEFT_OPENED_FLAG) {
//				printf("Closing Left-door.\n");
//				// Close Left Door.
//				doorState &= ~(LEFT_OPENED_FLAG);
//			} else {
//				printf("ERROR-STATE: NO DOOR TO CLOSE; Left-door is not open!\n");
//			}
//			break;
//		case EVENT_RC:
//			printf("[WEIGHT_MEASURED] EVENT_RC received. ");
//
//			// Check if Right-door is open.
//			if (doorState & RIGHT_OPENED_FLAG) {
//				printf("Closing Right-door.\n");
//				// Close Right Door.
//				doorState &= ~(RIGHT_OPENED_FLAG);
//			} else {
//				printf("ERROR-STATE: NO DOOR TO CLOSE; Right-door is not open!\n");
//			}
//			break;
//		case EVENT_GLL:
//			printf("[WEIGHT_MEASURED] GLL received.\n");
//
//			// Check if Left-door is closed.
//			if (doorState & ~LEFT_OPENED_FLAG) {
//				// Lock left door.
//				doorState &= ~LEFT_UNLOCKED_FLAG;
//				printf("Left-door unlocked; transitioning to STATE_LEFT_SECURED\n");
//				return STATE_LEFT_SECURED;
//			}
//			break;
//		case EVENT_GRL:
//			printf("[WEIGHT_MEASURED] GRL received.\n");
//
//			// Check if Right-door is closed.
//			if (doorState & ~RIGHT_OPENED_FLAG) {
//				// Lock right door.
//				doorState &= ~RIGHT_UNLOCKED_FLAG;
//				printf("Right-door unlocked; transitioning to STATE_RIGHT_SECURED\n");
//				return STATE_RIGHT_SECURED;
//			}
//			break;
//		default:
//			break;
//	}
//
//	// No change in state.
//	return STATE_WEIGHT_MEASURED;
//}
//
//// STATE_LEFT_SECURED: Left door is now closed and locked.
//DES_State left_secured_state_handler(void) {
//    if (device_request.eventType == EVENT_GRU) {
//    	printf("[LEFT_SECURED] EVENT_GRU received. ");
//
//        // Unlock right door event; enforce airlock.
//        doorState |= RIGHT_UNLOCKED_FLAG;
//        doorState &= ~(LEFT_UNLOCKED_FLAG | LEFT_OPENED_FLAG);
//        printf("Right-door unlocked; transitioning to STATE_RIGHT_UNLOCKED\n");
//        return STATE_RIGHT_UNLOCKED;
//    }
//
//	// Return to Idle state.
//	printf("[LEFT_SECURED] Remaining locked; cycle complete, returning to STATE_IDLE\n");
//    return STATE_IDLE;
//}
//
//// STATE_RIGHT_UNLOCKED: Right scan has been processed and the right door is to be unlocked.
//DES_State right_unlocked_state_handler(void) {
//    switch (device_request.eventType) {
//        case EVENT_RO:
//        	printf("[RIGHT_UNLOCKED] EVENT_RO received. ");
//
//            doorState |= RIGHT_OPENED_FLAG;
//            printf("Right door opened; transitioning to STATE_RIGHT_OPENED\n");
//            return STATE_RIGHT_OPENED;
//        case EVENT_GRL:
//        	printf("[RIGHT_UNLOCKED] EVENT_GRL received. ");
//
//            doorState &= ~RIGHT_UNLOCKED_FLAG;
//            printf("Right door locked; transitioning to STATE_IDLE\n");
//            return STATE_IDLE;
//        default:
//        	break;
//    }
//
//	// No change in state.
//    return STATE_RIGHT_UNLOCKED;
//}
//
//// STATE_RIGHT_OPENED: Right door is open. Await weight measurement or door closure.
//DES_State right_opened_state_handler(void) {
//    switch (device_request.eventType) {
//        case EVENT_WS:
//        	printf("[RIGHT_OPENED] EVENT_WS received. ");
//
//            printf("Weight measured; transitioning to STATE_WEIGHT_MEASURED\n");
//            return STATE_WEIGHT_MEASURED;
//        case EVENT_RC:
//        	printf("[RIGHT_OPENED] EVENT_RC received. ");
//
//            doorState &= ~LEFT_OPENED_FLAG;
//            printf("[RIGHT_OPENED] EVENT_RC: Right door closed; transitioning to STATE_RIGHT_SECURED\n");
//            return STATE_RIGHT_SECURED;
//        default:
//        	break;
//    }
//
//	// No change in state.
//    return STATE_RIGHT_OPENED;
//}
//
//// STATE_RIGHT_SECURED: Right door is now closed and locked.
//DES_State right_secured_state_handler(void) {
//    if (device_request.eventType == EVENT_GLU) {
//    	printf("[RIGHT_SECURED] EVENT_GLU received. ");
//
//		// Unlock Left door.
//		doorState |= LEFT_UNLOCKED_FLAG;
//		doorState &= ~(RIGHT_UNLOCKED_FLAG | LEFT_OPENED_FLAG);
//		printf("Left door unlocked; transitioning to STATE_LEFT_UNLOCKED\n");
//		return STATE_LEFT_UNLOCKED;
//    }
//
//	// Return to Idle state.
//	printf("[RIGHT_SECURED] Remaining locked; cycle complete, returning to STATE_IDLE\n");
//	return STATE_IDLE;
//}

//–––––– MAIN FSM LOOP ––––––
int main(void) {

	device_request.eventType = -1;
	device_request.data = -1;

    // Print the controller's process ID
    printf("des_controller: running with process id %d\n\n", getpid());

    // Create a QNX channel for IPC
    chid = ChannelCreate(0);
    if (chid == -1) {
        perror("ChannelCreate failed");
        return EXIT_FAILURE;
    }

    // Start the FSM in the IDLE state
    currentState = &initial_state;
    while (1) {
    	updateStateMachine(); // Performs an update on the state machine.
//    	updateDisplay(); // Sends a message containing global buffer's contents to the display.
    	printf("\n");
    	receiveMessage(); // Listens for requests from external devices.
    	printf("des_controller: Message received: {%s}\n", getInputCode(device_request.eventType));
    }

    // Cleanup (never reached in this infinite loop)
    ChannelDestroy(chid);
    return EXIT_SUCCESS;
}

