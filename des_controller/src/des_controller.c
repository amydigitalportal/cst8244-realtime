#include "des.h"
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


name_attach_t* attach;
int coid;

DES_Message device_request;
DES_State *currentState;
int isAccessAuthorized = 0;

Door leftDoor = {0};
Door rightDoor = {0};
int registeredWeight = 0;

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
DES_State *handle_cleanup_state(void);
DES_State *handle_weight_measured_state(void);

// Define DES States
DES_State initial_state 			= { STATE_INITIAL, 			"INITIAL",			handle_initial_state 			};
DES_State final_state 				= { STATE_FINAL, 			"FINAL",			handle_final_state 				};
DES_State idle_state 				= { STATE_IDLE, 			"IDLE",				handle_idle_state 				};
DES_State access_granted_state 		= { STATE_ACCESS_GRANTED, 	"ACCESS_GRANTED",	handle_access_granted_state 	};
DES_State entry_opened_state 		= { STATE_ENTRY_OPENED, 	"ENTRY_OPENED",		handle_entry_opened_state 		};
DES_State entry_closed_state 		= { STATE_ENTRY_CLOSED, 	"ENTRY_CLOSED",		handle_entry_closed_state 		};
DES_State entry_unlocked_state 		= { STATE_ENTRY_UNLOCKED, 	"ENTRY_UNLOCKED",	handle_entry_unlocked_state 	};
DES_State entry_secured_state 		= { STATE_ENTRY_SECURED, 	"ENTRY_SECURED",	handle_entry_secured_state 		};
DES_State exit_opened_state 		= { STATE_EXIT_OPENED, 		"EXIT_OPENED",		handle_exit_opened_state 		};
DES_State exit_closed_state 		= { STATE_EXIT_CLOSED, 		"EXIT_CLOSED",		handle_exit_closed_state 		};
DES_State exit_unlocked_state 		= { STATE_EXIT_UNLOCKED, 	"EXIT_UNLOCKED",	handle_exit_unlocked_state 		};
DES_State cleanup_state 			= { STATE_CLEANUP, 			"CLEANUP",			handle_cleanup_state 			};
DES_State weight_measured_state 	= { STATE_WEIGHT_MEASURED, 	"WEIGHT_MEASURED",	handle_weight_measured_state 	};


void updateDisplay(const char *format, ...) {
    va_list args;
    va_start(args, format);

    // Properly forward variadic arguments
    char temp_buffer[BUFFER_SIZE];
    vsnprintf(temp_buffer, BUFFER_SIZE, format, args);
    va_end(args);

    write_to_buffer("%s", temp_buffer);
//    printf("%s\n", pGlobalBuffer); // Immediately print the buffer


	// Send IPC message
//    coid = name_open(NAMESPACE_DISPLAY, 0);
    if (coid == -1) {
        printf("des_controller: name_open failed to connect to des_display. Foregoing send.\n");
        return;
    }

	DisplayMessage msg;
	msg.type = DISPLAY;
	strncpy(msg.payload, pGlobalBuffer, BUFFER_SIZE - 1);
	msg.payload[BUFFER_SIZE - 1] = '\0'; // Ensure null termination

	// Send message to the display
	if (MsgSend(coid, &msg, sizeof(msg), NULL, 0) == -1) {
		perror("Controller: MsgSend failed");
	}

//    name_close(coid);
}

void broadcastShutdown() {
    write_to_buffer("%s", "des_controller: Relaying shutdown order!");

//    int coid = name_open(NAMESPACE_DISPLAY, 0);
    if (coid == -1) {
        printf("des_controller: name_open failed to connect to des_display. Foregoing send.\n");
        return;
    }

	DisplayMessage msg;
	msg.type = SHUTDOWN;
	strncpy(msg.payload, pGlobalBuffer, BUFFER_SIZE - 1);
	msg.payload[BUFFER_SIZE - 1] = '\0'; // Ensure null termination

	// Send message to the display
	if (MsgSend(coid, &msg, sizeof(msg), NULL, 0) == -1) {
		perror("des_controller: MsgSend failed");
	}

//	name_close(coid);
}

// Blocking function to receive a DES_Message from des_inputs.
void receiveMessage() {
//	char raw_buffer[sizeof(DES_Message)];
//	printf("[DEBUG] sizeof(DES_Message) = %zu\n", sizeof(DES_Message));
//	int rcvid = MsgReceive(attach->chid, raw_buffer, sizeof(raw_buffer), NULL);
    int rcvid = MsgReceive(attach->chid, &device_request, sizeof(device_request), NULL);
    if (rcvid == -1) {
        perror("des_controller: MsgReceive failed!");
        return;
    }

	const char* inputCode = getInputCode(device_request.eventType);
	printf("\n[des_controller] received input: ");

	if (device_request.data >= 0) {
		printf("{%s, '%d'}", inputCode, device_request.data);
	} else {
		printf("{%s}", inputCode);
	}

	// Acknowledge receipt.
	if (MsgReply(rcvid, EOK, NULL, 0) == -1) {
		perror("des_controller: MsgReply failed!");
	}
}

