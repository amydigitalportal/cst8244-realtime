#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/dispatch.h>
#include <sys/types.h>
#include <errno.h>

#include "../../a2_common/a2.h"


char data[DEV_STATUS_BUFSIZE];
my_message_t msg;

metronome_config_t current_config = {};
metronome_config_t next_config = {};
volatile bool next_config_pending = false;

name_attach_t* attach;

static const rhythm_pattern_t rhythm_table[] = {
	{2, 4, 4,  {"|1", "&", "2", "&"}},
	{3, 4, 6,  {"|1", "&", "2", "&", "3", "&"}},
	{4, 4, 8,  {"|1", "&", "2", "&", "3", "&", "4", "&"}},
	{5, 4, 10, {"|1", "&", "2", "&", "3", "&", "4", "-", "5", "-"}},
	{3, 8, 6,  {"|1", "-", "2", "-", "3", "-"}},
	{6, 8, 6,  {"|1", "&", "a", "2", "&", "a"}},
	{9, 8, 9,  {"|1", "&", "a", "2", "&", "a", "3", "&", "a"}},
	{12,8, 12, {"|1", "&", "a", "2", "&", "a", "3", "&", "a", "4", "&", "a"}}
};






/**
 * Helper function to perform general cleanup and a graceful shutdown when an error occurs.
 */
void cleanExitFailure() {
	printf("-- Cannot proceed due to error! Metronome process shutting down.\n");
	name_detach(attach, 0);
	exit(EXIT_FAILURE);
}

/**
 * Helper function to perfom general cleanup on succesful finish of server operation
 */
void cleanExitSuccess() {
	printf("-- Metronome driver finished; shutting down. Goodbye!\n");
	name_detach(attach, 0);
	exit(EXIT_SUCCESS);
}


/**
 * Helper method for fetching the pattern matching the specified top and bottom time-sig values.
 */
const rhythm_pattern_t* get_rhythmPattern(int top, int bottom) {
	// Iterable through table...
	for (int i = 0; i < sizeof(rhythm_table)/sizeof(rhythm_table[0]); i++) {
		// Check if inputs match the table entry
		if (rhythm_table[i].top == top && rhythm_table[i].bottom == bottom) {
			return &rhythm_table[i];
		}
	}

	// No pattern matching args ...
	return NULL;
}

void handle_message(my_message_t *msg) {
	switch (msg->pulse.code) {
//		// Print the small integer from the pulse
//		printf("Small integer: %d\n", msg->pulse.value.sival_int);
//
//		// Open the device (at the device path)
//		if (read_device_status(status, sizeof(status), value, sizeof(value)) == 0) {
//			// Handle the device status
//			if (handle_device_status(status, value)) {
//				// device closed - ready to shutdown
//				cleanExitSuccess();
//			}
//		}

	case METRONOME_PULSE_CODE:
		break;

	case PAUSE_PULSE_CODE:
		break;

	case QUIT_PULSE_CODE:
		break;

	case SET_CONFIG_PULSE_CODE:
		break;

	default:
		break;
	}
}



int main(void) {

	// -- PHASE I: create a named channel to receive pulses
	attach = name_attach( NULL, DEVICE_NAME, 0 );

	// Calculate the seconds-per-beat and nano seconds for the interval timer
	long sec_per_beat, sec_per_measure, nanos = 0;
	sec_per_beat = 60 / current_config.bpm;

//	sec_per_measure =


	// Create an interval timer to "drive" the metronome
	struct sigevent event; // signal emitted on timer tick
	timer_t timer_id;
	struct itimerspec itime; // timer specifications


	// Check whether a new config has been set
	if (next_config_pending) {
		// Update the current config
		current_config = next_config;
		next_config_pending = false; // reset the flag
	}

	// -- BEGIN: Server loop
	bool is_process_looping = true;
	while (is_process_looping) {

		// Listen for a pulse
		rcvid = MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL);

		// Safeguard against bad reception.
		if (rcvid != 0) {
			fprintf(stderr, "myController: MsgReceivePulse entountered unexpected error! rcvid: %d\n", rcvid);
			// Terminate gracefully.
			cleanExitFailure();
		}

		// Process the message.
		handle_message(&msg);

	}

	return EXIT_SUCCESS;
}

