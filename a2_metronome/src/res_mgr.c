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

#include "a2.h"

#define RM_EXPECTED_ARGC 4

// Metronome vars
char metronome_status_str[MSG_BUFSIZE]; 			// Holds read-reply content (ie. reporting on the status of the metronome device upon being read.)
metronome_config_t current_metro_conf 	= {}; 		// The active configuration of the metronome
metronome_config_t next_metro_conf 		= {}; 		// A configuration in queue for the next measure(s).
volatile bool next_config_pending 		= false; 	// Flag determining whether the current config is to be updated
pthread_t metronome_thread;

// Signal & Timer vars
struct sigevent timer_event; // signal emitted on timer tick
timer_t timer_id;
struct itimerspec itime; // timer specifications

// IPC vars
my_message_t msg;
name_attach_t* attach;
int metronome_coid;
int rcvid;

// Lookup table for rhythm pattenrs
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


/**
 * Helper function to perform general cleanup and a graceful shutdown when an error occurs.
 */
void cleanExitFailure() {
	printf("-- Cannot proceed due to error! Metronome process shutting down.\n");
	name_detach(attach, 0);
	exit(EXIT_FAILURE);
}

/**
 * Helper function to perfom general cleanup on succesful finish of server operation
 */
void cleanExitSuccess() {
	printf("-- Metronome driver finished; shutting down. Goodbye!\n");
	name_detach(attach, 0);
	exit(EXIT_SUCCESS);
}

/////////////////////////
/// --- METRONOME --- ///

void* metronome_thread_func(void* arg) {
	// -- PHASE I: create a named channel to receive pulses
	attach = name_attach( NULL, DEVICE_NAME, 0 );
	if (!attach) {
		perror("[Metro] name_attach failed");
		return NULL;
	}

	// Calculate the seconds-per-beat and nano seconds for the interval timer
//	long sec_per_beat, sec_per_measure, nanos = 0;
//	sec_per_beat = 60 / current_config.bpm;
//	sec_per_measure =
//
//
//	// Create an interval timer to "drive" the metronome
//
//
//	// Check whether a new config has been set
//	if (next_config_pending) {
//		// Update the current config
//		current_config = next_config;
//		next_config_pending = false; // reset the flag
//	}

	// -- BEGIN: Metronome loop
	int is_metro_looping = 1;
	while (is_metro_looping) {

		// Listen for a pulse
		rcvid = MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL);

		// Safeguard against bad reception.
		if (rcvid != 0) {
			fprintf(stderr, "myController: MsgReceivePulse entountered unexpected error! rcvid: %d\n", rcvid);
			// Terminate gracefully.
			cleanExitFailure();
		}

		// Process the message.
		if (rcvid == 0) {
			switch (msg.pulse.code) {
				case QUIT_PULSE_CODE:
					printf("[Metro] Received QUIT pulse. Stopping metronome...\n");
					is_metro_looping = 0;
					break;

				// other pulse handlers...
			}
		}
	}

	// -- PHASE III: Cleanup after loop exits
	printf("[Metro] Cleaning up metronome thread...\n");

//	if (timer_id != 0)
//		timer_delete(timer_id);

	name_detach(attach, 0);
	return EXIT_SUCCESS;
}


///////////////////////
/// --- RES MGR --- ///

command_t parse_command(const char *cmd) {
	if ( strcmp(cmd, INSTR_QUIT) 	== 0) return CMD_QUIT;
	if ( strcmp(cmd, INSTR_PAUSE) 	== 0) return CMD_PAUSE;
	if ( strcmp(cmd, INSTR_START) 	== 0) return CMD_START;
	if ( strcmp(cmd, INSTR_STOP)  	== 0) return CMD_STOP;
	if ( strcmp(cmd, INSTR_SET)   	== 0) return CMD_SET;
	return CMD_INVALID;
}

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
	int nbytes;

	nbytes = strlen(metronome_status_str);

	//test to see if we have already sent the whole message.
	if (ocb->offset == nbytes)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nbytes = min(nbytes, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nbytes);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, metronome_status_str, nbytes);

	//update offset into our data used to determine start position for next read.
	ocb->offset += nbytes;

	//If we are going to send any bytes update the access time for this resource.
	if (nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
	int nbytes = 0;

	// Check that we received all the data in one message
	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {

		char *msg_buf;
		char command_buf[CMD_BUFSIZE] = {0};
		char arg_buf[CMD_BUFSIZE] = {0};

		// Obtain message contents
		msg_buf = (char*) (msg + 1);
		msg_buf[msg->i.nbytes - 1] = '\0'; // trim newline from the input

		// parse command and optional arguments
		sscanf(msg_buf, CMD_SCAN_FORMAT, command_buf, arg_buf);

		// process command and (if provided) args
		command_t cmd = parse_command(command_buf);
		switch (cmd) {

			case CMD_PAUSE:
				printf("[RM] Received PAUSE command with arg: %s\n", arg_buf);
				break;

			case CMD_QUIT:
				printf("[RM] Sending QUIT pulse to metronome...\n");

				int prio = SchedGet(0, 0, NULL); // get current thread priority
				int result = MsgSendPulse(metronome_coid,
				                          prio,
				                          QUIT_PULSE_CODE,
										  0); // empty value

				if (result == -1) {
					perror("[RM] MsgSendPulse failed");
				}
				break;

			case CMD_START:
				break;

			case CMD_STOP:
				break;

			case CMD_SET:
				break;

			default:
				fprintf(stderr, "[RM] Unknown or unhandled command: %s\n", command_buf);
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
	// Attempt opening a namespace connection to metronome device and capture file descriptor (coid)
	if ((metronome_coid = name_open(DEVICE_NAME, 0)) == -1) {
		perror("name_open failed.");
		return EXIT_FAILURE;
	}

	return (iofunc_open_default(ctp, msg, handle, extra));
}

int main(int argc, char *argv[]) {

	// Validate number of command arguments.
	if (argc != 4) {
		fprintf(stderr, USAGE_STR);
		exit(EXIT_FAILURE);
	}

	// Process command-line args.
	current_metro_conf.bpm 		= min(	max(atoi(argv[1]), 0), MAX_BPM);
	current_metro_conf.top 		= 		max(atoi(argv[2]), 0);
	current_metro_conf.bottom 	= 		max(atoi(argv[3]), 0);

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

	if (pthread_create(&metronome_thread, &attr, &metronome_thread_func, NULL) != 0) {
		perror("res_mgr: Failed to create metronome thread");
		return EXIT_FAILURE;
	}

	// -- START main dispatch loop.
	ctp = dispatch_context_alloc(dpp);
	while (1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}

	// Gracefully teardown.
	name_close(metronome_coid);

	return EXIT_SUCCESS;
}

