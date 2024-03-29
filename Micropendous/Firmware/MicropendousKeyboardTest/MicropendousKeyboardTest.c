/*
             LUFA Library
     Copyright (C) Dean Camera, 2013.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2013  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Modified for MicropendousKeyboardTest by Opendous Inc. 2013-03-16 (www.micropendous.org)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
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
 *  Main source file for the Keyboard demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "MicropendousKeyboardTest.h"

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
 	{
		.Config =
			{
				.InterfaceNumber              = 0,
				.ReportINEndpoint             =
					{
						.Address      = KEYBOARD_EPADDR,
						.Size         = KEYBOARD_EPSIZE,
						.Banks        = 1,
					},
				.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
			},
    };


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
	{
		HID_Device_USBTask(&Keyboard_HID_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Disable JTAG to allow testing port F */
	#if (	defined(__AVR_AT90USB1287__) || defined(__AVR_AT90USB647__) ||  \
			defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB646__) ||  \
			defined(__AVR_ATmega16U4__)  || defined(__AVR_ATmega32U4__) ||  \
			defined(__AVR_ATmega32U6__)	)
		JTAG_DISABLE();
	#endif

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
	#endif

	/* Hardware Initialization */
	LEDs_Init();
	LEDs_TurnOnLEDs(LEDS_ALL_LEDS);
	Buttons_Init();
	DISABLE_VOLTAGE_TXRX(); // used on Micropendous REV1/2 boards
	DISABLE_EXT_SRAM(); // used on Micropendous REV1/2 boards
	SELECT_USB_B(); // needed for Micropendous REV1/2 boards
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

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);

	USB_Device_EnableSOFEvents();

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean true to force the sending of the report, false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

	//uint8_t JoyStatus_LCL    = Joystick_GetStatus();
	//uint8_t ButtonStatus_LCL = Buttons_GetStatus();

	uint8_t UsedKeyCodes = 0;

	// 32-pin USB AVRs
	#if (defined(__AVR_AT90USB162__) || defined(__AVR_AT90USB82__) || \
			defined(__AVR_ATmega16U2__) || defined(__AVR_ATmega32U2__))
				  if (~PINB & (1 << PINB0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PINB & (1 << PINB1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		} else if (~PINB & (1 << PINB2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PINB & (1 << PINB3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PINB & (1 << PINB4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PINB & (1 << PINB5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PINB & (1 << PINB6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINB & (1 << PINB7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;

		} else if (~PINC & (1 << PINC2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PINC & (1 << PINC4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PINC & (1 << PINC5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PINC & (1 << PINC6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINC & (1 << PINC7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;

		} else if (~PIND & (1 << PIND0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PIND & (1 << PIND1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		} else if (~PIND & (1 << PIND2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PIND & (1 << PIND3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PIND & (1 << PIND4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PIND & (1 << PIND5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PIND & (1 << PIND6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PIND & (1 << PIND7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;
		}
	#endif // end 32-pin USB AVRs


	// 44-pin USB AVRs
	#if (defined(__AVR_ATmega16U4__)  || defined(__AVR_ATmega32U4__))
				  if (~PINB & (1 << PINB0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PINB & (1 << PINB1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		} else if (~PINB & (1 << PINB2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PINB & (1 << PINB3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PINB & (1 << PINB4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PINB & (1 << PINB5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PINB & (1 << PINB6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINB & (1 << PINB7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;

		} else if (~PINC & (1 << PINC6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINC & (1 << PINC7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;

		} else if (~PIND & (1 << PIND0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PIND & (1 << PIND1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		} else if (~PIND & (1 << PIND2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PIND & (1 << PIND3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PIND & (1 << PIND4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PIND & (1 << PIND5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PIND & (1 << PIND6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PIND & (1 << PIND7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;

		} else if (~PINE & (1 << PINE2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PINE & (1 << PINE6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;

		} else if (~PINF & (1 << PINF0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PINF & (1 << PINF1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		} else if (~PINF & (1 << PINF4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PINF & (1 << PINF5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PINF & (1 << PINF6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINF & (1 << PINF7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;
		}
	#endif // end 44-pin USB AVRs


	// 64-pin USB AVRs
	#if (defined(__AVR_AT90USB1287__) || defined(__AVR_AT90USB647__) ||  \
			defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB646__) ||  \
			defined(__AVR_ATmega32U6__))

                       if (~PINB & (1 << PINB0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		// On Micropendous boards pin B1 is the LED so it cannot be read
		#if ((BOARD != BOARD_MICROPENDOUS_REV1) && (BOARD != BOARD_MICROPENDOUS_REV2))
		} else if (~PINB & (1 << PINB1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		#endif // BOARD != MICROPENDOUS_REV1/2
		} else if (~PINB & (1 << PINB2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PINB & (1 << PINB3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PINB & (1 << PINB4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PINB & (1 << PINB5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PINB & (1 << PINB6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINB & (1 << PINB7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_B;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;

		} else if (~PIND & (1 << PIND0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PIND & (1 << PIND1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		} else if (~PIND & (1 << PIND2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PIND & (1 << PIND3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PIND & (1 << PIND4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PIND & (1 << PIND5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PIND & (1 << PIND6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PIND & (1 << PIND7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_D;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;

		} else if (~PINF & (1 << PINF0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PINF & (1 << PINF1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		} else if (~PINF & (1 << PINF2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PINF & (1 << PINF3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PINF & (1 << PINF4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PINF & (1 << PINF5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PINF & (1 << PINF6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINF & (1 << PINF7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_F;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;


		} else if (~PINE & (1 << PINE2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;


		// On Micropendous boards Ports A, C, and most of E control various board functions and are not exposed
		#if ((BOARD != BOARD_MICROPENDOUS_REV1) && (BOARD != BOARD_MICROPENDOUS_REV2))
		} else if (~PINA & (1 << PINA0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_A;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PINA & (1 << PINA1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_A;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		} else if (~PINA & (1 << PINA2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_A;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PINA & (1 << PINA3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_A;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PINA & (1 << PINA4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_A;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PINA & (1 << PINA5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_A;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PINA & (1 << PINA6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_A;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINA & (1 << PINA7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_A;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;

		} else if (~PINC & (1 << PINC0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PINC & (1 << PINC1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;
		} else if (~PINC & (1 << PINC2)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_2_AND_AT;
		} else if (~PINC & (1 << PINC3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PINC & (1 << PINC4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PINC & (1 << PINC5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PINC & (1 << PINC6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINC & (1 << PINC7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_C;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;

		} else if (~PINE & (1 << PINE0)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
		} else if (~PINE & (1 << PINE1)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_1_AND_EXCLAMATION;

		} else if (~PINE & (1 << PINE3)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_3_AND_HASHMARK;
		} else if (~PINE & (1 << PINE4)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_4_AND_DOLLAR;
		} else if (~PINE & (1 << PINE5)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_5_AND_PERCENTAGE;
		} else if (~PINE & (1 << PINE6)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_6_AND_CARET;
		} else if (~PINE & (1 << PINE7)) {
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_E;
			KeyboardReport->KeyCode[UsedKeyCodes++] = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;
		#endif // BOARD != MICROPENDOUS_REV1/2

		}

	#endif // end 64-pin USB AVRs

	//if (UsedKeyCodes)
	//  KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;

	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	uint8_t  LEDMask   = LEDS_NO_LEDS;
	uint8_t* LEDReport = (uint8_t*)ReportData;

	if (*LEDReport & HID_KEYBOARD_LED_NUMLOCK)
	  LEDMask |= LEDS_LED1;

	if (*LEDReport & HID_KEYBOARD_LED_CAPSLOCK)
	  LEDMask |= LEDS_LED3;

	if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK)
	  LEDMask |= LEDS_LED4;

	LEDs_SetAllLEDs(LEDMask);
}

