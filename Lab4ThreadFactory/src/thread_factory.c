#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#define MAX_THREADS 100

sem_t semaphore;
volatile sig_atomic_t running = 1;

void handle_sigusr1(int signo) {
	if (signo == SIGUSR1) {
		printf("\nReceived SIGUSR1, terminating program...\n");
		running = 0;
		sem_post(&semaphore); // Unblock any waiting threads
	}
}

void* thread_subroutine(void* arg) {
	int thread_id = *(int*)arg;
	free(arg); // Free dynamic allocated mem
	printf("Thread %d created.\n", thread_id);

	while (running) {
		sem_wait(&semaphore); // wait for semaphore

		if (!running) // if program is terminating.
			break;

		printf("Thread %d unblocked.\n", thread_id);

		// Do some work
		int num_seconds = 5;
		printf("Sleeping for %d seconds...\n", num_seconds);
		sleep(num_seconds);
	}

	printf("Thread %d exiting...\n", thread_id);
	return NULL;
}

int main(void) {
	// setup() ------------

	struct sigaction sa; // Declare a `sigaction` structure to configure the signal handler

	// Set the signal handler function for SIGINT
	sa.sa_handler = handle_sigusr1; // Assign the handler function `sigint_handler` to handle SIGINT

	// Set flags for signal handling behavior
	sa.sa_flags = 0; // Basic handling.

	// Clear the signal mask (no signals are blocked while the handler runs)
	sigemptyset(&sa.sa_mask);

	// Use sigaction to register the SIGUSR1 signal type.
	// If sigaction fails ...
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		// Print an error message and exit the program
		perror("sigaction"); // Print the reason for failure
		exit(1);             // Exit with a non-zero status to indicate an error
	}


	// Initialize the semaphore
	sem_init(&semaphore, 0, 0);

	// Declare threads
	pthread_t threads[MAX_THREADS];
	int num_threads = 0;

	// Prepare for input
	char input[100];  // Buffer for user input

	// program() ----------------

	while (1) {
		// Prompt user to enter a number.
		printf("Enter the number of child threads:\n");

		// Read input from the user.
		if (fgets(input, sizeof(input), stdin) == NULL) {
			printf("Error reading input. Please try again.\n");
			continue;
		}

		// Attempt to convert input to an integer.
		if (sscanf(input, "%d", &num_threads) != 1) {
			printf("-- ERROR: Invalid input! Please enter a non-negative integer.\n");
			continue;  // Ask again.
		} else if (num_threads <= 0 || num_threads > MAX_THREADS) {
			printf("-- ERROR: Invalid number of threads. Max threads allowed: %d\n", MAX_THREADS);
			continue;
		}

		break;  // Valid input, exit loop.
	}



//	// Display parent PID.
//	printf("\n");
//	print_pid_msg("Parent running...");
//	parent_pid = getpid(); // Store the parent's PID before forking

	// For specified number of threads...
	for (int i = 0; i < num_threads; i++) {
//		// ... Fork process to spawn children.
//		pid_t pid = fork();
//
//		// Check for failed fork.
//		if (pid < 0) {
//			printf("Fork failed at iteration: '%d'. Exiting process.\n", i);
//			perror("Fork failed");
//			exit(1);
//		}
//		// Otherwise...
//		if (pid == 0) { // Child process
//
//			// Print child PID.
//			print_pid_msg("Child running...");
//
//			// Loop until USR1 signal handler has performed.
//			while (!usr1Happened) {
//			}
//
//			// Report preparation to exit.
//			print_pid_msg("Child exiting.");
//			exit(EXIT_SUCCESS);
//		}

		// Allocate memory for thread id reference
		int* thread_id = malloc(sizeof(int));
		// Check if allocation failed
		if (!thread_id) {
			perror("-- ERROR: malloc failed!");
			exit(EXIT_FAILURE);
		}

		*thread_id = i + 1;

		// Attempt to create a new thread
		if (pthread_create(&threads[i], NULL, thread_subroutine, thread_id) != 0) {
            perror("-- ERROR: `pthread_create` failed!");
            exit(EXIT_FAILURE);
		}
	}

	printf("All threads created. Use 'kill -s SIGUSR1 %d' to terminate.\n", getpid());

	// Keep the main thread alive until SIGUSR1 is received
	while (running) {
		sleep(1);
	}

	// Cleanup
	sem_destroy(&semaphore);
	printf("Main thread exiting.\n");


//
//	// Parent: Waiting for all children
//	int finished_children = 0;
//	while (finished_children < num_children) {
//		// Wait for any child to finish (and return its PID.)
//		pid_t child_pid = wait(NULL);
//
//		if (child_pid > 0) { // Child process returned its PID.
//			// Count the number of children finished
//			finished_children++;
//		}
//	}

//	// Report parent process done.
//	print_pid_msg("Children finished, parent exiting.");

	return 0; // Return 0 to indicate the program executed successfully
}
