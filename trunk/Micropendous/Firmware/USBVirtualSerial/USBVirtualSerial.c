/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Modified for maximum STDIO throughput by Opendous Inc. 2011-02-27
  For more information visit: www.Micropendous.org/USBVirtualSerial

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the VirtualSerial demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "USBVirtualSerial.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber         = 0,

				.DataINEndpointNumber           = CDC_TX_EPNUM,
				.DataINEndpointSize             = CDC_TXRX_EPSIZE,
				.DataINEndpointDoubleBank       = false,

				.DataOUTEndpointNumber          = CDC_RX_EPNUM,
				.DataOUTEndpointSize            = CDC_TXRX_EPSIZE,
				.DataOUTEndpointDoubleBank      = false,

				.NotificationEndpointNumber     = CDC_NOTIFICATION_EPNUM,
				.NotificationEndpointSize       = CDC_NOTIFICATION_EPSIZE,
				.NotificationEndpointDoubleBank = false,
			},
	};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs
 */
static FILE USBSerialStream;

/* Global buffer for use with STDIO functions */
volatile char buffer[CDC_TXRX_EPSIZE];

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();

	for (;;)
	{
		MainTask();

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	// Disable watchdog if enabled by bootloader/fuses
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// Disable clock division
	clock_prescale_set(clock_div_1);

	// initialize the buffer to be used with STDIO functions
	uint8_t i;
	for (i = 0; i < CDC_TXRX_EPSIZE; i++){
		buffer[i] = 0;
	}

	/* disable JTAG to allow corresponding pins to be used - PF4, PF5, PF6, PF7 */
	/* TODO - remove this if you want to use your JTAG debugger to debug this firmware */
	#if (defined(__AVR_AT90USB1287__) || defined(__AVR_AT90USB647__) ||  \
			defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB646__) ||  \
			defined(__AVR_ATmega16U4__)  || defined(__AVR_ATmega32U4__) ||  \
			defined(__AVR_ATmega32U6__))
		// note the JTD bit must be written twice within 4 clock cycles to disable JTAG
		// you must also set the IVSEL bit at the same time, which requires IVCE to be set first
		// port pull-up resistors are enabled - PUD(Pull Up Disable) = 0
		MCUCR = (1 << JTD) | (1 << IVCE) | (0 << PUD);
		MCUCR = (1 << JTD) | (0 << IVSEL) | (0 << IVCE) | (0 << PUD);
	#endif

	/* Hardware Initialization */
	/* enable Ports based on which IC is being used */
	/* For more information look over the corresponding AVR's datasheet in the
		'I/O Ports' Chapter under subheading 'Ports as General Digital I/O' */
	#if (defined(__AVR_AT90USB162__) || defined(__AVR_AT90USB82__) || \
			defined(__AVR_ATmega16U2__) || defined(__AVR_ATmega32U2__))
		DDRD = 0;
		PORTD = 0xFF;
		DDRB = 0;
		PORTB = 0xFF;
		DDRC = 0;
		PORTC |= (1 << PC2) | (1 << PC4) | (1 << PC5) | (1 << PC6) | (1 << PC7); //only PC2,4,5,6,7 are pins
		// be careful using PortC as PC0 is used for the Crystal and PC1 is nRESET
	#endif

	#if (defined(__AVR_ATmega16U4__)  || defined(__AVR_ATmega32U4__))
		DDRD = 0;
		PORTD = 0xFF;
		DDRB = 0;
		PORTB = 0xFF;
		DDRC = 0;
		PORTC = (1 << PC6) | (1 << PC7); //only PC6,7 are pins
		DDRE = 0;
		PORTE = (1 << PE2) | (1 << PE6); //only PE2,6 are pins
		DDRF = 0;
		PORTF = (1 << PF0) | (1 << PF1) | (1 << PF4) | (1 << PF5) | (1 << PF6) | (1 << PF7); // only PF0,1,4,5,6,7 are pins
	#endif

	#if (defined(__AVR_AT90USB1287__) || defined(__AVR_AT90USB647__) ||  \
			defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB646__) ||  \
			defined(__AVR_ATmega32U6__))
		DDRA = 0;
		PORTA = 0xFF;
		DDRB = 0;
		PORTB = 0xFF;
		DDRC = 0;
		PORTC = 0xFF;
		DDRD = 0;
		PORTD = 0xFF;
		DDRE = 0;
		PORTE = 0xFF;
		DDRF = 0;
		PORTF = 0xFF;
		#if (BOARD == BOARD_MICROPENDOUS)
		// set PortB pin 1 to an output as it connects to an LED on the Micropendous
		DDRB |= (1 << PB1);
		// Set PE4=1 to disable external SRAM, PE6=0 to disable TXB0108, PE7=1 to select USB-B connector
		DDRE |= ((1 << PE4) | (1 << PE6) | (1 << PE7));
		PORTE |= ((1 << PE4) | (1 << PE7));
		PORTE &= ~(1 << PE6);
		#else // other boards such as the Micropendous3 or Micropendous4
		// Set PE6=1 to disable external SRAM
		DDRE |= (1 << PE6);
		PORTE |= (1 << PE6);
		#endif
	#endif

	// Hardware Initialization
	LEDs_Init();
	DISABLE_VOLTAGE_TXRX;
	DISABLE_EXT_SRAM;
	SELECT_USB_B;
	USB_Init();
}


