/*
 * HUANsic_SU20T.c
 *
 *  Created on: 2025年8月31日
 *      Author: abcde
 */

#include "HUANsic_SU20T.h"
#include "XinDongLib/Time.h"
#include "XinDongLib/Serial.h"
#include <string.h>

#define SU20T_TIMEOUT	100

const uint8 SU_START_MSG[] = "START";
const uint8 SU_STOP_MSG[] = "STOP";
const uint8 SU_SHOW_MSG[] = "SHOW";
const uint8 SU_HB_MSG[] = "OK";

const uint8 SU_LEFT_ON_CMD[] = { 0xAA, 0x55, 0x02, 0x01, 0x55, 0xAA };
const uint8 SU_LEFT_OFF_CMD[] = { 0xAA, 0x55, 0x02, 0x00, 0x55, 0xAA };
const uint8 SU_RIGHT_ON_CMD[] = { 0xAA, 0x55, 0x03, 0x01, 0x55, 0xAA };
const uint8 SU_RIGHT_OFF_CMD[] = { 0xAA, 0x55, 0x03, 0x00, 0x55, 0xAA };
const uint8 SU_BREAK_ON_CMD[] = { 0xAA, 0x55, 0x04, 0x01, 0x55, 0xAA };
const uint8 SU_BREAK_OFF_CMD[] = { 0xAA, 0x55, 0x04, 0x00, 0x55, 0xAA };
const uint8 SU_TAIL_ON_CMD[] = { 0xAA, 0x55, 0x05, 0x01, 0x55, 0xAA };
const uint8 SU_TAIL_OFF_CMD[] = { 0xAA, 0x55, 0x05, 0x00, 0x55, 0xAA };
const uint8 SU_LOWBEAM_ON_CMD[] = { 0xAA, 0x55, 0x06, 0x01, 0x55, 0xAA };
const uint8 SU_LOWBEAM_OFF_CMD[] = { 0xAA, 0x55, 0x06, 0x00, 0x55, 0xAA };

uint8 su_waiting_heartbeat = 0;

sint8 su_turning = 0;		// left < 0, right > 0
uint8 su_turn_counter = 0;
uint8 su_accelerating = 0;

void SU20T_Init(void) {
	while(SU20T_Check());
}

uint8 SU20T_Check(void) {
	uint32 startTime = Time_GetTime();
	uint8 heartbeatMsg[] = { 0xAA, 0x55, 0x01, 0x55, 0xAA };
	while(Serial_Transmit(heartbeatMsg, 5));
	su_waiting_heartbeat = 1;
	while(su_waiting_heartbeat) {
		if(Time_GetTime() - startTime > SU20T_TIMEOUT) {
			return 1;	// no device connected
		}
	}
	return 0;
}

void SU20T_Decode(uint8 *msg) {
	if(strcmp(msg, SU_START_MSG) == 0) {

	} else if(strcmp(msg, SU_STOP_MSG) == 0) {

	} else if(strcmp(msg, SU_SHOW_MSG) == 0) {

	} else if(strcmp(msg, SU_HB_MSG)) {
		su_waiting_heartbeat = 0;
	}
}

void SU20T_100ms_Isr(void) {
	if(su_turning) {
		if(su_turning > 0) {
			// right turn
			if(su_turn_counter < 10) {
				Serial_Transmit(SU_RIGHT_ON_CMD, 6);
			} else if (su_turn_counter < 20) {
				Serial_Transmit(SU_RIGHT_OFF_CMD, 6);
			}
		} else if(su_turning < 0) {
			// left turn
			if(su_turn_counter < 10) {
				Serial_Transmit(SU_LEFT_ON_CMD, 6);
			} else if (su_turn_counter < 20) {
				Serial_Transmit(SU_LEFT_OFF_CMD, 6);
			}
		}
		su_turn_counter++;
		su_turn_counter = su_turn_counter % 20;
	}
}

void SU20T_SetTurning(sint8 dir) {
	if(dir == 0) {
		su_turning = 0;
		su_turn_counter = 0;
		Serial_Transmit(SU_LEFT_OFF_CMD, 6);
		Serial_Transmit(SU_RIGHT_OFF_CMD, 6);
	} else {
		su_turn_counter = 0;
		su_turning = dir;
	}
}

void SU20T_SetBreaking(uint8 brk) {
	if(brk) {
		Serial_Transmit(SU_BREAK_ON_CMD, 6);
	} else {
		Serial_Transmit(SU_BREAK_OFF_CMD, 6);
	}
}

void SU20T_SetLowBeam(uint8 on) {
	if(on) {
		Serial_Transmit(SU_LOWBEAM_ON_CMD, 6);
	} else {
		Serial_Transmit(SU_LOWBEAM_OFF_CMD, 6);
	}
}

void SU20T_SetTail(uint8 on) {
	if(on) {
		Serial_Transmit(SU_TAIL_ON_CMD, 6);
	} else {
		Serial_Transmit(SU_TAIL_OFF_CMD, 6);
	}
}
