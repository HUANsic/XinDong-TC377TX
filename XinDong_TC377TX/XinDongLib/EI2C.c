#include "EI2C.h"

/*
 * 	Use shifted address!!! e.g. 0xA0
 */

//void _EI2C_SCL_Rise(EI2C_Typedef *ei2c);
//void _EI2C_SCL_Fall(EI2C_Typedef *ei2c);
//void _EI2C_SDA_High(EI2C_Typedef *ei2c);
//void _EI2C_SDA_Low(EI2C_Typedef *ei2c);
//uint8 _EI2C_SDA_State(EI2C_Typedef *ei2c);
//uint8 _EI2C_SCL_State(EI2C_Typedef *ei2c);
//void _EI2C_Hold(EI2C_Typedef *ei2c);
//void _EI2C_Start(EI2C_Typedef *ei2c);
//void _EI2C_Restart(EI2C_Typedef *ei2c);
//void _EI2C_Stop(EI2C_Typedef *ei2c);
//void _EI2C_SendBit(EI2C_Typedef *ei2c, uint8 bit);
//uint8 _EI2C_ReadBit(EI2C_Typedef *ei2c);
//uint8 _EI2C_SendByte(EI2C_Typedef *ei2c, uint8 byte);
//uint8 _EI2C_ReadByte(EI2C_Typedef *ei2c, uint8 ack);
inline void _EI2C_SCL_Rise(EI2C_Typedef *ei2c) {
	IfxPort_setPinState(ei2c->scl_port, ei2c->scl_pin, IfxPort_State_high);
}

inline void _EI2C_SCL_Fall(EI2C_Typedef *ei2c) {
	IfxPort_setPinState(ei2c->scl_port, ei2c->scl_pin, IfxPort_State_low);
}

inline void _EI2C_SDA_High(EI2C_Typedef *ei2c) {
	IfxPort_setPinState(ei2c->sda_port, ei2c->sda_pin, IfxPort_State_high);
}

inline void _EI2C_SDA_Low(EI2C_Typedef *ei2c) {
	IfxPort_setPinState(ei2c->sda_port, ei2c->sda_pin, IfxPort_State_low);
}

inline uint8 _EI2C_SDA_State(EI2C_Typedef *ei2c) {
	return IfxPort_getPinState(ei2c->sda_port, ei2c->sda_pin);
}

inline uint8 _EI2C_SCL_State(EI2C_Typedef *ei2c) {
	return IfxPort_getPinState(ei2c->scl_port, ei2c->scl_pin);
}

void _EI2C_Hold() {
	uint32 i;
	/*
	 EI2C_Hold:	.type	func	instruction cycle count (total cycle count)
	 mov	d0,#0				1(1)
	 .L283:
	 j	.L2						1(2)
	 .L3:
	 NOP						1((n-1)*30+3)
	 .L383:
	 NOP						1((n-1)*30+4)
	 .L384:
	 NOP						1((n-1)*30+5)
	 .L385:
	 NOP						1((n-1)*30+6)
	 .L386:
	 NOP						1((n-1)*30+7)
	 .L387:
	 NOP						1((n-1)*30+8)
	 .L388:
	 NOP						1((n-1)*30+9)
	 .L389:
	 NOP						1((n-1)*30+10)
	 .L390:
	 NOP						1((n-1)*30+11)
	 .L391:
	 NOP						1((n-1)*30+12)
	 .L392:
	 NOP						1((n-1)*30+13)
	 .L393:
	 NOP						1((n-1)*30+14)
	 .L394:
	 NOP						1((n-1)*30+15)
	 .L395:
	 NOP						1((n-1)*30+16)
	 .L396:
	 NOP						1((n-1)*30+17)
	 .L397:
	 NOP						1((n-1)*30+18)
	 .L398:
	 NOP						1((n-1)*30+19)
	 .L399:
	 NOP						1((n-1)*30+20)
	 .L400:
	 NOP						1((n-1)*30+21)
	 .L401:
	 NOP						1((n-1)*30+22)
	 .L402:
	 NOP						1((n-1)*30+23)
	 .L403:
	 NOP						1((n-1)*30+24)
	 .L404:
	 NOP						1((n-1)*30+25)
	 .L405:
	 NOP						1((n-1)*30+26)
	 .L406:
	 NOP						1((n-1)*30+27)
	 .L407:
	 NOP						1((n-1)*30+28)
	 .L408:
	 NOP						1((n-1)*30+29)
	 .L409:
	 add	d0,#1				1(n*30)
	 .L2:
	 mov	d15,#100			1(n*30+1)
	 .L410:
	 jlt.u	d0,d15,.L3			1~3(n*30+2)(n*30+4)
	 .L411:
	 ret						1(n*30+5)
	 */
	for (i = 0; i < 22; i++) {		// 100kHz if i=100
		// place [CPU_CLK_MHz/10 - 3] number of NOPs in this loop
		// then each cycle is approximately 100ns
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");
//		__asm("NOP");

//		__asm("NOP");
//		__asm("NOP");
		__asm("NOP");
		__asm("NOP");
		__asm("NOP");

		__asm("NOP");
		__asm("NOP");
	}
	(void) i;
}

