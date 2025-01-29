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


}
