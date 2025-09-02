#include "Ifx_Types.h"
#include "IfxGtm_Tom_Pwm.h"

#include "Movements.h"


#define SERVO_FREQUENCY 50
#define SERVO_PERIOD 50000
#define SERVO_1MS_COUNT (SERVO_PERIOD / 1000 * SERVO_FREQUENCY)

#define MOTOR_FREQUENCY 32000
#define MOTOR_PERIOD 100
#define MOTOR_REVERSE FALSE


struct PID {
    float target_speed;
    float current_speed;
    float error;
    float last_error;
    float kp, ki, kd;
    float integral;
} _pid;

float _center = 0, _range = 1;


void Servo_Init(){
    IfxGtm_Tom_Pwm_Driver driver;
    IfxGtm_Tom_Pwm_Config config;

    IfxGtm_Tom_Pwm_initConfig(&config, &MODULE_GTM);

    config.tom = SERVO_TOM_PIN.tom;
    config.tomChannel = SERVO_TOM_PIN.channel;
    config.clock = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
    config.period = SERVO_PERIOD;
    config.dutyCycle = (uint32) (SERVO_1MS_COUNT * 1.5);
    config.signalLevel = Ifx_ActiveState_high;
    config.synchronousUpdateEnabled = TRUE;     // avoid changing in the middle
    config.pin.outputPin = &SERVO_TOM_PIN;

    IfxGtm_Tom_Pwm_init(&driver, &config);

    IfxGtm_Tom_Pwm_start(&driver, TRUE);
}

void Servo_Set(float angle){
    angle = (angle > 1) ? 1 : ((angle < -1) ? -1 : angle);
    angle = 1.5 + _center + angle * _range;
    IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[SERVO_TOM_PIN.tom], SERVO_TOM_PIN.channel,
            (uint16) (SERVO_1MS_COUNT * angle));
}

void Servo_SetCenter(float angle){
    _center = angle;
    return;
}

void Motor_Init() {
    IfxGtm_Tom_Pwm_Driver driverPositive, driverNegative;
    IfxGtm_Tom_Pwm_Config config;

    /* Initialize the configuration structure with default parameters */
    IfxGtm_Tom_Pwm_initConfig(&config, &MODULE_GTM);

    config.tom = MOTOR_OUTB_TOM_PIN.tom;
    config.tomChannel = MOTOR_OUTB_TOM_PIN.channel;
    config.clock = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
    config.period = MOTOR_PERIOD;
    config.dutyCycle = 0;
    config.signalLevel = Ifx_ActiveState_low;
    config.synchronousUpdateEnabled = TRUE;     // avoid changing in the middle
    config.pin.outputPin = &MOTOR_OUTB_TOM_PIN;

    IfxGtm_Tom_Pwm_init(&driverPositive, &config);

    config.tom = MOTOR_OUTA_TOM_PIN.tom;
    config.tomChannel = MOTOR_OUTA_TOM_PIN.channel;
    config.pin.outputPin = &MOTOR_OUTA_TOM_PIN;

    IfxGtm_Tom_Pwm_init(&driverNegative, &config);

    IfxGtm_Tom_Pwm_start(&driverPositive, TRUE);
    IfxGtm_Tom_Pwm_start(&driverNegative, TRUE);
}

void Motor_Set(float power) {
    power = (power > 1) ? 1 : ((power < -1) ? -1 : power);
    if (MOTOR_REVERSE){
        if (power < 0) {
            IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[MOTOR_OUTB_TOM_PIN.tom], MOTOR_OUTB_TOM_PIN.channel,
                    (uint16) (MOTOR_PERIOD * -power));
            IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[MOTOR_OUTA_TOM_PIN.tom], MOTOR_OUTA_TOM_PIN.channel, 0);
        } else {
            IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[MOTOR_OUTB_TOM_PIN.tom], MOTOR_OUTB_TOM_PIN.channel, 0);
            IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[MOTOR_OUTA_TOM_PIN.tom], MOTOR_OUTA_TOM_PIN.channel,
                    (uint16) (MOTOR_PERIOD * power));
        }
    }
    else{
        if (power < 0) {
            IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[MOTOR_OUTB_TOM_PIN.tom], MOTOR_OUTB_TOM_PIN.channel, 0);
            IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[MOTOR_OUTA_TOM_PIN.tom], MOTOR_OUTA_TOM_PIN.channel,
                    (uint16) (MOTOR_PERIOD * -power));
        } else {
            IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[MOTOR_OUTB_TOM_PIN.tom], MOTOR_OUTB_TOM_PIN.channel,
                    (uint16) (MOTOR_PERIOD * power));
            IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[MOTOR_OUTA_TOM_PIN.tom], MOTOR_OUTA_TOM_PIN.channel, 0);
        }
    }
}

void PID_Init(float kp, float ki, float kd) {
    _pid.target_speed = 0.0;
    _pid.current_speed = 0.0;
    _pid.error = 0.0;
    _pid.last_error = 0.0;
    _pid.integral = 0.0;

    _pid.kp = kp;
    _pid.ki = ki;
    _pid.kd = kd;
}

void PID_SetParams(float kp, float ki, float kd) {
    _pid.kp = kp;
    _pid.ki = ki;
    _pid.kd = kd;
}

float PID_Output(float target_speed, float current_speed) {
    _pid.target_speed = target_speed;
    _pid.current_speed = current_speed;

    _pid.error = _pid.target_speed - _pid.current_speed;
    _pid.integral += _pid.error;

    float output = _pid.kp * _pid.error + _pid.ki * _pid.integral + _pid.kd * (_pid.error - _pid.last_error);

    _pid.last_error = _pid.error;
    return output;
}