/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}



/** Event handler for the CDC Class driver Line Encoding Changed event.
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
	/* TODO - Use Virtual Serial Port settings to control your application.
		Think of this as a sideband channel.  Use it for control/status messaging.
		Leave the data channel (MainTask and sdio.h calls) for data only.
	*/

	switch (CDCInterfaceInfo->State.LineEncoding.ParityType)
	{
		case CDC_PARITY_Odd:
			// do something here
			break;
		case CDC_PARITY_Even:
			// and/or maybe here
			break;
		case CDC_PARITY_None:
			// maybe something here
			break;
		case CDC_PARITY_Mark:
			// something here could work
			break;
		case CDC_PARITY_Space:
			// you guessed it, something could go here
			break;
	}


	switch (CDCInterfaceInfo->State.LineEncoding.CharFormat)
	{
		case CDC_LINEENCODING_OneStopBit:
			// do something here
			break;
		case CDC_LINEENCODING_OneAndAHalfStopBits:
			// and/or maybe do something here
			break;
		case CDC_LINEENCODING_TwoStopBits:
			// something here could work
			break;
	}


	switch (CDCInterfaceInfo->State.LineEncoding.DataBits)
	{
		case 6:
			// possibly something
			break;
		case 7:
			// possibly something
			break;
		case 8:
			// possibly something
			break;
	}


	switch (CDCInterfaceInfo->State.LineEncoding.BaudRateBPS)
	{
		case 9600:
			// possibly something
			break;
		case 14400:
			// possibly something
			break;
		case 19200:
			// possibly something
			break;
		case 38400:
			// possibly something
			break;
		case 57600:
			// possibly something
			break;
		case 115200:
			// possibly something
			break;
	}

}


/** CDC class driver event for a control line state change on a CDC interface. This event fires each time the host requests a
 *  control line state change (containing the virtual serial control line states, such as DTR). The new control line states
 *  are available in the ControlLineStates.HostToDevice value inside the CDC interface structure passed as a parameter, set as
 *  a mask of CDC_CONTROL_LINE_OUT_* masks.  1 is for 'Set'(Low) and 0 is for 'Clear'(High) as these are active low signals.
 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state
*/
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
	/* TODO - Use Virtual Serial Port settings to control your application.
		Think of this as a sideband channel.  Use it for control/status messaging.
		Leave the data channel (MainTask and sdio.h calls) for data only.
	*/
	if ((CDCInterfaceInfo->State.ControlLineStates.HostToDevice) & CDC_CONTROL_LINE_OUT_RTS) {
		// Host has set the RTS line - '0'
	} else {
		// Host has cleared the RTS line - '1'
	}

	if ((CDCInterfaceInfo->State.ControlLineStates.HostToDevice) & CDC_CONTROL_LINE_OUT_DTR) {
		// Host has set the DTR line - '0'
	} else {
		// Host has cleared the DTR line - '1'
	}
}


