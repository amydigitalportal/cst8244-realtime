#include "../../des_controller/src/des.h"


int main(void) {
	// Print the Display's process ID
//    printf("des_display: running with process id %d\n\n", getpid());


	name_attach_t *attach;

	// Attach to namespace
	attach = name_attach(NULL, NAMESPACE_DISPLAY, 0);
	if (attach == NULL) {
		perror("name_attach failed!");
		exit(EXIT_FAILURE);
	}
    printf("des_display: Listening via namespace: '%s'\n", NAMESPACE_DISPLAY);


	DisplayMessage displayMessage;
    int rcvid;

    while (1) {

    	// Listen to messages from the controller
    	rcvid = MsgReceive(attach->chid, &displayMessage, sizeof(displayMessage), NULL);
		if (rcvid == -1) {
			perror("MsgReceive failed");
			continue;
		}

		switch (displayMessage.type) {
		case DISPLAY:
			printf("%s\n", displayMessage.payload);
			// Acknowledge receipt.
			MsgReply(rcvid, 0, NULL, 0); // No replies.
			break;
		case SHUTDOWN:
			printf("Received SHUTDOWN command!\n\n");
			MsgReply(rcvid, 0, NULL, 0);

			// Proceed to finalize.
			goto shutdown;
		default:
			perror("des_display: Unknown message type! Looping back to listening for messages...");
			continue;
		}
    }

shutdown:
	printf("des_display: cleaning up and shutting down... Goodbye!\n\n");
	sleep(1);
	name_detach(attach, 0);
	return EXIT_SUCCESS;
}
