/*
 * a2.h
 *
 *  Created on: Mar. 28, 2025
 *      Author: Amy-PC
 */

#ifndef A2_H_
#define A2_H_

// Preprocessor Gandalf
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// String representations of the instruction set that the ResMgr recognizes
#define INSTR_START "start"
#define INSTR_STOP 	"stop"
#define INSTR_SET 	"set"
#define INSTR_PAUSE "pause"
#define INSTR_QUIT 	"quit"

typedef enum {
	CMD_INVALID,
	CMD_START,
	CMD_STOP,
	CMD_SET,
	CMD_PAUSE,
	CMD_QUIT
} command_t;

#define DEVICE_NAME "metronome"
#define METRONOME_DEV_PATH      "/dev/local/metronome"
#define METRONOME_HELP_DEV_PATH "/dev/local/metronome-help"
#define MSG_BUFSIZE 255
#define CMD_BUFSIZE 32
#define CMD_SCAN_FORMAT "%31s %31s"

#define METRONOME_PULSE_CODE 	(_PULSE_CODE_MINAVAIL + 1)
#define PAUSE_PULSE_CODE     	(_PULSE_CODE_MINAVAIL + 2)
#define QUIT_PULSE_CODE      	(_PULSE_CODE_MINAVAIL + 3)
#define SET_CONFIG_PULSE_CODE 	(_PULSE_CODE_MINAVAIL + 4)
#define START_PULSE_CODE 		(_PULSE_CODE_MINAVAIL + 5)
#define STOP_PULSE_CODE 		(_PULSE_CODE_MINAVAIL + 6)

#define METRO_CFG_ARGC 3
#define MIN_BPM 0
#define MAX_BPM 400
#define USAGE_STR "Usage: metronome <bpm " STR(MIN_BPM) "-" STR(MAX_BPM) "> <timesig-top> <timesig-bottom>"

#define METRO_MIN_PAUSE 1
#define METRO_MAX_PAUSE 9

typedef union {
	struct _pulse pulse;
	char msg[MSG_BUFSIZE];
} my_message_t;

typedef struct {
	int top;
	int bottom;
	int num_intervals;
	const char *pattern[12]; // max 12 from 12/8
} rhythm_pattern_t;

typedef struct {
	int bpm;							// Beats per minute
	double timer_interval_sec; 			// Tick interval of the timer
	const rhythm_pattern_t *rp;			// Current pattern configured to the metronome
} metronome_config_t;

typedef struct {
	char status_str[MSG_BUFSIZE];		// Holds read-reply content (ie. reporting on the status of the metronome device upon being read.)
	metronome_config_t cfg;				// The active configuration of the metronome
	const rhythm_pattern_t *next_rp;	// The pattern of the next measure
	bool pattern_update_pending;		// Flag indicating whether the pattern is to change on the next measure
	bool timer_update_pending;			// Flag indicating whether the timer should change its tick rate
	bool is_playing;					// The metronome play status.
	bool is_paused;						// Status of metronome timer pause.

	my_message_t msg;
	name_attach_t* p_attach;
	int coid;
	int rcvid;

	pthread_t thread;  					// Worker thread simulating a driver for a metronome device.

	struct sigevent timer_event; // signal emitted on timer tick
	timer_t timer_id;
} metronome_t;

#endif /* A2_H_ */
