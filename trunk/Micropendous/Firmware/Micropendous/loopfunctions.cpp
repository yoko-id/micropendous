#include "WProgram.h"
/*
 Micropendous

 Enumerates as a Composite USB Device - Keyboard and USB Virtual Serial Port
 Demonstrates Virtual USB Serial Port loopback using Arduino code
 Also flashes and fades a LED connected to pin 8 - PD0
 Demonstrates multi-tasking ability of LUFAduino by using two loop functions and delays
 GND'ing any pin will type the corresponding pin's name

 The circuit:
 Any USB AVR with > 2kbytes SRAM - ATmega32U4, ATmega32U6, AT90USB???6/7

 created 2009-10-03 by Opendous Inc. - based on Fading.pde from Arduino
 (C) under the GNU LGPL v2 license, see LICENSE.txt

 http://www.Micropendous.org/Micropendous
*/

int ledPin =  0;	// connect an LED to pin 0 - PD0


void setup(void)
{
	// Wiring Interface Initialization
	Serial.begin(115200);

	pinMode(ledPin, OUTPUT);
}



void loop(void)
{
	if ((Serial.enumerated() > 0) && (Serial.available() > 0))
	{
		// Echo back recevied characters to the host
		Serial.write(Serial.read());
	}
}



void loop2(void)
{
	digitalWrite(ledPin, HIGH);   // set the LED on
	delay(500);                  // wait for a second
	digitalWrite(ledPin, LOW);    // set the LED off
	delay(500);                  // wait for a second

	// fade in from min to max in increments of 5 points:
	for(int fadeValue = 0 ; fadeValue <= 255; fadeValue +=5) { 
		// sets the value (range from 0 to 255):
		analogWrite(ledPin, fadeValue);         
		// wait for 50 milliseconds to see the dimming effect    
		delay(50);                            
	}

	// fade out from max to min in increments of 5 points:
	for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -=5) { 
		// sets the value (range from 0 to 255):
		analogWrite(ledPin, fadeValue);         
		// wait for 50 milliseconds to see the dimming effect    
		delay(50);                            
	}
}