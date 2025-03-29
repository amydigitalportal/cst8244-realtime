#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/types.h>
#include <errno.h>

#include "../../a2_common/a2.h"

#define RM_EXPECTED_ARGC 4

char data[DEV_STATUS_BUFSIZE];
//int server_coid;

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

command_t parse_command(const char *cmd) {
	if (strcmp(cmd, INSTR_START) == 0) return CMD_START;
	if (strcmp(cmd, INSTR_SET) == 0)   return CMD_SET;
	if (strcmp(cmd, INSTR_PAUSE) == 0) return CMD_PAUSE;
	if (strcmp(cmd, INSTR_QUIT) == 0)  return CMD_QUIT;
	return CMD_INVALID;
}


int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
	int nb;

	nb = strlen(data);

	//test to see if we have already sent the whole message.
	if (ocb->offset == nb)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, data, nb);

	//update offset into our data used to determine start position for next read.
	ocb->offset += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
	int nbytes = 0;


	// Check that we received all the data in one message
	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {

		char *buf;
		char command[CMD_BUFSIZE] = {0};
		char arg[CMD_BUFSIZE] = {0};

		// trim newline from the input
		buf[msg->i.nbytes - 1] = '\0';

		// parse command and optional arguments
		sscanf(buf, CMD_SCAN_FORMAT, command, arg);

		switch (parse_command(command)) {
			case CMD_START:
				break;
			case CMD_PAUSE:
				break;
			case CMD_SET:
				break;
			case CMD_QUIT:
				break;
			default:
				break;
		}

		nbytes = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES(ctp, nbytes);

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra) {
//	if ((server_coid = name_open(DEVICE_NAME, 0)) == -1) {
//		perror("name_open failed.");
//		return EXIT_FAILURE;
//	}
//
//	return (iofunc_open_default(ctp, msg, handle, extra));
}

int main(int argc, char *argv[]) {

	// Validate number of command arguments.
	if (argc != 4) {
		fprintf(stderr, USAGE_STR);
		exit(EXIT_FAILURE);
	}

	// Process command-line args.
	next_config.bpm 	= min(	max(atoi(argv[1]), 0), MAX_BPM);
	next_config.top 	= 		max(atoi(argv[2]), 0);
	next_config.bottom 	= 		max(atoi(argv[3]), 0);
	next_config_pending = true;

	// -- BEGIN ResMgr.

	dispatch_t *dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	dispatch_context_t *ctp;
	int id;

	if ((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s:  Unable to allocate dispatch context.\n", argv[0]);
		return (EXIT_FAILURE);
	}
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS,
			&io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	// Bind requests on the device path to the dispatch handle.
	if ((id = resmgr_attach(dpp, NULL, DEVICE_PATH, _FTYPE_ANY, 0,
			&connect_funcs, &io_funcs, &ioattr)) == -1) {
		fprintf(stderr, "%s:  Unable to attach name.\n", argv[0]);
		return (EXIT_FAILURE);
	}

	printf("myDevice: path '%s' registered to resource manager! Ready for dispatch ...\n",
			DEVICE_PATH);

	// Create metronome thread.
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(NULL, &attr, metronome_thread, NULL) != 0) {
		perror("res_mgr: Failed to create metronome thread");
		return EXIT_FAILURE;
	}

	// -- START main dispatch loop.
	ctp = dispatch_context_alloc(dpp);
	while (1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}
	return EXIT_SUCCESS;
}

