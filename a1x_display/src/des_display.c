#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <errno.h>
#include "des-mva.h"

// Message received from controller
typedef struct {
    int output;  // holds a value from Output enum
} output_msg_t;

int main() {
    // Create a channel to receive output messages from the controller.
    // For this example, we assume that the controller will attach using channel number 1.
    int chid = ChannelCreate(0);
    if(chid == -1) {
        perror("ChannelCreate failed");
        exit(EXIT_FAILURE);
    }

    printf("des_display PID: %d, waiting for output messages on channel %d\n", getpid(), chid);

    // In a production system, the display's channel number (here assumed to be 1)
    // would be communicated to the controller (or fixed by convention).

    output_msg_t msg;
    int rcvid;

    while (1) {
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if(rcvid == -1) {
            perror("MsgReceive failed");
            break;
        }

        // Print the status message based on the Output code received.
        if (msg.output >= 0 && msg.output < NUM_OUTPUTS) {
            printf("des_display: %s\n", outMessage[msg.output]);
        } else {
            printf("des_display: Received unknown output code: %d\n", msg.output);
        }

        // If the STOP_MSG is received, exit the loop.
        if(msg.output == STOP_MSG)
            break;

        // Reply to the controller’s MsgSend.
        MsgReply(rcvid, 0, NULL, 0);
    }

    ChannelDestroy(chid);
    printf("des_display: Exiting.\n");
    return 0;
}
