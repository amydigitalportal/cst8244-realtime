#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

void sigint_handler(int sig);
volatile sig_atomic_t usr1Happened = 0;

// Creates a formatted PID string
static char* pid_formatted() {
	char *buffer = malloc(16);
	if (buffer != NULL) {
		snprintf(buffer, 16, "PID = %d", getpid());
	}
	return buffer;
}

// Function to print message with current PID.
static void print_pid_msg(char *msg) {
	char *pid_str = pid_formatted();
	if (pid_str != NULL) {
		printf("%s : %s\n", pid_str, msg);
	}
	free(pid_str);
}

// Signal handler function to handle SIGINT
void sigint_handler(int sig) {
	usr1Happened = 1;
	print_pid_msg("Received USR1.");
}

/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(void) {

	struct sigaction sa; // Declare a `sigaction` structure to configure the signal handler

	// Set the signal handler function for SIGINT
	sa.sa_handler = sigint_handler; // Assign the handler function `sigint_handler` to handle SIGINT

	// Set flags for signal handling behavior
	sa.sa_flags = 0; // Basic handling. You can use SA_RESTART here to automatically restart interrupted system calls.

	// Clear the signal mask (no signals are blocked while the handler runs)
	sigemptyset(&sa.sa_mask);

	// -- Use sigaction to register the SIGUSR1 signal type.

	// If sigaction fails ...
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		// Print an error message and exit the program
		perror("sigaction"); // Print the reason for failure
		exit(1);             // Exit with a non-zero status to indicate an error
	}

	// Print the current process ID (PID) so you can interact with the program using signals if needed
	print_pid_msg("Running ...");

	// Loop until signal has been received
	while (!usr1Happened) {
	}

	print_pid_msg("Exiting.");
	return 0; // Return 0 to indicate the program executed successfully
}
