#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <errno.h>

#include "../calc_message.h"

// Status codes
#define SRVR_OK 0
#define SRVR_ERR_DIV_BY_ZERO -1
#define SRVR_ERR_INVALID_OPERATOR -2

int main(void) {
	int chid, rcvid;
	client_send_t client_message;
	server_response_t response;

	// Create communication channel.
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("ChannelCreate failed");
		exit(EXIT_FAILURE);
	}

	printf("Server started. Channel ID: %d\n", chid);

	// Begin Server Loop
	while (1) {
		// Receive message from client
		rcvid = MsgReceive(chid, &client_message, sizeof(client_message), NULL);
		if (rcvid == -1) {
			perror("MsgReceive failed");
			continue;
		}

		// Default response status
		response.statusCode = SRVR_OK;
		response.answer = 0;
		snprintf(response.errorMsg, sizeof(response.errorMsg), "No error");

		double lhs = (double) client_message.left_hand;
		double rhs = (double) client_message.right_hand;

		// Decide on operation based on client_message.operator
		switch (client_message.operator) {
		case '+':
			response.answer = lhs + rhs;
			break;
		case '-':
			response.answer = lhs - rhs;
			break;
		case '*':
			response.answer = lhs * rhs;
			break;
		case '/':
			if (rhs != 0) {
				response.answer = lhs / rhs;
			} else {
				response.statusCode = SRVR_UNDEFINED;
				snprintf(response.errorMsg, sizeof(response.errorMsg),
						"Division by zero");
			}
			break;
		default:
			response.statusCode = SRVR_INVALID_OPERATOR;
			snprintf(response.errorMsg, sizeof(response.errorMsg),
					"Invalid operator: %c", client_message.operator);
			break;
		}

		// Reply to the Client
		int replyStatus = MsgReply(rcvid, EOK, &response,
				sizeof(server_response_t));
		if (replyStatus == -1) {
			perror("MsgReply failed");
		}
	}

	// Cleanup for future loop exit mechanism.
//		ChannelDestroy(chid);
	return EXIT_SUCCESS;
}
