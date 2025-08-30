#include "IO.h"
#include "XinDong_Config.h"
#include "Interrupts.h"
#include <IfxGtm_PinMap.h>
#include <IfxGtm_Tim_In.h>

IfxGtm_Tim_In reedDriver;

//  reference code: TC264_XinDong_Demo_v51/Src/Utilities.c

void Reed_Init(void) {
	/*
	 *  Here I'm using TIM because:
	 *      (1) ERU doesn't support filtering
	 *      (2) There are limited (8) ERU outputs, I want to reserve them for future use
	 */
	IfxGtm_Tim_In_Config config;

	IfxGtm_Tim_In_initConfig(&config, &MODULE_GTM);
	config.timIndex = REED_TIM_PIN.tim;
	config.channelIndex = REED_TIM_PIN.channel;
	config.isrProvider = IfxSrc_Tos_cpu0;
	config.isrPriority = IO_REED_PRIORITY;
	config.capture.irqOnNewVal = TRUE;      // interrupt on falling edge!!!
	config.capture.activeEdge = IfxGtm_Tim_In_ActiveEdge_falling;
	config.filter.inputPin = &REED_TIM_PIN;
	config.filter.risingEdgeMode = IfxGtm_Tim_In_ConfigFilterMode_individualDeglitchTimeHold;
	config.filter.fallingEdgeMode = IfxGtm_Tim_In_ConfigFilterMode_individualDeglitchTimeHold;
	config.filter.risingEdgeFilterTime = 0.003;     // de-glitch for 3ms
	config.filter.fallingEdgeFilterTime = 0.003;
	config.mode = IfxGtm_Tim_Mode_inputEvent;

	IfxGtm_Tim_In_init(&reedDriver, &config);
}

//----------------------------------------------------------------LED--------------------------------------------------------

// Init
void IO_Init(void) {
	IfxPort_setPinMode(IO_LED1_PORT, IO_LED1_PIN, IfxPort_Mode_outputPushPullGeneral);
	IfxPort_setPinMode(IO_LED2_PORT, IO_LED2_PIN, IfxPort_Mode_outputPushPullGeneral);
	IfxPort_setPinMode(IO_LED3_PORT, IO_LED3_PIN, IfxPort_Mode_outputPushPullGeneral);
	IfxPort_setPinMode(IO_LED4_PORT, IO_LED4_PIN, IfxPort_Mode_outputPushPullGeneral);
	IfxPort_setPinMode(IO_DIP1_PORT, IO_DIP1_PIN, IfxPort_Mode_inputNoPullDevice);
	IfxPort_setPinMode(IO_DIP2_PORT, IO_DIP2_PIN, IfxPort_Mode_inputNoPullDevice);
	IfxPort_setPinMode(IO_DIP3_PORT, IO_DIP3_PIN, IfxPort_Mode_inputNoPullDevice);
	IfxPort_setPinMode(IO_DIP4_PORT, IO_DIP4_PIN, IfxPort_Mode_inputNoPullDevice);
	Reed_Init();
}

// Turn on the LED with specified number (1-4)
void IO_LED_On(int ledNum) {
	if (ledNum == 1) {
		IfxPort_setPinHigh(IO_LED1_PORT, IO_LED1_PIN);
	}
	else if (ledNum == 2) {
		IfxPort_setPinHigh(IO_LED2_PORT, IO_LED2_PIN);
	}
	else if (ledNum == 3) {
		IfxPort_setPinHigh(IO_LED3_PORT, IO_LED3_PIN);
	}
	else if (ledNum == 4) {
		IfxPort_setPinHigh(IO_LED4_PORT, IO_LED4_PIN);
	}
}

// Turn off the LED with specified number (1-4)
void IO_LED_Off(int ledNum) {
	if (ledNum == 1) {
		IfxPort_setPinLow(IO_LED1_PORT, IO_LED1_PIN);
	}
	else if (ledNum == 2) {
		IfxPort_setPinLow(IO_LED2_PORT, IO_LED2_PIN);
	}
	else if (ledNum == 3) {
		IfxPort_setPinLow(IO_LED3_PORT, IO_LED3_PIN);
	}
	else if (ledNum == 4) {
		IfxPort_setPinLow(IO_LED4_PORT, IO_LED4_PIN);
	}
}

void IO_LED_Set(uint8 ledNum, uint8 state) {
	if (ledNum == 1) {
		IfxPort_setPinState(IO_LED1_PORT, IO_LED1_PIN, state ? IfxPort_State_high : IfxPort_State_low);
	}
	else if (ledNum == 2) {
		IfxPort_setPinState(IO_LED2_PORT, IO_LED2_PIN, state ? IfxPort_State_high : IfxPort_State_low);
	}
	else if (ledNum == 3) {
		IfxPort_setPinState(IO_LED3_PORT, IO_LED3_PIN, state ? IfxPort_State_high : IfxPort_State_low);
	}
	else if (ledNum == 4) {
		IfxPort_setPinState(IO_LED4_PORT, IO_LED4_PIN, state ? IfxPort_State_high : IfxPort_State_low);
	}
}

// Toggle the state of the LED with specified number (1-4)
void IO_LED_Toggle(int ledNum) {
	if (ledNum == 1) {
		IfxPort_togglePin(IO_LED1_PORT, IO_LED1_PIN);
	}
	else if (ledNum == 2) {
		IfxPort_togglePin(IO_LED2_PORT, IO_LED2_PIN);
	}
	else if (ledNum == 3) {
		IfxPort_togglePin(IO_LED3_PORT, IO_LED3_PIN);
	}
	else if (ledNum == 4) {
		IfxPort_togglePin(IO_LED4_PORT, IO_LED4_PIN);
	}
}

//----------------------------------------------------------------SWITCH-------------------------------------------------------
// Read the state of the DIP switch with specified number (1-4)
// Returns TRUE if the switch is in one state, FALSE otherwise
boolean IO_DIP_Read(int swNum) {
	if (swNum == 1) {
		return IfxPort_getPinState(IO_DIP1_PORT, IO_DIP1_PIN);
	}
	else if (swNum == 2) {
		return IfxPort_getPinState(IO_DIP2_PORT, IO_DIP2_PIN);
	}
	else if (swNum == 3) {
		return IfxPort_getPinState(IO_DIP3_PORT, IO_DIP3_PIN);
	}
	else if (swNum == 4) {
		return IfxPort_getPinState(IO_DIP4_PORT, IO_DIP4_PIN);
	}
	else {
		return FALSE; // Return default value for invalid number
	}
}

void IO_Reed_ISR() {
	IfxGtm_Tim_In_update(&reedDriver);
	if (reedDriver.newData) {
		Reed_Triggered();
	}
}

__weak__ void Reed_Triggered(void) {
	;       // defined in USER file
}

