#include "Serial.h"
#include "IfxAsclin_Asc.h"
#include "Interrupts.h"

IfxAsclin_Asc serial_handler;

//	reference code: TC264_XinDong_Demo_v51/Src/UART.c

uint8 *_serial_rx_ptr;
uint8 _serial_rx_tag;
uint32 _serial_rx_length, _serial_rx_length_got;
Ifx_SizeT _serial_rx_count_this;

uint8 _serial_rx_buffer[SERIAL_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8], _serial_tx_buffer[SERIAL_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];

void Serial_Init() {
	/* Initialize an instance of IfxAsclin_Asc_Config with default values */
	IfxAsclin_Asc_Config ascConfig;
	IfxAsclin_Asc_initModuleConfig(&ascConfig, SERIAL_TX_ASCLIN_PIN.module);

	/* Set the desired baud rate */
	ascConfig.baudrate.baudrate = SERIAL_BAUDRATE; /* Set the baud rate in bit/s */
	ascConfig.baudrate.oversampling = IfxAsclin_OversamplingFactor_16; /* Set the oversampling factor */

	/* Configure the sampling mode */
	ascConfig.bitTiming.medianFilter = IfxAsclin_SamplesPerBit_three; /* Set the number of samples per bit */
	ascConfig.bitTiming.samplePointPosition = IfxAsclin_SamplePointPosition_8; /* Set the first sample position */

	/* ISR priorities and interrupt target */
	ascConfig.interrupt.txPriority = SERIAL_TX_PRIORITY;
	ascConfig.interrupt.rxPriority = SERIAL_RX_PRIORITY;
	ascConfig.interrupt.erPriority = SERIAL_ERR_PRIORITY;
	ascConfig.interrupt.typeOfService = SERIAL_TOS;

	/* FIFO configuration */
	ascConfig.txBuffer = _serial_tx_buffer;
	ascConfig.txBufferSize = 256;
	ascConfig.rxBuffer = _serial_rx_buffer;
	ascConfig.rxBufferSize = 256;
	// ascConfig.fifo.txFifoInterruptLevel = IfxAsclin_TxFifoInterruptLevel_0; /* txFifoInterruptLevel = 0. optimised to write upto 16 bytes at a time */
	// ascConfig.fifo.rxFifoInterruptLevel = IfxAsclin_RxFifoInterruptLevel_1; /* interrupt on every byte */

	/* Pin configuration */
	IfxAsclin_Asc_Pins pins = { NULL_PTR, IfxPort_InputMode_pullUp,
			&SERIAL_RX_ASCLIN_PIN, IfxPort_InputMode_pullUp,
			NULL_PTR, IfxPort_OutputMode_pushPull,
			&SERIAL_TX_ASCLIN_PIN, IfxPort_OutputMode_pushPull,
			IfxPort_PadDriver_cmosAutomotiveSpeed1
	};
	ascConfig.pins = &pins;

	/* Initialize module with above parameters */
	IfxAsclin_Asc_initModule(&serial_handler, &ascConfig);
}

uint8 Serial_Transmit(uint8 *dataptr, sint16 length) {
	if (length > SERIAL_BUFFER_SIZE)
		return 2;
	if (Ifx_Fifo_writeCount(serial_handler.tx) < length)
		return 1;
	// make sure it only writes if it need not wait, i.e. all can be written in the sw fifo
	IfxAsclin_Asc_write(&serial_handler, dataptr, &length, 10);
	return 0;
}

uint8 Serial_Receive(uint8 *dataptr, uint32 length, uint8 tag) {
	if (_serial_rx_tag)
		return 1;	// must abort previous ones before starting a new reception
	if (!dataptr)
		return 2;
	if (!length)
		return 3;
	if (!tag)
		return 4;	// not allowed to override default tag
	_serial_rx_ptr = dataptr;
	_serial_rx_length = length;	// record the number of bytes to receive
	_serial_rx_count_this = _serial_rx_length > 240 ? 240 : (sint16) _serial_rx_length;
	_serial_rx_length_got = 0;
	_serial_rx_tag = tag;
	return 0;
}

