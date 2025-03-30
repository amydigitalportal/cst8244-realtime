/*
 * a2.h
 *
 *  Created on: Mar. 28, 2025
 *      Author: Amy-PC
 */

#ifndef A2_H_
#define A2_H_

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

#define MAX_BPM 400
#define USAGE_STR "Usage: metronome <bpm> <timesig-top> <timesig-bottom>"

#define METRONOME_PULSE_CODE 	(_PULSE_CODE_MINAVAIL + 1)
#define PAUSE_PULSE_CODE     	(_PULSE_CODE_MINAVAIL + 2)
#define QUIT_PULSE_CODE      	(_PULSE_CODE_MINAVAIL + 3)
#define SET_CONFIG_PULSE_CODE 	(_PULSE_CODE_MINAVAIL + 4)
#define START_PULSE_CODE 		(_PULSE_CODE_MINAVAIL + 5)
#define STOP_PULSE_CODE 		(_PULSE_CODE_MINAVAIL + 6)

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
	int bpm;
	double timer_interval_sec; // Tick interval of the timer
	const rhythm_pattern_t *rp;
} metronome_config_t;

#endif /* A2_H_ */
