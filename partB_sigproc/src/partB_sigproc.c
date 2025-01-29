#include <stdio.h>
#include <stdlib.h>

static char* pid_string() {
	char* buffer = malloc(16);
    if (buffer != NULL) {
        snprintf(buffer, 16, "PID = %d", getpid());
    }
    return buffer;
}

static void print_pid_msg(char* msg) {
	char* pid_str = pid_string();
	if (pid_str != NULL)
	{
		printf("%s : %s\n", pid_str, msg);
	}
	free(pid_str);
}


/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(void) {

	printf("//run program from Momentics IDE\n", getpid());

	int num_children;
	char input[100];  // Buffer for user input

	while (1) {
		printf("Enter the number of children:\n");

		// Read input from the user
		if (fgets(input, sizeof(input), stdin) == NULL) {
			printf("Error reading input. Please try again.\n");
			continue;
		}

		// Attempt to convert input to an integer
		if (sscanf(input, "%d", &num_children) != 1 || num_children < 0) {
			printf("-- ERROR: Invalid input! Please enter a non-negative integer.\n");
			continue;  // Ask again
		}

		break;  // Valid input, exit loop
	}

	printf("Valid input!");
}