uint8 Serial_Receive_Abort() {
	uint8 tmptag = _serial_rx_tag;
	uint8 *tmpptr = _serial_rx_ptr;
	Ifx_SizeT bytes_read;
	uint32 tmplengthgot = _serial_rx_length_got, tmplengthremaining = _serial_rx_length - _serial_rx_length_got;
	// these variables MUST change to default before calling Serial_Received in case it calls Serial_Receive
	_serial_rx_tag = 0;
	_serial_rx_ptr = 0;
	_serial_rx_length = 1;
	_serial_rx_count_this = 1;
	_serial_rx_length_got = 0;
	// if Serial_Receive was called (registered), call Serial_Received as the end of reception
	if (tmptag) {
		bytes_read = Ifx_Fifo_readCount(serial_handler.rx);
		bytes_read = bytes_read < tmplengthremaining ? bytes_read : (sint16) tmplengthremaining;
		IfxAsclin_Asc_read(&serial_handler, tmpptr + tmplengthgot, &bytes_read, 10);
		tmplengthgot += bytes_read;
		// Ifx_Fifo_flush(serial_handler.rx, 0);
		Ifx_Fifo_clear(serial_handler.rx);
		Serial_Received(tmpptr, tmplengthgot, tmptag);
	} else {
		Ifx_Fifo_clear(serial_handler.rx);
	}
	return tmptag;
}

void Serial_Tx_ISR(void) {
	IfxAsclin_Asc_isrTransmit(&serial_handler);
}

void Serial_Rx_ISR(void) {
	uint8 buffer;
	uint8 tmptag = _serial_rx_tag;
	uint8 *tmpptr = _serial_rx_ptr;
	Ifx_SizeT bytes_read;
	uint32 tmplength = _serial_rx_length;

	// receive the byte and push it into fifo
	IfxAsclin_Asc_isrReceive(&serial_handler);

	// see how many byte are in the fifo, if it is equal to the registered length, run callback function, or if it is full, record partial data
	bytes_read = Ifx_Fifo_readCount(serial_handler.rx);
	if (_serial_rx_tag) {
		if (bytes_read >= _serial_rx_count_this) {
			// read bytes
			IfxAsclin_Asc_read(&serial_handler, _serial_rx_ptr + _serial_rx_length_got, &_serial_rx_count_this, 10);
			// update variables
			_serial_rx_length_got += _serial_rx_count_this;
			// see if there are any bytes to be received
			if (_serial_rx_length > _serial_rx_length_got) {
				// wait for next batch
				_serial_rx_count_this = _serial_rx_length - _serial_rx_length_got > 240 ? 240 : (sint16) (_serial_rx_length - _serial_rx_length_got);
			} else {
				// done!
				// these variables MUST change to default before calling Serial_Received in case it calls Serial_Receive
				tmpptr = _serial_rx_ptr;
				tmplength = _serial_rx_length;
				tmptag = _serial_rx_tag;
				_serial_rx_tag = 0;
				_serial_rx_ptr = 0;
				_serial_rx_length = 1;
				_serial_rx_count_this = 1;
				_serial_rx_length_got = 0;
				// run callback function
				Serial_Received(tmpptr, tmplength, tmptag);
			}
		}
	}
	// otherwise, receive one byte at a time and pass default parameters
	else {
		bytes_read = 1;
		IfxAsclin_Asc_read(&serial_handler, &buffer, &bytes_read, 10);
		Serial_Received(&buffer, 1, 0);
	}
}

void Serial_Err_ISR(void) {
	IfxAsclin_Asc_isrError(&serial_handler);
}

__weak__ void Serial_Received(uint8 *dataptr, uint32 length, uint8 tag) {
	(void) dataptr;
	(void) length;
	(void) tag;
}
