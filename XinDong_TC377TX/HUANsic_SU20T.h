/*
 * HUANsic_SU20T.h
 *
 *  Created on: 2025年8月31日
 *      Author: abcde
 */

#ifndef HUANSIC_SU20T_H_
#define HUANSIC_SU20T_H_

#include <Ifx_Types.h>

void SU20T_Init(void);

uint8 SU20T_Check(void);

void SU20T_Decode(uint8 *msg);

void SU20T_EatByte(uint8 data);

void SU20T_100ms_Isr(void);

void SU20T_SetTurning(sint8 dir);

void SU20T_SetBreaking(uint8 brk);

void SU20T_SetLowBeam(uint8 on);

void SU20T_SetTail(uint8 on);

#endif /* HUANSIC_SU20T_H_ */
