/*
 ** sigint.c -- grabs SIGINT
 *
 * Read:	http://beej.us/guide/bgipc/html/single/bgipc.html#signals
 * Source:	http://beej.us/guide/bgipc/examples/sigint.c
 *
 * Modified by: hurdleg@algonquincollege.com
 *
 * Usage:
 *  From Momentics IDE, run program; notice PID; enter some text, but don't hit the enter key
 *  At Neutrino prompt, issue the command: kill -s SIGINT <PID>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

void sigint_handler(int sig);

volatile sig_atomic_t usr1Happened = 0;

/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(void) {
	char s[140];                // A character array (string) to store user input
	struct sigaction sa;        // Declare a `sigaction` structure to configure the signal handler

	// Set the signal handler function for SIGINT
	sa.sa_handler = sigint_handler; // Assign the handler function `sigint_handler` to handle SIGINT

	// Set flags for signal handling behavior
	sa.sa_flags = 0; // Basic handling. You can use SA_RESTART here to automatically restart interrupted system calls.

	// Clear the signal mask (no signals are blocked while the handler runs)
	sigemptyset(&sa.sa_mask);

	// Install the SIGINT handler using sigaction
	// If sigaction fails ...
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		// Print an error message and exit the program
		perror("sigaction"); // Print the reason for failure
		exit(1);             // Exit with a non-zero status to indicate an error
	}

	// Print the current process ID (PID) so you can interact with the program using signals if needed
	printf("My PID is %d\n", getpid());

	// Prompt the user to enter a string
	printf("Enter a string:  ");

	// Read a string from the user using `fgets`
	if (fgets(s, sizeof(s), stdin) == NULL) {
		// If `fgets` fails (e.g., due to an interrupted system call), print an error message
		perror("fgets");
	} else {
		// Otherwise, print the string entered by the user
		printf("You entered: %s\n", s);
	}

	return 0; // Return 0 to indicate the program executed successfully
}

// Signal handler function to handle SIGINT
void sigint_handler(int sig) {
	usrHappened1 = 1;
}
