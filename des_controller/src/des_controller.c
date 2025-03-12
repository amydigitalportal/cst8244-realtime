#include "des.h"
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int chid = -1;  // QNX Channel ID
DES_Message device_request;
DES_State currentDesState;
int authenticatedPersonId = 0;

// Forward declarations of state handler functions
DES_State idle_state_handler(void);
DES_State left_unlocked_state_handler(void);
DES_State left_opened_state_handler(void);
DES_State left_secured_state_handler(void);
DES_State weight_measured_state_handler(void);
DES_State right_unlocked_state_handler(void);
DES_State right_opened_state_handler(void);
DES_State right_secured_state_handler(void);


// Function to send a message to des_display to have it updated.
void updateDisplay() {
	if (device_request.data >= 0) {
    	printf("des_controller: {%s, '%d}'\n", outMessage[device_request.eventType], device_request.data);
	} else {
    	printf("des_controller: {%s}\n", outMessage[device_request.eventType]);
	}
    // TODO: implement SendMsg
}

// Blocking function to receive a DES_Message from des_inputs.
void receiveMessage() {
    int rcvid = MsgReceive(chid, &device_request, sizeof(device_request), NULL);
    if (rcvid == -1) {
        perror("MsgReceive failed");
    } else {

    	if (device_request.data >= 0) {
        	printf("\ndes_controller: {%s, '%d}'\n\n", inMessage[device_request.eventType], device_request.data);
    	} else {
        	printf("\ndes_controller: {%s}\n\n", inMessage[device_request.eventType]);
    	}

        // Acknowledge receipt.
        MsgReply(rcvid, EOK, NULL, 0);
    }
}

void handleCurrentState() {
	if (device_request.eventType < 0) {
		return;
	}
    switch (currentDesState) {
        case STATE_IDLE:
        	currentDesState = idle_state_handler();
            break;
        case STATE_LEFT_UNLOCKED:
        	currentDesState = left_unlocked_state_handler();
            break;
        case STATE_LEFT_OPENED:
        	currentDesState = left_opened_state_handler();
            break;
        case STATE_WEIGHT_MEASURED:
        	currentDesState = weight_measured_state_handler();
            break;
        case STATE_LEFT_SECURED:
        	currentDesState = left_secured_state_handler();
            break;
        case STATE_RIGHT_UNLOCKED:
        	currentDesState = right_unlocked_state_handler();
            break;
        case STATE_RIGHT_OPENED:
        	currentDesState = right_opened_state_handler();
            break;
        case STATE_RIGHT_SECURED:
        	currentDesState = right_secured_state_handler();
            break;
        default:
            printf("Invalid state encountered. Resetting to STATE_IDLE.\n");
            currentDesState = STATE_IDLE;
            break;
    }
}

//–––––– STATE HANDLER FUNCTIONS ––––––

// STATE_IDLE: Wait for a scan event. If EVENT_LS, transition to left-unlock; if EVENT_RS, to right-unlock.
DES_State idle_state_handler(void) {
    switch (device_request.eventType) {
        case EVENT_LS:
            printf("[IDLE] EVENT_LS received. (person_id: %d)\n", device_request.data);
            authenticatedPersonId = device_request.data;
            break;
        case EVENT_GLU:
        	printf("[IDLE] EVENT_GLU received. Left-door unlocked!\n");
			doorState |= LEFT_UNLOCKED_FLAG;
			return STATE_LEFT_UNLOCKED;
			if (authenticatedPersonId <= 0)
				printf("-- WARNING! LOCK OVERRIDE -- No `person_id` provided.");
        	break;
        case EVENT_RS:
            printf("[IDLE] EVENT_RS received. (person_id: %d)\n", device_request.data);
            authenticatedPersonId = device_request.data;
            break;
        case EVENT_GRU:
			printf("[IDLE] EVENT_GRU received. Right-door unlocked!\n");
			doorState |= RIGHT_UNLOCKED_FLAG;
			return STATE_RIGHT_UNLOCKED;
			if (authenticatedPersonId <= 0)
				printf("-- WARNING! LOCK OVERRIDE -- No `person_id` provided.");
        	break;
        default:
        	break;
    }

//    printf("[IDLE] Current door state: 0x%X\n", doorState);
    return STATE_IDLE;
}

