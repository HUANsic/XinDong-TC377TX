#ifndef XINDONGLIB_XINDONG_CONFIG_H_
#define XINDONGLIB_XINDONG_CONFIG_H_

#include <Ifx_Types.h>
#include <IfxPort.h>

#define		__weak__				__attribute__((weak))

#define		PID_PERIOD_MS			50

/*	Pin Assignments */
// potentiometer analog input: POT1-AN10, POT2-AN11, POT3-AN13
#define		POT1_VADC_PIN			IfxEvadc_G1CH2_AN10_IN
#define		POT2_VADC_PIN			IfxEvadc_G1CH3_AN11_IN
#define		POT3_VADC_PIN			IfxEvadc_G1CH4_AN12_IN

// reed switch: P15.8
#define		REED_TIM_PIN			IfxGtm_TIM1_2_P15_8_IN
#define		REED_EXTI_PIN			IfxScu_REQ5A_P15_8_IN

// ultrasonic: echo-P21.5, trig-P21.6
#define		ULTRA_ECHO_TIM_PIN		IfxGtm_TIM0_3_P21_5_IN
#define		ULTRA_TRIG_PORT			&MODULE_P21
#define		ULTRA_TRIG_PIN			6

// led output: led1-P20.10, led2-P20.9, led3-P20.8, led4-P20.7
#define		IO_LED1_PORT			&MODULE_P20
#define		IO_LED1_PIN				10
#define		IO_LED2_PORT			&MODULE_P20
#define		IO_LED2_PIN				9
#define		IO_LED3_PORT			&MODULE_P20
#define		IO_LED3_PIN				8
#define		IO_LED4_PORT			&MODULE_P20
#define		IO_LED4_PIN				7

// DIP switch input: dip1-P20.14, dip2-P20.13, dip3-P20.12, dip4-P20.11
#define		IO_DIP1_PORT			&MODULE_P20
#define		IO_DIP1_PIN				14
#define		IO_DIP2_PORT			&MODULE_P20
#define		IO_DIP2_PIN				13
#define		IO_DIP3_PORT			&MODULE_P20
#define		IO_DIP3_PIN				12
#define		IO_DIP4_PORT			&MODULE_P20
#define		IO_DIP4_PIN				11

// battery 1S signal input: P15.0
#define		IO_BATT_PORT			&MODULE_P15
#define		IO_BATT_PIN				0

// bluetooth: AT-P33.10, TX-P33.9, RX-P33.8
#define		BLE_BUFFER_SIZE			256
#define		BLE_BAUDRATE			115200
#define		BLE_AT_PORT				&MODULE_P33
#define		BLE_AT_PIN				10
#define		BLE_TX_ASCLIN_PIN		IfxAsclin2_TX_P33_9_OUT
#define		BLE_RX_ASCLIN_PIN		IfxAsclin2_RXE_P33_8_IN

// serial: TX-P15.6, RX-P15.7
#define		SERIAL_BUFFER_SIZE		256
#define		SERIAL_BAUDRATE			115200
#define		SERIAL_TX_ASCLIN_PIN	IfxAsclin3_TX_P15_6_OUT
#define		SERIAL_RX_ASCLIN_PIN	IfxAsclin3_RXA_P15_7_IN

// servo: P33.4
#define		SERVO_TOM_PIN			IfxGtm_TOM0_0_TOUT26_P33_4_OUT

// motor: OUTA-P21.3, OUTB-P21.2
#define		MOTOR_OUTA_TOM_PIN		IfxGtm_TOM2_1_TOUT54_P21_3_OUT
#define		MOTOR_OUTB_TOM_PIN		IfxGtm_TOM2_0_TOUT53_P21_2_OUT

// encoder: A-P33.7, B-P33.6
#define		ENCODER_A_GPT12_PIN		IfxGpt120_T2INB_P33_7_IN
#define		ENCODER_B_GPT12_PIN		IfxGpt120_T2EUDB_P33_6_IN

// oled and imu: SCL-P13.1, SDA-P13.2
#define		I2C_SCL_I2C_PIN			IfxI2c0_SCL_P13_1_INOUT
#define		I2C_SDA_I2C_PIN			IfxI2c0_SDA_P13_2_INOUT
#define		MPU_AD0_PORT			&MODULE_P13
#define		MPU_AD0_PIN				0

// camera SCCB: SCL-P00.1, SDA-P00.2
#define		CAM_SCL_PORT			&MODULE_P00
#define		CAM_SCL_PIN				1
#define		CAM_SDA_PORT			&MODULE_P00
#define		CAM_SDA_PIN				2

// camera CIF data: D0-P02.0, D1-P02.1, D2-P02.2, D3-P02.3, D4-P02.4, D5-P02.5, D6-P02.6, D7-P02.7
#define		CAM_D0_CIF_PORT			IfxCif_D0_P02_0_IN
#define		CAM_D1_CIF_PORT			IfxCif_D1_P02_1_IN
#define		CAM_D2_CIF_PORT			IfxCif_D2_P02_2_IN
#define		CAM_D3_CIF_PORT			IfxCif_D3_P02_3_IN
#define		CAM_D4_CIF_PORT			IfxCif_D4_P02_4_IN
#define		CAM_D5_CIF_PORT			IfxCif_D5_P02_5_IN
#define		CAM_D6_CIF_PORT			IfxCif_D6_P02_6_IN
#define		CAM_D7_CIF_PORT			IfxCif_D7_P02_7_IN

// camera CIF hardware control: PCLK-P00.7, VSYNC-P00.8, HSYNC-P00.9
#define		CAM_PCLK_HW_CIF_PIN		IfxCif_PCLK_P00_7_IN
#define		CAM_VSYNC_HW_CIF_PIN	IfxCif_VSYNC_P00_8_IN
#define		CAM_HSYNC_HW_CIF_PIN	IfxCif_HSYNC_P00_9_IN

// camera CIF software control: PCLK-P11.10, VSYNC-P15.4, HSYNC-P10.3
#define		CAM_PCLK_SW_EXTI_PIN	IfxScu_REQ6D_P11_10_IN
#define		CAM_VSYNC_SW_EXTI_PIN	IfxScu_REQ0A_P15_4_IN
#define		CAM_HSYNC_SW_EXTI_PIN	IfxScu_REQ3A_P10_3_IN

// camera CIF general control: PWDN-P00.0, FLASH-P02.8, RESET-P00.3
#define		CAM_PWDN_PORT			&MODULE_P00
#define		CAM_PWDN_PIN			0
#define		CAM_FLASH_PORT			&MODULE_P02
#define		CAM_FLASH_PIN			8
#define		CAM_RESET_PORT			&MODULE_P00
#define		CAM_RESET_PIN			3

#endif		// XINDONGLIB_XINDONG_CONFIG_H_

/*
 * So the total RAM space is about 200k bytes, each 120x188 images takes up 45k bytes, for now we use three buffers, which takes 135k bytes
 */