/*          ___         ___ ___
 *  SCL        |       |
 *             |___ ___|
 *
 *          ___ ___ ___ ___
 *  SDA            |       |
 *          ___ ___|       |___
 */
void _EI2C_Start(EI2C_Typedef *ei2c) {
	/// can not assume line is in proper state: SDA = SCL = PullUp
	_EI2C_Hold();
	_EI2C_SCL_Fall(ei2c);
	_EI2C_Hold();
	_EI2C_SDA_High(ei2c);
	_EI2C_Hold();
	_EI2C_SCL_Rise(ei2c);
	_EI2C_Hold();
	_EI2C_SDA_Low(ei2c);
	_EI2C_Hold();
}

/*          ___         ___ ___ ___ ___
 *  SCL        |       |
 *             |___ ___|
 *
 *          ___ ___         ___ ___
 *  SDA            |       |       |
 *          ___ ___|___ ___|       |___
 */
void _EI2C_Restart(EI2C_Typedef *ei2c) {
	/// can not assume line is in proper state: SDA = SCL = PullUp
	_EI2C_Hold();
	_EI2C_SCL_Fall(ei2c);
	_EI2C_Hold();
	_EI2C_SDA_Low(ei2c);
	_EI2C_Hold();
	_EI2C_SCL_Rise(ei2c);
	_EI2C_Hold();
	_EI2C_SDA_High(ei2c);
	_EI2C_Hold();
	_EI2C_Hold();
	_EI2C_SDA_Low(ei2c);
	_EI2C_Hold();
}

/*          ___         ___ ___
 *  SCL        |       |
 *             |___ ___|
 *
 *          ___ ___         ___
 *  SDA            |       |
 *          ___ ___|___ ___|
 */
void _EI2C_Stop(EI2C_Typedef *ei2c) {
	// can not assume line is in proper state: SDA = SCL = PullUp
	_EI2C_Hold();
	_EI2C_SCL_Fall(ei2c);
	_EI2C_Hold();
	_EI2C_SDA_Low(ei2c);
	_EI2C_Hold();
	_EI2C_SCL_Rise(ei2c);
	_EI2C_Hold();
	_EI2C_SDA_High(ei2c);
	_EI2C_Hold();
}

/*          ___         ___
 *  SCL        |       |
 *             |___ ___|
 *
 *          ___ ___ ___ ___
 *  SDA            |
 *          ___ ___|___ ___
 */
void _EI2C_SendBit(EI2C_Typedef *ei2c, uint8 bit) {
	_EI2C_Hold();
	_EI2C_SCL_Fall(ei2c);
	_EI2C_Hold();
	if (bit)
		_EI2C_SDA_High(ei2c);
	else
		_EI2C_SDA_Low(ei2c);
	_EI2C_Hold();
	_EI2C_SCL_Rise(ei2c);
	_EI2C_Hold();
}

/*          ___         ___
 *  SCL        |       |
 *             |___ ___|
 *
 *          ___ ___ ___ ___
 *  SDA            |
 *          ___ ___|___ ___
 *                     ^
 *                Sample Here
 */
uint8 _EI2C_ReadBit(EI2C_Typedef *ei2c) {
	uint8 bit;
	_EI2C_Hold();
	_EI2C_SCL_Fall(ei2c);
	_EI2C_Hold();
	_EI2C_SDA_High(ei2c);    // release line
	_EI2C_Hold();
	_EI2C_SCL_Rise(ei2c);
	while (!_EI2C_SCL_State(ei2c))
		__nop();       // enable clock stretching
	bit = _EI2C_SDA_State(ei2c);
	_EI2C_Hold();
	return bit;
}

/*           ___ ___ ___ ___ ___ ___ ___ ___ ___
 *          |   |   |   |   |   |   |   |   |
 *  I2C     | T | T | T | T | T | T | T | T | R
 *          |___|___|___|___|___|___|___|___|___
 */
uint8 _EI2C_SendByte(EI2C_Typedef *ei2c, uint8 byte) {
	uint8 i;
	for (i = 0; i < 8; i++) {
		_EI2C_SendBit(ei2c, byte & 0x80);
		byte <<= 1;
	}
	return _EI2C_ReadBit(ei2c);
}

/*           ___ ___ ___ ___ ___ ___ ___ ___ ___
 *          |   |   |   |   |   |   |   |   |
 *  I2C     | R | R | R | R | R | R | R | R | T
 *          |___|___|___|___|___|___|___|___|___
 */
