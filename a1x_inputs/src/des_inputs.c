#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <errno.h>
#include "des-mva.h"

// Message sent to controller
typedef struct {
    int input;  // holds a value from Input enum
} input_msg_t;

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <controller_pid> <controller_channel>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t controller_pid = atoi(argv[1]);
    int controller_chid = atoi(argv[2]);  // The channel ID printed by des_controller

    printf("des_inputs PID: %d, sending inputs to controller PID: %d on channel %d\n",
           getpid(), controller_pid, controller_chid);

    // For simulation, we send a series of inputs.
    // (In a real system these might come from a user or sensors.)
    Input inputs_to_send[] = {
    		LEFT_BUTTON_DOWN,
			RIGHT_BUTTON_DOWN,
			RIGHT_BUTTON_UP,
			LEFT_BUTTON_UP,
            STOP_BUTTON
    };
    int num_inputs = sizeof(inputs_to_send) / sizeof(Input);

	// Attach to the controller's input channel.
	int coid = ConnectAttach(0, controller_pid, controller_chid, _NTO_SIDE_CHANNEL, 0);
	if(coid == -1) {
		perror("ConnectAttach failed");
		exit(EXIT_FAILURE);
	}
    for(int i = 0; i < num_inputs; i++) {

        input_msg_t msg;
        msg.input = inputs_to_send[i];

        printf("des_inputs: Sending input %s\n", inMessage[inputs_to_send[i]]);

        // Send the input message and wait for reply.
        if(MsgSend(coid, &msg, sizeof(msg), NULL, 0) == -1) {
            perror("MsgSend failed");
        }

        sleep(1);  // pause between inputs
    }
	ConnectDetach(coid);

    printf("des_inputs: Finished sending inputs.\n");
    return 0;
}
