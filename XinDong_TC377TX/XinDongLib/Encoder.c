#include "Encoder.h"
#include "Interrupts.h"
#include "IfxGpt12.h"
#include "IfxSrc.h"
#include "IfxCpu.h"
#include "IfxPort.h"

#define ENCODER_MAX_COUNT     0xFFFF
#define ENCODER_T2_SRC        &SRC_GPT120T2
#define ENCODER_REVERSE       TRUE


volatile sint32 encoderCount = 0;


void Encoder_Overflow_ISR(void) {
    boolean dir = IfxPort_getPinState(ENCODER_B_GPT12_PIN.pin.port, ENCODER_B_GPT12_PIN.pin.pinIndex);
    if (dir)
        encoderCount -= ENCODER_MAX_COUNT + 1;
    else
        encoderCount += ENCODER_MAX_COUNT + 1;

    IfxSrc_clearRequest(ENCODER_T2_SRC);
}

void Encoder_Init() {
    // counter for encoder initialize
    IfxGpt12_enableModule(&MODULE_GPT120);
    IfxGpt12_setGpt1BlockPrescaler(&MODULE_GPT120, IfxGpt12_Gpt1BlockPrescaler_4);
    IfxGpt12_setGpt2BlockPrescaler(&MODULE_GPT120, IfxGpt12_Gpt2BlockPrescaler_4);

    IfxGpt12_T2_setInput(&MODULE_GPT120, IfxGpt12_Input_B);
    IfxGpt12_T2_setEudInput(&MODULE_GPT120, IfxGpt12_EudInput_B);

    IfxGpt12_T2_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
    IfxGpt12_T2_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
    IfxGpt12_T2_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);

    // overflow/underflow interrupt initialize
    IfxGpt12_T2_setInterruptEnable(&MODULE_GPT120, TRUE);

    IfxSrc_init(ENCODER_T2_SRC, IfxSrc_Tos_cpu2, ENCODER_OVERFLOW_PRIORITY);
    IfxSrc_enable(ENCODER_T2_SRC);

    IfxGpt12_T2_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);

    encoderCount = 0;
}

sint32 Encoder_GetValue()
{
    uint16 raw = IfxGpt12_T2_getTimerValue(&MODULE_GPT120);
    sint32 total;

    total = encoderCount + raw;

    return ENCODER_REVERSE ? -total : total;
}
