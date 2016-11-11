#include <VirtualWire.h>
/*
 * Receiver433.ino
 *
 * Created: 5/14/2015 11:54:26 AM
 * Author: SG0891784
 */ 

#define RECEIVE_PIN	12

void setup()
{
	delay(1000);
	Serial.begin(9600);
	Serial.println("SETUP");
	
	vw_set_rx_pin(RECEIVE_PIN);
	vw_set_ptt_inverted(true);
	vw_setup(2000);
	
	vw_rx_start();
		
}

void loop()
{
	uint8_t buf[VW_MAX_MESSAGE_LEN];
	uint8_t buflen = VW_MAX_MESSAGE_LEN;
	if (vw_get_message(buf, &buflen)) // Non-blocking
	{
		uint8_t i;
		
		// Message with a good checksum received, dump it.
		Serial.print("Got: ");
		     
		for (i = 0; i < buflen; i++)
		{
			Serial.print(buf[i]);
			Serial.print(' ');
		}
		Serial.println();
			
	 }
}
