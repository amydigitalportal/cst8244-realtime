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
metronome_config_t metro_cfg 			= {}; 		// The active configuration of the metronome
const rhythm_pattern_t *next_rhythm;				// The pattern of the next measure
volatile bool pattern_update_pending 	= false; 	// Flag indicating whether the pattern is to change on the next measure
volatile bool bpm_update_pending 		= false;	// Flag indicating whether the timer should change its tick rate
pthread_t metronome_thread;							// Worker thread simulating a driver for a metronome device.

// Signal & Timer vars
struct sigevent timer_event; // signal emitted on timer tick
timer_t timer_id;

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
const rhythm_pattern_t* get_rhythm_pattern(int top, int bottom) {
	// Iterable through table...
	for (int i = 0; i < sizeof(rhythm_table)/sizeof(rhythm_table[0]); i++) {
		// Check if inputs match the table entry
		if (rhythm_table[i].top == top && rhythm_table[i].bottom == bottom) {
			return &rhythm_table[i];
		}
	}

	// No pattern matching args ...
	fprintf(stderr, "[Metro] no rhythm pattern matching provided args (top: %d, bottom: %d)\n",
			metro_cfg.rp->top, metro_cfg.rp->bottom);
	return NULL;
}



/////////////////////////
/// --- METRONOME --- ///

/**
 * Helper function to perform configuration on the metronome.
 */
int configure_metronome(int bpm, int top, int bottom) {
	const rhythm_pattern_t *target_pattern = get_rhythm_pattern(top, bottom);
	if (target_pattern == NULL) {
		return -1;
	}

	// Queue up the target rhythm pattern for the next measure
	next_rhythm = target_pattern;
	pattern_update_pending = true; // Trip the flag for update

	// Update the BPM (note: this will only take effect on the next "tick" of the timer)
	metro_cfg.bpm = bpm;

	return 0;
}

/**
 * Helper function to perform cleanup ops on the metronome thread.
 */
void _svr_cleanup() {
	printf("[Metro] Cleaning up metronome thread...\n");

	if (timer_id != 0)
		timer_delete(timer_id);

	if (attach != NULL)
		name_detach(attach, 0);
}

/**
 * Helper function to perform general cleanup and a graceful shutdown when an error occurs.
 */
void* svr_clean_exit_failure() {
	printf("-- Cannot proceed due to error! Metronome job shutting down.\n");
	_svr_cleanup();

	return NULL;
}

/**
 * Helper function to perfom general cleanup on succesful finish of server operation
 */
void* svr_clean_exit_success() {
	printf("-- Metronome job finished; job thread closing!\n");
	_svr_cleanup();

	return NULL;
}

void update_status_string(const metronome_config_t *cfg, double interval_secs) {
	memset(&metronome_status_str, 0, sizeof(metronome_status_str));
	snprintf(metronome_status_str,
			sizeof(metronome_status_str),
			"[metronome: %d bpm, time signature %d/%d, interval: %.2f sec]\n",
			cfg->bpm, cfg->rp->top, cfg->rp->bottom, interval_secs
	);
}

/**
 * Helper function used to calculate the seconds per tick interval for the metronome.
 */
double calc_interval_sec(const metronome_config_t *cfg) {
	const rhythm_pattern_t *pattern = cfg->rp;

	long sec_per_beat = 60 / (cfg->bpm > 0 ? cfg->bpm : 1);
	long sec_per_measure = sec_per_beat * pattern->top;

	return (sec_per_measure / pattern->num_intervals);
}

/**
 * Helper function to handle updating the timer specification using the given timer_t and interval in seconds.
 */
int update_metronome_timer(timer_t timer_id, double interval_sec) {
	struct itimerspec itime = {0};

	double adjusted_interval_sec = max(interval_sec, 0); // Safeguard against negative values.
	long sec = (long)adjusted_interval_sec;
	long nsec = (long)((adjusted_interval_sec - sec) * 1e9); // convert fractional sec to nsec

	itime.it_value.tv_sec = sec;
	itime.it_value.tv_nsec = nsec;

	itime.it_interval.tv_sec = sec;
	itime.it_interval.tv_nsec = nsec;

	// Update the timer
	if (timer_settime(timer_id, 0, &itime, NULL) == -1) {
		perror("[Metro] Failed to update timer interval");
		return -1;
	}

//	printf("[Metro] Timer interval updated to %.3f seconds\n", interval_sec);
	return 0;
}