uint8 _EI2C_ReadByte(EI2C_Typedef *ei2c, uint8 ack) {
	uint8 i, retVal = 0;
	for (i = 0; i < 8; i++) {
		retVal <<= 1;
		retVal |= _EI2C_ReadBit(ei2c) ? 1 : 0;
	}
	_EI2C_SendBit(ei2c, !ack);
	return retVal;
}

void EI2C_Init(EI2C_Typedef *ei2c) {
	IfxPort_setPinMode(ei2c->scl_port, ei2c->scl_pin, IfxPort_Mode_outputOpenDrainGeneral);
	IfxPort_setPinMode(ei2c->sda_port, ei2c->sda_pin, IfxPort_Mode_outputOpenDrainGeneral);
	_EI2C_SCL_Rise(ei2c);
	_EI2C_SDA_High(ei2c);
}

/*           ___ ____ ___ ____ ___ ___ ____ ___ ___ ___ ___ ____ ___
 *          |   |    |   |    |   |   |    |   |   |   |   |    |
 *  Master  | S |AD+W|   | RA |   | S |AD+R|   |   |ACK|   |NACK| P
 *          |___|____|___|____|___|___|____|___|___|___|___|____|___
 *          |   |    |   |    |   |   |    |   |   |   |   |    |
 *  Slave   |   |    |ACK|    |ACK|   |    |ACK| D |   | D |    |
 *          |___|____|___|____|___|___|____|___|___|___|___|____|___
 */
EI2C_Status EI2C_Mem_Read(EI2C_Typedef *ei2c, uint8 slaveAddress, uint8 MemAddress, uint8 *pData, uint16 size) {
	_EI2C_Start(ei2c);
	if (_EI2C_SendByte(ei2c, slaveAddress)) {
		_EI2C_Stop(ei2c);
		ei2c->status = EI2C_NO_DEVICE;
		return EI2C_NO_DEVICE;       // no device
	}
	if (_EI2C_SendByte(ei2c, MemAddress)) {
		_EI2C_Stop(ei2c);
		ei2c->status = EI2C_INVALID_ADDRESS;
		return EI2C_INVALID_ADDRESS;       // invalid address
	}
	_EI2C_Restart(ei2c);
	if (_EI2C_SendByte(ei2c, slaveAddress | 1)) {
		_EI2C_Stop(ei2c);
		ei2c->status = EI2C_NO_DEVICE;
		return EI2C_NO_DEVICE;       // no device
	}
	while (--size) {        // keep the last one
		*pData = _EI2C_ReadByte(ei2c, 1);
		pData++;
	}
	*pData = _EI2C_ReadByte(ei2c, 0);
	_EI2C_Stop(ei2c);
	ei2c->status = EI2C_OK;
	return EI2C_OK;
}

/*           ___  ____ ___  ____ ___  ____ ___  ____ ___  ___
 *          |    |    |    |    |    |    |    |    |    |
 *  Master  | S  |AD+W|    | RA |    | D  |    | D  |    | P
 *          |___ |____|___ |____|___ |____|___ |____|___ |___
 *          |    |    |    |    |    |    |    |    |    |
 *  Slave   |    |    |ACK |    |ACK |    |ACK |    |ACK |
 *          |___ |____|___ |____|___ |____|___ |____|___ |___
 */
EI2C_Status EI2C_Mem_Write(EI2C_Typedef *ei2c, uint8 slaveAddress, uint8 MemAddress, uint8 *pData, uint16 size) {
	_EI2C_Start(ei2c);
	if (_EI2C_SendByte(ei2c, slaveAddress)) {
		_EI2C_Stop(ei2c);
		ei2c->status = EI2C_NO_DEVICE;
		return EI2C_NO_DEVICE;       // no device
	}
	if (_EI2C_SendByte(ei2c, MemAddress)) {
		_EI2C_Stop(ei2c);
		ei2c->status = EI2C_INVALID_ADDRESS;
		return EI2C_INVALID_ADDRESS;       // invalid address
	}
	while (size--) {
		if (_EI2C_SendByte(ei2c, *pData)) {
			_EI2C_Stop(ei2c);
			ei2c->status = EI2C_ACK_MISSING;
			return EI2C_ACK_MISSING;   // acknowledge missing
		}
		pData++;
	}
	_EI2C_Stop(ei2c);
	ei2c->status = EI2C_OK;
	return EI2C_OK;
}

EI2C_Status EI2C_Call(EI2C_Typedef *ei2c, uint8 slaveAddress) {
	_EI2C_Start(ei2c);
	if (_EI2C_SendByte(ei2c, slaveAddress)) {
		_EI2C_Stop(ei2c);
		ei2c->status = EI2C_NO_DEVICE;
		return EI2C_NO_DEVICE;       // no device
	}
	_EI2C_Stop(ei2c);
	ei2c->status = EI2C_OK;
	return EI2C_OK;
}
