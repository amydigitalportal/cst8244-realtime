#include "../../des_controller/src/des.h"


int main(void) {
	// Print the Display's process ID
    printf("des_display: running with process id %d\n\n", getpid());

    // Open channel for IPC.
    int chid = ChannelCreate(0);
    if(chid == -1) {
        perror("ChannelCreate failed");
        exit(EXIT_FAILURE);
    }

    printf("des_display PID: %d, waiting for output messages on channel %d\n", getpid(), chid);

	DisplayMessage displayMessage;
    int rcvid;

    while (1) {

    	// Listen to messages from the controller
    	rcvid = MsgReceive(chid, &displayMessage, sizeof(displayMessage), NULL);
		if (rcvid == -1) {
			perror("MsgReceive failed");
			continue;
		}

		printf("%s\n", displayMessage.payload);

		// Acknowledge receipt.
		MsgReply(rcvid, 0, NULL, 0); // No replies.
    }

    ChannelDestroy(chid);
    return EXIT_SUCCESS;
}
