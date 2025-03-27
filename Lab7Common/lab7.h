/*
 * lab7.h
 *
 *  Created on: Mar. 27, 2025
 *      Author: Amy-PC
 */

#ifndef LAB7_H_
#define LAB7_H_


#define DEVICE_PATH "/dev/local/mydevice"
#define DEVICE_NAME "mydevice"
#define DEV_STATUS_BUFSIZE 255

#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL

typedef union {
	struct _pulse pulse;
	char msg[DEV_STATUS_BUFSIZE];
} my_message_t;

#endif /* LAB7_H_ */
