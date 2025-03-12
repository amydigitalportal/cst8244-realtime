#include "../../des_controller/src/des.h"


int coid;
DES_Message device_request;

// Blocking function to send a DES_Message to the controller.
void sendMessage() {
    if (coid == -1) {
        fprintf(stderr, "Invalid connection ID\n");
        return;
    }

    // Send message to the controller with request data.
    int send_response = MsgSend(
        coid,
        &device_request, sizeof(device_request),
        NULL, 0 // No need to receive a response
    );

    if (send_response == -1) {
        perror("MsgSend failed");
        if (coid != -1) {
            ConnectDetach(coid); // Safely detach only if valid
        }
    }
}

void simulate_event(EventType eventType, int data) {
	device_request.eventType = eventType;
	device_request.data = data;

	sendMessage();
}

int main(int argc, char *argv[]) {

	// Validate number of command arguments.
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <controller-pid> <channel-id>",
				argv[0]);
		exit(EXIT_FAILURE);
	}

	// Parse command-line args.
	pid_t controller_pid = atoi(argv[1]);
	int chid = atoi(argv[2]); // QNX Target Channel ID


	// Attempt to connect to the DES Controller process.
	coid = ConnectAttach(0, controller_pid, chid, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		perror("ConnectAttach failed");
		return EXIT_FAILURE;
	}

	EventType automated_inputs[] = {
			EVENT_LS,
			EVENT_GLU,
			EVENT_LO,
			EVENT_WS,
			EVENT_LC,
			EVENT_GLL,
			EVENT_GRU,
			EVENT_RO,
			EVENT_RC,
			EVENT_GRL,
			EVENT_GRL
	};
	const size_t seqlen = sizeof(automated_inputs) / sizeof(automated_inputs[0]);

	for (int i = 0; i < seqlen; i++) {
		simulate_event(automated_inputs[i], -1);
		sleep(2);
	}

	// Safely detach from connection.
	ConnectDetach(coid);
	return EXIT_SUCCESS;
}
