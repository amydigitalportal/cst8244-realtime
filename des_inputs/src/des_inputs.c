#include <ctype.h>
#include "../../des_controller/src/des.h"

#define MAX_INPUT_SIZE 32

// Lookup table for reverse mapping
typedef struct {
    const char *name;
    EventType event;
} EventLookup;

const EventLookup eventTable[] = {
    { "LS", EVENT_LS }, { "GLU", EVENT_GLU }, { "GLL", EVENT_GLL }, { "LO", EVENT_LO },
    { "LC", EVENT_LC }, { "RS", EVENT_RS }, { "GRU", EVENT_GRU }, { "GRL", EVENT_GRL },
    { "RO", EVENT_RO }, { "RC", EVENT_RC }, { "WS", EVENT_WS }, { "EXIT", EVENT_EXIT },
    { NULL, -1 }  // End marker
};



name_attach_t* attach;
int coid;
DES_Message device_request;


/**
 * Helper function to handle input.
 */
void getUserInput(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline
    }
}

// Function to convert string to EventType
EventType getEventType(const char *input) {
    for (int i = 0; eventTable[i].name != NULL; i++) {
    	// Convert input to upper-case then compare it to the lookup table.
        if (strcmp(strupr(input), eventTable[i].name) == 0) {
            return eventTable[i].event;
        }
    }
    return EVENT_UNKNOWN;  // If not found
}

// Blocking function to send a DES_Message to the controller.
void sendMessage() {

    if (device_request.eventType == EVENT_EXIT) {
    	printf("des_inputs: Exiting process...\n");
    }

//    printf("[DEBUG] sizeof(DES_Message) = %zu\n", sizeof(DES_Message));

    // Send message to the controller with request data.
    int send_response = MsgSend(coid, &device_request, sizeof(device_request), NULL, 0 );
    if (send_response == -1) {
        perror("des_inputs: MsgSend failed");
		name_close(coid);
    }
}

void simulate_event(EventType eventType, int data) {
	device_request.eventType = eventType;
	device_request.data = data;

	sendMessage();
}

void processEventType(EventType eventType) {
    switch (eventType) {
    case EVENT_LS:
    case EVENT_RS: {
    	char inputBuffer[30];
    	printf("Activating scanner! Please enter `person_id` (%d to %d): \n", VALID_PERSON_ID_MIN, VALID_PERSON_ID_MAX);
		getUserInput(inputBuffer, sizeof(inputBuffer));
		int personId = atoi(inputBuffer);
		simulate_event(eventType, personId);
    	break;
    }
    case EVENT_WS: {
    	char inputBuffer[30];
    	printf("Activating weight scale! Please enter person's `weight` (no decimals): \n");
		getUserInput(inputBuffer, sizeof(inputBuffer));
		int weight = atoi(inputBuffer);
		simulate_event(eventType, weight);
    	break;
    }
    default:
    	simulate_event(eventType, -1);
    	break;
    }
}

int main() {

	coid = name_open(NAMESPACE_CONTROLLER, 0);
    if (coid == -1) {
        perror("des_inputs: name_open failed to connect to controller. Process terminating...\n\n");
        exit(EXIT_FAILURE);
    }

	char inputBuffer[30];

	printf("\n-- Enter the event type (ls= left scan, rs= right scan, ws= weight scale, lo =left open, "
			"ro=right open, lc = left closed, rc = right closed , gru = guard right unlock, grl = guard, "
			"right lock, gll=guard left lock, glu = guard left unlock)\n\n");

	while (1) {

		getUserInput(inputBuffer, sizeof(inputBuffer));

        // Compare and process commands
        EventType eventType = getEventType(inputBuffer); // Convert string to upper case.

        processEventType(eventType);

        if (eventType == EVENT_EXIT)
        	break;
	}


    name_close(coid);

	return EXIT_SUCCESS;
}
