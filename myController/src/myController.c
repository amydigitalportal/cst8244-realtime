#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include "../../Lab7Common/lab7.h"

name_attach_t* attach;
my_message_t msg;

// my prototypes for helper functinos
int read_device_status(char *out_status, size_t status_size, char *out_value, size_t value_size);
int handle_device_status(const char *status, const char *value);




/**
 * Helper function to perform general cleanup and a graceful shutdown when an error occurs.
 */
void cleanExitFailure() {
	name_detach(attach, 0);
	exit(EXIT_FAILURE);
}

/**
 * Helper function to perfom general cleanup on succesful finish of server operation
 */
void cleanExitSuccess() {
	name_detach(attach, 0);
	exit(EXIT_SUCCESS);
}

/**
 * Custom helper function to read the status of a device (for code reusability).
 * This function will handle opening and closing of the file descriptor pinter.
 *
 * Returns:
 * 		0 - successful read of the deviec
 * 		1 - failure to either 1) open device or 2) correctly parse status AND value
 */
int read_device_status(char *out_status, size_t status_size, char *out_value, size_t value_size) {
	FILE *fd = fopen(DEVICE_PATH, "r"); // Attempt to open file to the device path.
	if (!fd) {
		perror("Failed to open device");
		return -1;
	}

	int numScannedStrings = fscanf(fd, "%s %s", out_status, out_value);
	fclose(fd);

	if (numScannedStrings != 2) {
		fprintf(stderr, "Failed to parse status and value from device! (received '%d' strings)\n", numScannedStrings);
		return -1;
	}

	return 0;
}


/**
 * Handler for checking whether a device is closed based on provided character
 * values (which are assumed to have been parsed from a device into a file descriptor.)
 *
 * Returns:
 * 		1 - device has been closed
 * 		0 - device status unreadable or not yet closed
 */
int handle_device_status(const char *status, const char *value) {
	// Ensure the status exists
	if (strcmp(status, "status") == 0) {
		// Print the value of the status
		printf("Status: %s\n", value);

		// Check if the value is 'closed'.
		if (strcmp(value, "closed") == 0) {
			return 1;
		}
	}

	return 0;
}





////////////////////
/// --- MAIN --- ///



int main(void) {
	char status[DEV_STATUS_BUFSIZE], value[DEV_STATUS_BUFSIZE]; // file-scan buffers for the device status
	int rcvid; // ID of message received

	// Configure myController as a server; Register the device within the namespace.
	attach = name_attach( NULL, DEVICE_NAME, 0 );
	if (attach == NULL) {
		perror("myController: name_attach failed! Process terminating ...");
		exit(EXIT_FAILURE);
	}

	printf ("myController: Listening via namespace '%s' ...\n", DEVICE_NAME);

	// -- Inital startup check.

	// Read the status
	if (read_device_status(status, sizeof(status), value, sizeof(value)) == 0) {
		printf("Startup device check - ");

		// check whether device is closed
		if (handle_device_status(status, value)) {
			// Device is already closed, so we can simply shutdown sevrer
			cleanExitSuccess();
		}
	}


	// -- BEGIN: Server loop
	while (1) {

		// Listen for a pulse
		rcvid = MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL);

		// Safeguard against bad reception.
		if (rcvid != 0) {
			fprintf(stderr, "myController: MsgReceivePulse entountered unexpected error! rcvid: %d\n", rcvid);
			// Terminate gracefully.
			cleanExitFailure();
		}

		// Check message's pulse code for a match on our pulse code.
		if (msg.pulse.code == MY_PULSE_CODE) {
			// Print the small integer from the pulse
			printf("Small integer: %d\n", msg.pulse.value.sival_int);

			// Open the device (at the device path)
			if (read_device_status(status, sizeof(status), value, sizeof(value)) == 0) {
				// Handle the device status
				if (handle_device_status(status, value)) {
					// device closed - ready to shutdown
					cleanExitSuccess();
				}
			}
		}
	}

	cleanExitSuccess(); // handles the clenaup
	return EXIT_SUCCESS; // unreachable
}
