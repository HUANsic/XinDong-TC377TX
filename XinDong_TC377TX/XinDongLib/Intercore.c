#include "Intercore.h"
#include "XinDong_Config.h"

uint8 _cpu0_ready = 0, _cpu1_ready = 0, _cpu2_ready = 0;
uint8 _initAllowed = 0;
uint8 _battery_connected = 0;

void Intercore_Init(void) {
	IfxPort_setPinMode(IO_BATT_PORT, IO_BATT_PIN, IfxPort_Mode_inputNoPullDevice);
}

void Intercore_CPU0_Ready(void) {
	_cpu0_ready = 1;
}

void Intercore_CPU1_Ready(void) {
	_cpu1_ready = 1;
}

void Intercore_CPU2_Ready(void) {
	_cpu2_ready = 1;
}

uint8 Intercore_InitAllowed(void) {
	_battery_connected = IfxPort_getPinState(IO_BATT_PORT, IO_BATT_PIN);
	return (_initAllowed && _battery_connected);
}

void Intercore_Checkpoint(void) {
	_battery_connected = IfxPort_getPinState(IO_BATT_PORT, IO_BATT_PIN);
}

void Intercore_AllowInitialize(void) {
	_initAllowed = 1;
}

uint8 Intercore_ReadyToGo(void) {
	return (_cpu0_ready && _cpu1_ready && _cpu2_ready);
}
