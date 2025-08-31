#include "Time.h"
#include "Interrupts.h"
#include "IfxGtm_Tom_Timer.h"

uint32 elapsed_ms;

IfxGtm_Tom_Timer clock;

void _GTM_Clocks_Init(void) {
	uint32 dummy;
	uint16 password;
	password = IfxScuWdt_getCpuWatchdogPassword();

	IfxScuWdt_clearCpuEndinit(password);
	GTM_CLC.U = 0x00000000; /* enable GTM module */
	dummy = GTM_CLC.U; /* need to read back before accessing SFRs */
	IfxScuWdt_setCpuEndinit(password);

	// set global clock to 2.5MHz
	IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, 2500000);

	// Set CLK0 to 100kHz
	IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, 100000);

	// Enable the fixed clock and clock 0
	IfxGtm_enable(&MODULE_GTM);
	IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0);
}

void Time_Start(void) {
	elapsed_ms = 0;

	// initialize GTM clock sources first
	_GTM_Clocks_Init();

	IfxGtm_Tom_Timer_Config tomConfig;
	IfxGtm_Tom_Timer_initConfig(&tomConfig, &MODULE_GTM);

	tomConfig.base.frequency = 1000;		// by default
	tomConfig.base.minResolution = 0.00001;
	tomConfig.base.isrPriority = TIME_PERIODIC_PRIORITY;
	tomConfig.base.isrProvider = TIME_PERIODIC_TOS;
	tomConfig.tom = IfxGtm_Tom_1;
	tomConfig.timerChannel = IfxGtm_Tom_Ch_0;
	tomConfig.initPins = FALSE;		// no output please (does nothing)

	IfxGtm_Tom_Timer_init(&clock, &tomConfig);
	IfxGtm_Tom_Timer_run(&clock);
}

uint32 Time_GetTime(void) {
	return elapsed_ms;
}

void Time_Delay(uint32 duration) {
	uint32 startTime = elapsed_ms;
	while (elapsed_ms - startTime < duration)
		// avoid overflow bug
		__asm("NOP");
	// poll
}

void Time_Delay_us(uint32 duration) {
	uint32 i;
	/*
	 *           Time_Delay_us:
8000227a:   mov  d15,0x01
8000227c:   j  0x80002282
67        		__asm("NOP");
8000227e:   nop
65        	for (i = 1; i < duration; i++)
80002280:   add  d15,0x01
80002282:   jlt.u  d15,d4,"Time.c"::67 (0x8000227E)
68        }
80002286:   ret
	 */
	// ~300 / 4, but based on measured result, i starts at 5 to compensate for function call time
	for (i = 5; i < duration * 75; i++){
		// avoid overflow bug
		__asm("NOP");
	}
}

void Time_Periodic_ISR(void) {
	elapsed_ms += 1;
	IfxGtm_Tom_Timer_acknowledgeTimerIrq(&clock);

	// run user ISR
#ifndef XINDONG_DEBUG
	if (elapsed_ms % 10 == 0)
		SWINT_Trigger_10ms();
	if (elapsed_ms % PID_PERIOD_MS == 0)
		SWINT_Trigger_pid();
	if (elapsed_ms % 100 == 0)
		SWINT_Trigger_100ms();
	if (elapsed_ms % 1000 == 0)
		SWINT_Trigger_1s();
#endif
}
