#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#define MAX_THREADS 100
#define SEMAPHORE_NAME "/thread_factory_semaphore"

sem_t *semaphore;
volatile sig_atomic_t running = 1;

void handle_sigusr1(int signo) {
	if (signo == SIGUSR1) {
		printf("\nReceived SIGUSR1, terminating program...\n");
		running = 0;

        // Wake up all potentially blocked threads
        for (int i = 0; i < MAX_THREADS; i++) {
            sem_post(semaphore);
        }

        printf("All threads have been signaled to wake up.\n");
	}
}

void* thread_subroutine(void* arg) {
	int thread_id = *(int*)arg;
	free(arg); // Free dynamic allocated mem
	printf("Thread %d created.\n", thread_id);

	while (running) {
		sem_wait(semaphore); // wait for semaphore

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


	sem_unlink(SEMAPHORE_NAME); // Remove any previous instance in case another process is still using it

	// Initialize the named semaphore
	int perms = 0666; // read-write for OWNER, GROUP, and OTHER
	semaphore = sem_open(SEMAPHORE_NAME, O_CREAT, perms, 0);
	// Check if semaphore failed to open
	if (semaphore == SEM_FAILED) {
	    perror("-- ERROR: `sem_open` failed!");
	    exit(EXIT_FAILURE);
	}


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

	// For specified number of threads...
	for (int i = 0; i < num_threads; i++) {
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
			free(thread_id); // free allocated memory

			exit(EXIT_FAILURE);
		}
	}

	printf("All threads created. Use 'kill -s SIGUSR1 %d' to terminate.\n", getpid());

	// Keep the main thread alive until SIGUSR1 is received
	while (running) {
		sleep(1);
	}

	// Cleanup
	sem_close(semaphore);

	printf("Main thread exiting.\n");

	return 0; // Return 0 to indicate the program executed successfully
}