// STATE_LEFT_UNLOCKED: Left scan has been processed and the left door is to be unlocked.
DES_State left_unlocked_state_handler(void) {
    switch (device_request.eventType) {
        case EVENT_LO:
			printf("[LEFT_UNLOCKED] EVENT_LO received. ");
			// Open left door
			doorState |= LEFT_OPENED_FLAG;
			printf("Left door opened; transitioning to STATE_LEFT_OPENED\n");
			return STATE_LEFT_OPENED;
        case EVENT_GLL:
            printf("[LEFT_UNLOCKED] EVENT_GLL received. ");
            // Cancel unlocking (lock left door)
            doorState &= ~LEFT_UNLOCKED_FLAG;
            printf("Left door locked; returning to STATE_IDLE\n");
            return STATE_IDLE;
        default:
        	break;
    }

    // No change in state.
    return STATE_LEFT_UNLOCKED;
}

// STATE_LEFT_OPENED: Left door is open. Await weight measurement or door closure.
DES_State left_opened_state_handler(void) {
    switch (device_request.eventType) {
        case EVENT_WS:
            printf("[LEFT_OPENED] EVENT_WS received. ");

            // Register weight measured.
            printf("Weight measured; transitioning to STATE_WEIGHT_MEASURED\n");
            return STATE_WEIGHT_MEASURED;
        case EVENT_LC:
            printf("[LEFT_OPENED] EVENT_LC received. \n");
            // Close Left Door.
            doorState &= ~LEFT_OPENED_FLAG;
            printf("Left door closed; transitioning to STATE_LEFT_SECURED");
            return STATE_LEFT_SECURED;
        default:
        	break;
    }

	// No change in state.
    return STATE_LEFT_OPENED;
}

// STATE_WEIGHT_MEASURED: Weight has been recorded. Determine which door was active.
DES_State weight_measured_state_handler(void) {

	switch (device_request.eventType) {
		case EVENT_LC:
			printf("[WEIGHT_MEASURED] EVENT_LC received. ");

			// Check if Left-door is open.
			if (doorState & LEFT_OPENED_FLAG) {
				printf("Closing Left-door.\n");
				// Close Left Door.
				doorState &= ~(LEFT_OPENED_FLAG);
			} else {
				printf("ERROR-STATE: NO DOOR TO CLOSE; Left-door is not open!\n");
			}
			break;
		case EVENT_RC:
			printf("[WEIGHT_MEASURED] EVENT_RC received. ");

			// Check if Right-door is open.
			if (doorState & RIGHT_OPENED_FLAG) {
				printf("Closing Right-door.\n");
				// Close Right Door.
				doorState &= ~(RIGHT_OPENED_FLAG);
			} else {
				printf("ERROR-STATE: NO DOOR TO CLOSE; Right-door is not open!\n");
			}
			break;
		case EVENT_GLL:
			printf("[WEIGHT_MEASURED] GLL received.\n");

			// Check if Left-door is closed.
			if (doorState & ~LEFT_OPENED_FLAG) {
				// Lock left door.
				doorState &= ~LEFT_UNLOCKED_FLAG;
				printf("Left-door unlocked; transitioning to STATE_LEFT_SECURED\n");
				return STATE_LEFT_SECURED;
			}
			break;
		case EVENT_GRL:
			printf("[WEIGHT_MEASURED] GRL received.\n");

			// Check if Right-door is closed.
			if (doorState & ~RIGHT_OPENED_FLAG) {
				// Lock right door.
				doorState &= ~RIGHT_UNLOCKED_FLAG;
				printf("Right-door unlocked; transitioning to STATE_RIGHT_SECURED\n");
				return STATE_RIGHT_SECURED;
			}
			break;
		default:
			break;
	}

	// No change in state.
	return STATE_WEIGHT_MEASURED;
}

