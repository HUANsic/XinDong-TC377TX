#include "Intercore.h"
#include "XinDong_Config.h"

uint8 cpu0_ready = 0, cpu1_ready = 0, cpu2_ready = 0;
uint8 initAllowed = 0;
uint8 battery_connected = 0;

void Intercore_Init(void) {
	IfxPort_setPinMode(IO_BATT_PORT, IO_BATT_PIN, IfxPort_Mode_inputNoPullDevice);
}

void Intercore_CPU0_Ready(void) {
	cpu0_ready = 1;
}

void Intercore_CPU1_Ready(void) {
	cpu1_ready = 1;
}

void Intercore_CPU2_Ready(void) {
	cpu2_ready = 1;
}

uint8 Intercore_InitAllowed(void) {
	battery_connected = IfxPort_getPinState(IO_BATT_PORT, IO_BATT_PIN);
	return (initAllowed && battery_connected);
}

void Intercore_Checkpoint(void) {
	battery_connected = IfxPort_getPinState(IO_BATT_PORT, IO_BATT_PIN);
}

void Intercore_AllowInitialize(void) {
	initAllowed = 1;
}

uint8 Intercore_ReadyToGo(void) {
	return (cpu0_ready && cpu1_ready && cpu2_ready);
}
