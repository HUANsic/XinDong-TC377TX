#include "Bluetooth.h"
#include "IfxAsclin_Asc.h"
#include "Interrupts.h"

IfxAsclin_Asc ble_handler;

//	reference code: TC264_XinDong_Demo_v51/Src/UART.c

uint8 *ble_rx_ptr;
uint8 ble_rx_tag;
uint32 ble_rx_length, ble_rx_length_got;
Ifx_SizeT ble_rx_count_this;

uint8 ble_rx_buffer[BLE_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8], ble_tx_buffer[BLE_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];

void Bluetooth_Init() {
	/* Initialize an instance of IfxAsclin_Asc_Config with default values */
	IfxAsclin_Asc_Config ascConfig;
	IfxAsclin_Asc_initModuleConfig(&ascConfig, BLE_TX_ASCLIN_PIN.module);

	/* Set the desired baud rate */
	ascConfig.baudrate.baudrate = BLE_BAUDRATE; /* Set the baud rate in bit/s */
	ascConfig.baudrate.oversampling = IfxAsclin_OversamplingFactor_16; /* Set the oversampling factor */

	/* Configure the sampling mode */
	ascConfig.bitTiming.medianFilter = IfxAsclin_SamplesPerBit_three; /* Set the number of samples per bit */
	ascConfig.bitTiming.samplePointPosition = IfxAsclin_SamplePointPosition_8; /* Set the first sample position */

	/* ISR priorities and interrupt target */
	ascConfig.interrupt.txPriority = BLE_TX_PRIORITY;
	ascConfig.interrupt.rxPriority = BLE_RX_PRIORITY;
	ascConfig.interrupt.erPriority = BLE_ERR_PRIORITY;
	ascConfig.interrupt.typeOfService = BLE_TOS;

	/* FIFO configuration */
	ascConfig.txBuffer = ble_tx_buffer;
	ascConfig.txBufferSize = 256;
	ascConfig.rxBuffer = ble_rx_buffer;
	ascConfig.rxBufferSize = 256;
	// ascConfig.fifo.txFifoInterruptLevel = IfxAsclin_TxFifoInterruptLevel_0; /* txFifoInterruptLevel = 0. optimised to write upto 16 bytes at a time */
	// ascConfig.fifo.rxFifoInterruptLevel = IfxAsclin_RxFifoInterruptLevel_1; /* interrupt on every byte */

	/* Pin configuration */
	IfxAsclin_Asc_Pins pins = { NULL_PTR, IfxPort_InputMode_pullUp,
			&BLE_RX_ASCLIN_PIN, IfxPort_InputMode_pullUp,
			NULL_PTR, IfxPort_OutputMode_pushPull,
			&BLE_TX_ASCLIN_PIN, IfxPort_OutputMode_pushPull,
			IfxPort_PadDriver_cmosAutomotiveSpeed1
	};
	ascConfig.pins = &pins;

	/* Initialize module with above parameters */
	IfxAsclin_Asc_initModule(&ble_handler, &ascConfig);

	/* Initialize AT pin */
	IfxPort_setPinMode(BLE_AT_PORT, BLE_AT_PIN, IfxPort_Mode_outputPushPullGeneral);
	IfxPort_setPinState(BLE_AT_PORT, BLE_AT_PIN, IfxPort_State_high);
}

uint8 Bluetooth_Transmit(uint8 *dataptr, Ifx_SizeT length) {
	if (length > BLE_BUFFER_SIZE)
		return 2;
	if (Ifx_Fifo_writeCount(ble_handler.tx) < length)
		return 1;
	// make sure it only writes if it need not wait, i.e. all can be written in the sw fifo
	IfxAsclin_Asc_write(&ble_handler, dataptr, &length, 10);
	return 0;
}

uint8 Bluetooth_Receive(uint8 *dataptr, uint32 length, uint8 tag) {
	if (ble_rx_tag)
		return 1;	// must abort previous ones before starting a new reception
	if (!dataptr)
		return 2;
	if (!length)
		return 3;
	if (!tag)
		return 4;	// not allowed to override default tag
	ble_rx_ptr = dataptr;
	ble_rx_length = length;	// record the number of bytes to receive
	ble_rx_count_this = ble_rx_length > 240 ? 240 : (sint16) ble_rx_length;
	ble_rx_length_got = 0;
	ble_rx_tag = tag;
	return 0;
}

