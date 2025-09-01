#include "Ultrasonic.h"
#include "Interrupts.h"
#include "Time.h"
#include "IfxGtm_Tim_In.h"
#include "IfxGtm_regdef.h"
#include "IfxSrc_reg.h"


#define SOUND_SPEED_MM_US 0.343


IfxGtm_Tim_In timDriver_pulseIn;

volatile uint32     _ultrasonicDistance = 0;
volatile uint8      _ultrasonicReady = 0;


void Ultrasonic_PulseIn_ISR(void)
{
    uint16 cnt = timDriver_pulseIn.channel->GPR1.B.GPR1;
    IfxGtm_Tim_In_update(&timDriver_pulseIn);

    timDriver_pulseIn.channel->CNTS.B.CNTS = 0;

    // disable interrupts
    IfxGtm_Tim_Ch_setChannelNotification(timDriver_pulseIn.channel, 0, 0, 0, 0);

    // deal with data
    if (cnt > 5000){
        _ultrasonicReady = 1;
        _ultrasonicDistance = -1;
    }
    else {
        _ultrasonicReady = 1;
        // find length in millimeters
        _ultrasonicDistance = cnt;
        _ultrasonicDistance *= 340000 / 2;
        _ultrasonicDistance /= timDriver_pulseIn.captureClockFrequency;
    }
}

void Ultrasonic_Init() {
    // initialize trigger pin
    IfxPort_setPinMode(ULTRA_TRIG_PORT, ULTRA_TRIG_PIN, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinState(ULTRA_TRIG_PORT, ULTRA_TRIG_PIN, IfxPort_State_low);

    // initialize echo pin
    IfxGtm_Tim_In_Config config;

    IfxGtm_Tim_In_initConfig(&config, &MODULE_GTM);
    config.timIndex = ULTRA_ECHO_TIM_PIN.tim;
    config.channelIndex = ULTRA_ECHO_TIM_PIN.channel;
    config.isrProvider = ULTRASONIC_PULSEIN_TOS;
    config.isrPriority = ULTRASONIC_PULSEIN_PRIORITY;
    config.capture.activeEdge = IfxGtm_Tim_In_ActiveEdge_raising;
    config.filter.inputPin = &ULTRA_ECHO_TIM_PIN;
    config.mode = IfxGtm_Tim_Mode_gatedPeriodic;

    IfxGtm_Tim_In_init(&timDriver_pulseIn, &config);
}

void Ultrasonic_Trigger() {
    // generate 15us pulse
    IfxPort_setPinState(ULTRA_TRIG_PORT, ULTRA_TRIG_PIN, IfxPort_State_high);
    Time_Delay_us(10);
    IfxPort_setPinState(ULTRA_TRIG_PORT, ULTRA_TRIG_PIN, IfxPort_State_low);

    timDriver_pulseIn.channel->CNTS.B.CNTS = 0;
    while (timDriver_pulseIn.channel->CNT.B.CNT != 0)
        ;       // wait for it

    // set time out to 5k/100k=50ms, here timer update is "time out"!!!
    timDriver_pulseIn.channel->CNTS.B.CNTS = 5000;

    // clear all interrupt flags
    timDriver_pulseIn.channel->IRQ.NOTIFY.U = 0x01F;

    // enable new value and timeout interrupt
    IfxGtm_Tim_Ch_setChannelNotification(timDriver_pulseIn.channel, 1, 1, 0, 0);
}

uint32 Ultrasonic_GetValue(){
    return _ultrasonicDistance;
}

uint8 Ultrasonic_IsReady(){
    return _ultrasonicReady;
}
