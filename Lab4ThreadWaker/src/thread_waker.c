#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#define SEM_NAME "/thread_factory_semaphore"

int main(void) {
	sem_t *semaphore;
	int num_wakeups;

	printf("thread_waker started. PID: %d\n", getpid());

	// Open the existing semaphore created by thread_factory
	semaphore = sem_open(SEM_NAME, 0);
	if (semaphore == SEM_FAILED) {
		perror("-- ERROR: `sem_open` failed!");
		return EXIT_FAILURE;
	}

	char input[100];  // Buffer for input

	while (1) {
		// Prompt user input
		printf("Enter number of threads to wake up (0 to exit): ");

		// Read input as a string
		fgets(input, sizeof(input), stdin);

		// Convert input to an integer
		if (sscanf(input, "%d", &num_wakeups) != 1) {
			printf("-- ERROR: Invalid input! Please enter a number.\n");
			continue;  // Ask again
		}

	    if (num_wakeups <= 0) {
	        printf("Exiting thread_waker...\n");
	        break;
	    }

		// Increment the semaphore by the specified number
		for (int i = 0; i < num_wakeups; i++) {
			sem_post(semaphore);
		}

		printf("Woke up %d thread(s).\n", num_wakeups);
	}

	// Cleanup
	sem_close(semaphore);
	return EXIT_SUCCESS;
}
