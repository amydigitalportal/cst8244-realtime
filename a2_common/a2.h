/*
 * a2.h
 *
 *  Created on: Mar. 28, 2025
 *      Author: Amy-PC
 */

#ifndef A2_H_
#define A2_H_

#define DEVICE_NAME "metronome"
#define DEVICE_PATH "/dev/local/metronome"
#define DEV_STATUS_BUFSIZE 255

#define MAX_BPM 400
#define USAGE_STR "Usage: metronome <bpm> <timesig-top> <timesig-bottom>"

#define METRONOME_PULSE_CODE 	(_PULSE_CODE_MINAVAIL + 1)
#define PAUSE_PULSE_CODE     	(_PULSE_CODE_MINAVAIL + 2)
#define QUIT_PULSE_CODE      	(_PULSE_CODE_MINAVAIL + 3)
#define SET_CONFIG_PULSE_CODE 	(_PULSE_CODE_MINAVAIL + 3)

typedef union {
	struct _pulse pulse;
	char msg[DEV_STATUS_BUFSIZE];
} my_message_t;

typedef struct {
	int bpm;
	int top;
	int bottom;
} metronome_config_t;

typedef struct {
	int top;
	int bottom;
	int num_intervals;
	const char *pattern[12]; // max 12 from 12/8
} rhythm_pattern_t;

#endif /* A2_H_ */