/** CDC class driver event for a send break request sent to the device from the host. This is
 *  generally used to separate data or to indicate a special condition to the receiving device.
 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state
 *  \param[in] Duration	Duration of the break that has been sent by the host, in milliseconds.
 */
void EVENT_CDC_Device_BreakSent(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo, const uint8_t Duration)
{
	/* TODO - Use Virtual Serial Port break signal to control your application.
		Think of this as a sideband channel.  Use it for control/status messaging.
		Leave the data channel (MainTask and sdio.h calls) for data only.
	*/
}



/* MainTask will run once initialization is complete */
/* TODO - place your application code here */
void MainTask(void)
{
	int count = 0;

	// If the host has sent data then echo it back

	// Throughput for char reads/writes maxes out at 1 kbyte/second as USB requests are spaced about 1ms apart
/*	if ((temp = fgetc(&USBSerialStream)) >= 0) {
		fputc(temp, &USBSerialStream);
	}
*/

	// Throughput is maximized if the full EP buffer is read and sent each time
	// Throughput approaches CDC_TXRX_EPSIZE kbytes/second and depends on transfer size from host 
	if ((count = fread(&buffer, 1, CDC_TXRX_EPSIZE, &USBSerialStream)) > 0) {
		fwrite(&buffer, 1, count, &USBSerialStream);
	}


	// Example for using formatted standard IO functions
/*	while (haveData()) {
		// as long as getchar() is returning data, process it
		printf_P(PSTR("\r\nUSBVirtualSerial\r\n")); // send a string that is constant and stored in FLASH
		tempInt = getchar(); // receive a character
		printf("Received char = %3d\r\n", tempInt); // send a string that is dynamic and stored in SRAM
		// avoid long dynamic strings as there is little SRAM
	}
*/

	// If the DTR, DSR, ... lines have changed, need to tell the host
	// For non-USBtoSerial usage, use this as a sideband data channel
	if (0) {

		// The DCD line state has been set high
		if (0) {
			(VirtualSerial_CDC_Interface.State.ControlLineStates.DeviceToHost) |= CDC_CONTROL_LINE_IN_DCD;
		}

		// The DSR line state has been set high
		if (0) {
			(VirtualSerial_CDC_Interface.State.ControlLineStates.DeviceToHost) |= CDC_CONTROL_LINE_IN_DSR;
		}

		// The BREAK line state has been set high
		if (0) {
			(VirtualSerial_CDC_Interface.State.ControlLineStates.DeviceToHost) |= CDC_CONTROL_LINE_IN_BREAK;
		}

		// The RING line state has been set high
		if (0) {
			(VirtualSerial_CDC_Interface.State.ControlLineStates.DeviceToHost) |= CDC_CONTROL_LINE_IN_RING;
		}

		// A framing error has occurred
		if (0) {
			(VirtualSerial_CDC_Interface.State.ControlLineStates.DeviceToHost) |= CDC_CONTROL_LINE_IN_FRAMEERROR;
		}

		// A parity error has occurred
		if (0) {
			(VirtualSerial_CDC_Interface.State.ControlLineStates.DeviceToHost) |= CDC_CONTROL_LINE_IN_PARITYERROR;
		}

		// A data overrun error has occurred
		if (0) {
			(VirtualSerial_CDC_Interface.State.ControlLineStates.DeviceToHost) |= CDC_CONTROL_LINE_IN_OVERRUNERROR;
		}

		// Tell the host
		CDC_Device_SendControlLineStateChange(&VirtualSerial_CDC_Interface);
	}

} // MainTask
