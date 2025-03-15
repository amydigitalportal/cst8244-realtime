#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include "des-mva.h"

// Global variables
State currentState = START_STATE;
int punch_count = 0;
int chid;  // Channel ID for input reception
pid_t display_pid;  // Global display process PID

// Message structures
typedef struct {
    int input;  // Input enum value
} input_msg_t;

typedef struct {
    int output; // Output enum value
} output_msg_t;

// Function prototypes for state handling
State handle_start();
State handle_ready();
State handle_left_down();
State handle_right_down();
State handle_armed();
State handle_punch();
State handle_exit();
State handle_stop();

// Send output message to display process
void send_output_msg(Output output) {
    int coid = ConnectAttach(0, display_pid, 1, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        perror("ConnectAttach to des_display failed");
        return;
    }

    output_msg_t out_msg;
    out_msg.output = output;

    if (MsgSend(coid, &out_msg, sizeof(out_msg), NULL, 0) == -1) {
        perror("MsgSend to des_display failed");
    }

    ConnectDetach(coid);
}

// Receives input message when required
Input receive_input() {
    input_msg_t in_msg;
    int rcvid = MsgReceive(chid, &in_msg, sizeof(in_msg), NULL);
    if (rcvid == -1) {
        perror("MsgReceive failed");
        return STOP_BUTTON;  // Default to STOP_BUTTON on failure
    }
    MsgReply(rcvid, 0, NULL, 0);
    return (Input) in_msg.input;
}

// Main FSM processing loop
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <display_pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    display_pid = atoi(argv[1]);  // Assign global display PID

    // Create channel for receiving inputs
    chid = ChannelCreate(0);
    if (chid == -1) {
        perror("ChannelCreate failed");
        exit(EXIT_FAILURE);
    }

    printf("des_controller PID: %d, receiving inputs on channel %d\n", getpid(), chid);

    while (currentState != STOP_STATE) {
        switch (currentState) {
            case START_STATE: currentState = handle_start(); break;
            case READY_STATE: currentState = handle_ready(); break;
            case LEFT_DOWN_STATE: currentState = handle_left_down(); break;
            case RIGHT_DOWN_STATE: currentState = handle_right_down(); break;
            case ARMED_STATE: currentState = handle_armed(); break;
            case PUNCH_STATE: currentState = handle_punch(); break;
            case EXIT_STATE: currentState = handle_exit(); break;
            case STOP_STATE: currentState = handle_stop(); break;
            default: break;
        }
    }

    ChannelDestroy(chid);
    printf("des_controller: Stopping.\n");
    return 0;
}

// State Handlers

State handle_start() {
    send_output_msg(START_MSG);
    return READY_STATE;
}

State handle_ready() {
    send_output_msg(READY_MSG);

    Input input = receive_input();
    if (input == LEFT_BUTTON_DOWN)
        return LEFT_DOWN_STATE;
    if (input == RIGHT_BUTTON_DOWN)
        return RIGHT_DOWN_STATE;

    return READY_STATE;
}

State handle_left_down() {
    send_output_msg(LEFT_DOWN_MSG);

    Input input = receive_input();
    if (input == RIGHT_BUTTON_DOWN)
        return ARMED_STATE;
    if (input == LEFT_BUTTON_UP)
        return READY_STATE;

    return LEFT_DOWN_STATE;
}

State handle_right_down() {
    send_output_msg(RIGHT_DOWN_MSG);

    Input input = receive_input();
    if (input == LEFT_BUTTON_DOWN)
        return ARMED_STATE;
    if (input == RIGHT_BUTTON_UP)
        return READY_STATE;

    return RIGHT_DOWN_STATE;
}

State handle_armed() {
    send_output_msg(ARMED_MSG);

    Input input = receive_input();
    if (input == LEFT_BUTTON_UP || input == RIGHT_BUTTON_UP)
        return PUNCH_STATE;

    return ARMED_STATE;
}

State handle_punch() {
    send_output_msg(PUNCH_MSG);

    for (punch_count = 1; punch_count <= 5; punch_count++) {
        printf("Punch #%d\n", punch_count);
        sleep(1);
    }

    punch_count = 0;
    return EXIT_STATE;
}

State handle_exit() {
    send_output_msg(EXIT_MSG);
    return STOP_STATE;
}

State handle_stop() {
    send_output_msg(STOP_MSG);
    return STOP_STATE;
}