DES_StateID updateStateMachine() {
	if (device_request.eventType < 0) {
		return -1;
	}

	// perform the state's function and retrieve next state
	DES_State *nextState = currentState->handler();

	// Capture the next state.
	currentState = nextState;

	// Handle CLEANUP phase.
	if (currentState->id == STATE_CLEANUP) {
		updateDisplay("%s\n", getOutputMessage(STATE_CLEANUP));

	    name_close(coid);
	    name_detach(attach, 0);

	    // Force Transition to Final State
	    currentState = &final_state;
	    updateDisplay("%s\n", getOutputMessage(STATE_FINAL));
	    broadcastShutdown();
	    return STATE_FINAL;
	}

	return currentState->id;
}

void resetDoor(Door *door) {
	if (door) {
		door->isEntrance = 0;
		door->isOpened = 0;
		door->isUnlocked = 0;
	}
}

void reinitialize() {
	isAccessAuthorized = 0;
	resetDoor(&leftDoor);
	resetDoor(&rightDoor);
	registeredWeight = 0;
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
	updateDisplay("Error: Unrecognized input '%s' from current state (%s)! State remaining unchanged.\n", getInputCode(device_request.eventType), currentState->name);
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
	// perform authentication
	case EVENT_LS: 	authSuccess = validateCredentials(person_id, &leftDoor); break;
	case EVENT_RS:	authSuccess = validateCredentials(person_id, &rightDoor); break;
	default: break;
	}

	isAccessAuthorized = authSuccess;

	if (device_request.eventType == EVENT_LS || device_request.eventType == EVENT_RS) {
		if (isAccessAuthorized) {
			updateDisplay("(person_id: %d) AUTHORIZED. %s\n", person_id, getOutputMessage(access_granted_state.id));
			next_state = &access_granted_state;
		} else {
			updateDisplay("(person_id: %d) DENIED. Maintaining idle state ...\n");
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
	case EVENT_GRU: {
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
	DES_State *next_state = &entry_closed_state;

	switch (device_request.eventType) {
	case EVENT_GLL:
	case EVENT_GRL: {
		Door *targetDoor = (device_request.eventType == EVENT_GLL) ? &leftDoor : &rightDoor;
		if (targetDoor->isEntrance) {
			targetDoor->isUnlocked = 0;
			updateDisplay("%s\n", getOutputMessage(STATE_ENTRY_SECURED));
			next_state = &entry_secured_state;
		} else {
			updateDisplay("Error: a different door is currently waiting to be secured! Maintaining 'ENTRY_CLOSED' state.");
		}
		break;
	}
	default:
		handleInvalidInputRequest();
		break;
	}

	return next_state;
}

DES_State *handle_entry_opened_state() {
	DES_State *next_state = &entry_opened_state;
	int weightReading = device_request.data;

	switch (device_request.eventType) {
	case EVENT_WS:
		registeredWeight = weightReading;
		updateDisplay("%s\n", getOutputMessage(STATE_WEIGHT_MEASURED));
		next_state = &weight_measured_state;
		break;
	case EVENT_LC:
	case EVENT_RC: {
		Door *targetDoor = (device_request.eventType == EVENT_LC) ? &leftDoor : &rightDoor;
		// Check if target door is the EXIT door.
		if (!targetDoor->isEntrance) {
			updateDisplay("Error: another access point is awaiting unlock.");
		} else {
			targetDoor->isOpened = 0;
			updateDisplay("%s\n", getOutputMessage(STATE_ENTRY_CLOSED));
			next_state = &entry_closed_state;
		}
		break;
	}
	default:
		handleInvalidInputRequest();
		break;
	}

	return next_state;
}

DES_State *handle_entry_secured_state() {
	DES_State *next_state = &entry_secured_state;

	// Handle unoccupied control point.
	if (registeredWeight <= 0) {
		// Transition to idle state.
		updateDisplay("Door cycle cancelled - %s\n", getOutputMessage(STATE_IDLE));
		reinitialize();
		next_state = &idle_state;
	}
	else
	// Handle occupied control point.
	{
		switch (device_request.eventType) {
		case EVENT_GLU:
		case EVENT_GRU: {
			Door *targetDoor = (device_request.eventType == EVENT_GLU) ? &leftDoor : &rightDoor;
			if (targetDoor->isEntrance) {
				updateDisplay("Error: a different door is awaiting unlock! Maintaining 'ENTRY_SECURED' state.");
			} else {
				targetDoor->isUnlocked = 1;
				updateDisplay("%s\n", getOutputMessage(STATE_EXIT_UNLOCKED));
				next_state = &exit_unlocked_state;
			}
			break;
		}
		default:
			handleInvalidInputRequest();
			break;
		}
	}

	return next_state;
}

DES_State *handle_entry_unlocked_state() {
	DES_State *next_state = &entry_unlocked_state;

	switch (device_request.eventType) {
	case EVENT_LO:
	case EVENT_RO: {
		Door *targetDoor = (device_request.eventType == EVENT_LO) ? &leftDoor : &rightDoor;
		if (!targetDoor->isEntrance) {
			updateDisplay("Error: a different entry door is currently unlocked! Maintaining 'ENTRY_UNLOCKED' state.");
		} else {
			// Check if the door is still locked
			if (!targetDoor->isUnlocked) {
				updateDisplay("Error: Cannot open locked door! Maintaining 'ENTRY_UNLOCKED' state.");
			} else {
				targetDoor->isOpened = 1;
				updateDisplay("%s\n", getOutputMessage(STATE_ENTRY_OPENED));
				next_state = &entry_opened_state;
			}
		}
		break;
	}
	default:
		handleInvalidInputRequest();
		break;
	}

	return next_state;
}


DES_State *handle_exit_closed_state() {
	DES_State *next_state = &exit_closed_state;

	switch (device_request.eventType) {
	case EVENT_GLL:
	case EVENT_GRL: {
		Door *targetDoor = (device_request.eventType == EVENT_GLL) ? &leftDoor : &rightDoor;
		if (targetDoor->isEntrance) {
			updateDisplay("Error: a different exit door is waiting to be opened! Maintaining 'EXIT_UNLOCKED' state.");
		} else {
			targetDoor->isUnlocked = 0;
			updateDisplay("Exit door secured and cycle complete - %s\n", getOutputMessage(STATE_IDLE));
			reinitialize();
			next_state = &idle_state;
		}
		break;
	}
	default:
		handleInvalidInputRequest();
		break;
	}

	return next_state;
}

DES_State *handle_exit_opened_state() {
	DES_State *next_state = &exit_opened_state;

	switch (device_request.eventType) {
	case EVENT_LC:
	case EVENT_RC: {
		Door *targetDoor = (device_request.eventType == EVENT_LC) ? &leftDoor : &rightDoor;
		if (targetDoor->isEntrance) {
			updateDisplay("Error: a different exit door is waiting to be closed! Maintaining 'EXIT_UNLOCKED' state.");
		} else {
			targetDoor->isOpened = 0;
			updateDisplay("%s\n", getOutputMessage(STATE_EXIT_CLOSED));
			next_state = &exit_closed_state;
		}
		break;
	}
	default:
		handleInvalidInputRequest();
		break;
	}

	return next_state;
}

DES_State *handle_exit_unlocked_state() {
	DES_State *next_state = &exit_unlocked_state;

	switch (device_request.eventType) {
	case EVENT_LO:
	case EVENT_RO: {
		Door *targetDoor = (device_request.eventType == EVENT_LO) ? &leftDoor : &rightDoor;
		if (targetDoor->isEntrance) {
			updateDisplay("Error: a different exit door is waiting to be opened! Maintaining 'EXIT_UNLOCKED' state.");
		} else {
			targetDoor->isOpened = 1;
			updateDisplay("%s\n", getOutputMessage(STATE_EXIT_OPENED));
			next_state = &exit_opened_state;
		}
		break;
	}
	default:
		handleInvalidInputRequest();
		break;
	}

	return next_state;
}

DES_State *handle_weight_measured_state() {
	DES_State *next_state = &weight_measured_state;

	switch (device_request.eventType) {
	case EVENT_LC:
	case EVENT_RC: {
		Door *targetDoor = (device_request.eventType == EVENT_LC) ? &leftDoor : &rightDoor;
		if (targetDoor->isEntrance) {
			targetDoor->isOpened = 0;
			updateDisplay("%s\n", getOutputMessage(STATE_ENTRY_CLOSED));
			next_state = &entry_closed_state;
		} else {
			updateDisplay("Error: a different door is currently opened! Maintaining 'WEIGHT_MEASURED' state.");
		}
		break;
	}
	default:
		handleInvalidInputRequest();
		break;
	}

	return next_state;
}

DES_State *handle_cleanup_state() {
	return &idle_state;
}

DES_State *handle_final_state() {
	return &final_state;
}


//–––––– MAIN FSM LOOP ––––––
int main() {
	// Perform attachment to namespace for IPC
	attach = name_attach(NULL, NAMESPACE_CONTROLLER, 0);
	if (attach == NULL) {
		perror("des_controller: name_attach failed. Process terminating...");
		exit(EXIT_FAILURE);
	}
    printf("des_controller: Listening via namespace '%s'\n", NAMESPACE_CONTROLLER);


    coid = name_open(NAMESPACE_DISPLAY, 0);

	// -- BEGIN Controller Server Logic.

	device_request.eventType = -1;
	device_request.data = -1;

    // Start the FSM in the IDLE state
    currentState = &initial_state;
    while (1) {
    	// Perform an update on the state machine.
    	if (updateStateMachine() == STATE_FINAL) {
    	    sleep(1);
    	    return EXIT_SUCCESS;
    	}

    	printf("\n");
    	receiveMessage(); // Listens for requests from external devices.
    }

    return EXIT_SUCCESS;
}

