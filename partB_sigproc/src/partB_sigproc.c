#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void sigint_handler(int sig);
volatile sig_atomic_t usr1Happened = 0;


// Creates a formatted PID string
static char* pid_formatted() {
	char* buffer = malloc(16);
    if (buffer != NULL) {
        snprintf(buffer, 16, "PID = %d", getpid());
    }
    return buffer;
}

// Function to print message with current PID.
static void print_pid_msg(char* msg) {
	char* pid_str = pid_formatted();
	if (pid_str != NULL)
	{
		printf("%s : %s\n", pid_str, msg);
	}
	free(pid_str);
}


// Signal handler function to handle SIGINT
void sigint_handler(int sig) {
	usr1Happened = 1;
	print_pid_msg("Child received USR1.");
}

/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(void) {

	// setup() ------------

	struct sigaction sa;        // Declare a `sigaction` structure to configure the signal handler

	// Set the signal handler function for SIGINT
	sa.sa_handler = sigint_handler; // Assign the handler function `sigint_handler` to handle SIGINT

	// Set flags for signal handling behavior
	sa.sa_flags = 0; // Basic handling. You can use SA_RESTART here to automatically restart interrupted system calls.

	// Clear the signal mask (no signals are blocked while the handler runs)
	sigemptyset(&sa.sa_mask);

	// Use sigaction to register the SIGUSR1 signal type.
	// If sigaction fails ...
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		// Print an error message and exit the program
		perror("sigaction"); // Print the reason for failure
		exit(1);             // Exit with a non-zero status to indicate an error
	}

	printf("//run program from Momentics IDE\n", getpid());

	int num_children;
	char input[100];  // Buffer for user input

	// program() ----------------

	while (1) {
		// Prompt user to enter a number.
		printf("Enter the number of children:\n");

		// Read input from the user.
		if (fgets(input, sizeof(input), stdin) == NULL) {
			printf("Error reading input. Please try again.\n");
			continue;
		}

		// Attempt to convert input to an integer.
		if (sscanf(input, "%d", &num_children) != 1 || num_children < 0) {
			printf("-- ERROR: Invalid input! Please enter a non-negative integer.\n");
			continue;  // Ask again.
		}

		break;  // Valid input, exit loop.
	}

	// Display parent PID.
	printf("\n");
	print_pid_msg("Parent running...");

    // For specified number of children...
    for (int i = 0; i < num_children; i++) {
    	// ... Fork process to spawn children.
        pid_t pid = fork();

        // Check for failed fork.
        if (pid < 0) {
        	printf("Fork failed at iteration: '%d'. Exiting process.\n", i);
            perror("Fork failed");
            exit(1);
        }
        // Otherwise...
        if (pid == 0) { // Child process

        	// Print child PID.
        	print_pid_msg("Child running...");

        	while (!usr1Happened) {
        		// Loop until USR1 signal handler has performed.
        	}

        	// Report preparation to exit.
        	print_pid_msg("Child exiting.");
        	exit(EXIT_SUCCESS);
        }
    }

	// Parent: Waiting for all children
	int finished_children = 0;
	while (finished_children < num_children) {
		pid_t child_pid = wait(NULL);  // Wait for any child

		if (child_pid > 0) { // Child process returned its PID.
			// Count the number of children finished
			finished_children++;
		}
	}

	// Report parent process done.
	print_pid_msg("Children finished, parent exiting.");
	exit(EXIT_SUCCESS);
}