// STATE_LEFT_SECURED: Left door is now closed and locked.
DES_State left_secured_state_handler(void) {
    if (device_request.eventType == EVENT_GRU) {
    	printf("[LEFT_SECURED] EVENT_GRU received. ");

        // Unlock right door event; enforce airlock.
        doorState |= RIGHT_UNLOCKED_FLAG;
        doorState &= ~(LEFT_UNLOCKED_FLAG | LEFT_OPENED_FLAG);
        printf("Right-door unlocked; transitioning to STATE_RIGHT_UNLOCKED\n");
        return STATE_RIGHT_UNLOCKED;
    }

	// Return to Idle state.
	printf("[LEFT_SECURED] Remaining locked; cycle complete, returning to STATE_IDLE\n");
    return STATE_IDLE;
}

// STATE_RIGHT_UNLOCKED: Right scan has been processed and the right door is to be unlocked.
DES_State right_unlocked_state_handler(void) {
    switch (device_request.eventType) {
        case EVENT_RO:
        	printf("[RIGHT_UNLOCKED] EVENT_RO received. ");

            doorState |= RIGHT_OPENED_FLAG;
            printf("Right door opened; transitioning to STATE_RIGHT_OPENED\n");
            return STATE_RIGHT_OPENED;
        case EVENT_GRL:
        	printf("[RIGHT_UNLOCKED] EVENT_GRL received. ");

            doorState &= ~RIGHT_UNLOCKED_FLAG;
            printf("Right door locked; transitioning to STATE_IDLE\n");
            return STATE_IDLE;
        default:
        	break;
    }

	// No change in state.
    return STATE_RIGHT_UNLOCKED;
}

// STATE_RIGHT_OPENED: Right door is open. Await weight measurement or door closure.
DES_State right_opened_state_handler(void) {
    switch (device_request.eventType) {
        case EVENT_WS:
        	printf("[RIGHT_OPENED] EVENT_WS received. ");

            printf("Weight measured; transitioning to STATE_WEIGHT_MEASURED\n");
            return STATE_WEIGHT_MEASURED;
        case EVENT_RC:
        	printf("[RIGHT_OPENED] EVENT_RC received. ");

            doorState &= ~LEFT_OPENED_FLAG;
            printf("[RIGHT_OPENED] EVENT_RC: Right door closed; transitioning to STATE_RIGHT_SECURED\n");
            return STATE_RIGHT_SECURED;
        default:
        	break;
    }

	// No change in state.
    return STATE_RIGHT_OPENED;
}

// STATE_RIGHT_SECURED: Right door is now closed and locked.
DES_State right_secured_state_handler(void) {
    if (device_request.eventType == EVENT_GLU) {
    	printf("[RIGHT_SECURED] EVENT_GLU received. ");

		// Unlock Left door.
		doorState |= LEFT_UNLOCKED_FLAG;
		doorState &= ~(RIGHT_UNLOCKED_FLAG | LEFT_OPENED_FLAG);
		printf("Left door unlocked; transitioning to STATE_LEFT_UNLOCKED\n");
		return STATE_LEFT_UNLOCKED;
    }

	// Return to Idle state.
	printf("[RIGHT_SECURED] Remaining locked; cycle complete, returning to STATE_IDLE\n");
	return STATE_IDLE;
}

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
    currentDesState = STATE_IDLE;
    while (1) {
    	handleCurrentState(currentDesState); // Change states when necessary.

    	receiveMessage(); // Reads request.
    }

    // Cleanup (never reached in this infinite loop)
    ChannelDestroy(chid);
    return EXIT_SUCCESS;
}