void* metronome_thread_func(void* arg) {
	// -- PHASE I: create a named channel to receive pulses
	attach = name_attach( NULL, DEVICE_NAME, 0 );
	if (!attach) {
		perror("[Metro] name_attach failed");
		return svr_clean_exit_failure;
	}


	// Initialize the Timer Pulse event signal
	SIGEV_PULSE_INIT(
		&timer_event,           // The sigevent struct
		ConnectAttach(0, 0, attach->chid, _NTO_SIDE_CHANNEL, 0), // connection ID to the channel
		SchedGet(0, 0, NULL),   // priority of the pulse
		METRONOME_PULSE_CODE,   // your defined pulse code
		0                       // value you can optionally carry
	);

	// Create the interval timer to "drive" the metronome
	if (timer_create(CLOCK_MONOTONIC, &timer_event, &timer_id) == -1) {
		perror("[Metro] Failed to create timer");
		return svr_clean_exit_failure;
	}

	// -- BEGIN: Metronome loop
	int is_metro_looping = 1;
	while (is_metro_looping) {

		// Listen for a pulse
		rcvid = MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL);

		// Safeguard against bad reception.
		if (rcvid != 0) {
			fprintf(stderr, "myController: MsgReceivePulse entountered unexpected error! rcvid: %d\n", rcvid);
			// Terminate gracefully.
			return svr_clean_exit_failure();
		}

		// Process the message.
		if (rcvid == 0) {
			switch (msg.pulse.code) {
				case QUIT_PULSE_CODE:
					printf("[Metro] Received QUIT pulse. Stopping metronome...\n");
					is_metro_looping = 0;
					break;

				case METRONOME_PULSE_CODE:

					// IF reached end of measure...
					//	// Check whether a new config has been set
					//	if (next_config_pending) {
					//		// Update the current config
					//		current_config = next_config;
					//		next_config_pending = false; // reset the flag
					//	}
					break;

					// check if bpm has changed
					if (bpm_update_pending) {
						// Calculate the interval in seconds based on current configuration
						double new_interval_sec = calc_interval_sec(&metro_cfg);

						// Update the metronome timer tick rate
						update_metronome_timer(timer_id, new_interval_sec);

						bpm_update_pending = false; // Flip off the flag.
					}
				// other pulse handlers...
			}
		}
	}

	// -- PHASE III: Cleanup after loop exits
	return svr_clean_exit_success();
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

/**
 * Helper function to handle common cleanup on the ResMgr client.
 */
void _clt_cleanup() {
	if (metronome_coid)
		name_close(metronome_coid);
}

/**
 * Helper function to perform general cleanup and a graceful shutdown of the client when an error occurs.
 */
int clt_clean_exit_failure() {
	printf("-- Cannot proceed due to error! Resource manager shutting down.\n");
	_clt_cleanup();
	return EXIT_FAILURE;
}

/**
 * Helper function to perform general cleanup on succesful finish of client operation
 */
int clt_clean_exit_success() {
	printf("-- Metronome Resource Manager finished; shutting down. Goodbye!\n");
	_clt_cleanup();
	return EXIT_SUCCESS;
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

	// Process command-line args for metronome's initial configuration.
	int init_bpm 	= min(	max(atoi(argv[1]), 0), MAX_BPM);
	int init_top 	= 		max(atoi(argv[2]), 0);
	int init_bottom = 		max(atoi(argv[3]), 0);

	if (configure_metronome(init_bpm, init_top, init_bottom) != 0) {
		return EXIT_FAILURE;
	}

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
		return clt_clean_exit_failure();
	}

	// -- START main dispatch loop.
	ctp = dispatch_context_alloc(dpp);
	while (1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}

	// Gracefully teardown.
	return clt_clean_exit_success();
}