uint8 Bluetooth_Receive_Abort() {
	uint8 tmptag = ble_rx_tag;
	uint8 *tmpptr = ble_rx_ptr;
	Ifx_SizeT bytes_read;
	uint32 tmplengthgot = ble_rx_length_got, tmplengthremaining = ble_rx_length - ble_rx_length_got;
	// these variables MUST change to default before calling Bluetooth_Received in case it calls Bluetooth_Receive
	ble_rx_tag = 0;
	ble_rx_ptr = 0;
	ble_rx_length = 1;
	ble_rx_count_this = 1;
	ble_rx_length_got = 0;
	// if Bluetooth_Receive was called (registered), call Bluetooth_Received as the end of reception
	if (tmptag) {
		bytes_read = Ifx_Fifo_readCount(ble_handler.rx);
		bytes_read = bytes_read < tmplengthremaining ? bytes_read : (sint16) tmplengthremaining;
		IfxAsclin_Asc_read(&ble_handler, tmpptr + tmplengthgot, &bytes_read, 10);
		tmplengthgot += bytes_read;
		// Ifx_Fifo_flush(ble_handler.rx, 0);
		Ifx_Fifo_clear(ble_handler.rx);
		Bluetooth_Received(tmpptr, tmplengthgot, tmptag);
	} else {
		Ifx_Fifo_clear(ble_handler.rx);
	}
	return tmptag;
}

void Bluetooth_AT(uint8 enter) {
	if (enter) {
		IfxPort_setPinState(BLE_AT_PORT, BLE_AT_PIN, IfxPort_State_low);
	} else {
		IfxPort_setPinState(BLE_AT_PORT, BLE_AT_PIN, IfxPort_State_high);
	}
}

void BLE_Tx_ISR(void) {
	IfxAsclin_Asc_isrTransmit(&ble_handler);
}

void BLE_Rx_ISR(void) {
	uint8 buffer;
	uint8 tmptag = ble_rx_tag;
	uint8 *tmpptr = ble_rx_ptr;
	Ifx_SizeT bytes_read;
	uint32 tmplength = ble_rx_length;

	// receive the byte and push it into fifo
	IfxAsclin_Asc_isrReceive(&ble_handler);

	// see how many byte are in the fifo, if it is equal to the registered length, run callback function, or if it is full, record partial data
	bytes_read = Ifx_Fifo_readCount(ble_handler.rx);
	if (ble_rx_tag) {
		if (bytes_read >= ble_rx_count_this) {
			// read bytes
			IfxAsclin_Asc_read(&ble_handler, ble_rx_ptr + ble_rx_length_got, &ble_rx_count_this, 10);
			// update variables
			ble_rx_length_got += ble_rx_count_this;
			// see if there are any bytes to be received
			if (ble_rx_length > ble_rx_length_got) {
				// wait for next batch
				ble_rx_count_this = ble_rx_length - ble_rx_length_got > 240 ? 240 : (sint16) (ble_rx_length - ble_rx_length_got);
			} else {
				// done!
				// these variables MUST change to default before calling Bluetooth_Received in case it calls Bluetooth_Receive
				tmpptr = ble_rx_ptr;
				tmplength = ble_rx_length;
				tmptag = ble_rx_tag;
				ble_rx_tag = 0;
				ble_rx_ptr = 0;
				ble_rx_length = 1;
				ble_rx_count_this = 1;
				ble_rx_length_got = 0;
				// run callback function
				Bluetooth_Received(tmpptr, tmplength, tmptag);
			}
		}
	}
	// otherwise, receive one byte at a time and pass default parameters
	else {
		bytes_read = 1;
		IfxAsclin_Asc_read(&ble_handler, &buffer, &bytes_read, 10);
		Bluetooth_Received(&buffer, 1, 0);
	}
}

void BLE_Err_ISR(void) {
	IfxAsclin_Asc_isrError(&ble_handler);
}

__weak__ void Bluetooth_Received(uint8 *dataptr, uint32 length, uint8 tag) {
	(void) dataptr;
	(void) length;
	(void) tag;
}
