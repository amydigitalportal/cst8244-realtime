#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <errno.h>

#include "../../Lab5Server/calc_message.h"

int main(int argc, char *argv[]) {

	int coid; // Connection ID
	client_send_t msg_send;
	server_response_t msg_receive;

	// Validate the number of command-line args.
	if (argc != 5) {
		fprintf(stderr, "Usage: %s <server-pid> <left> <operator> <right>", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Parse command-line args.
	pid_t server_pid = atoi(argv[1]);
	msg_send.left_hand 	= atoi(argv[2]);
	msg_send.operator 	= argv[3][0]; // Access first character of the argument string
	msg_send.right_hand = atoi(argv[4]);

	// Connect to the server channel.
	coid = ConnectAttach(0, server_pid, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		perror("ConnectAttach failed");
		exit(EXIT_FAILURE);
	}

	// Send message to the server and receive the response.
	int send_response = MsgSend(coid, &msg_send, sizeof(msg_send), &msg_receive, sizeof(msg_receive));
	if (send_response == -1) {
		perror("MsgSend failed");
		ConnectDetach(coid); // Safely detach from channel.
	}

	// Process server's response.
	if (msg_receive.statusCode == SRVR_OK) {
		printf("The server has calculated the result: %d %c %d = %f\n",
				msg_send.left_hand,
				msg_send.operator,
				msg_send.right_hand,
				msg_receive.answer
		);
	} else {
		fprintf(stderr, "Error: %s (Code: %d)\n", msg_receive.errorMsg, msg_receive.statusCode);
	}

	// CLIENT SCRIPT FINISHED!

	// Cleanup: Safely detach from the server's channel.
	ConnectDetach(coid);
	return EXIT_SUCCESS;
}
