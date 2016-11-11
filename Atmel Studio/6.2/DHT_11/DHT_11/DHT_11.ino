#include <VirtualWire.h>
#include <dht.h>
//
//    FILE: dht11_test.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.01
// PURPOSE: DHT library test sketch for DHT11 && Arduino
//     URL:
//
// Released to the public domain
//

dht DHT;

#define DHT11_PIN 5
#define LM35PIN A0
#define LEDPIN	13

uint16_t reading = 0;
uint8_t	count = 1;
float tempC;

void setup()
{
	Serial.begin(115200);
	Serial.println("DHT TEST PROGRAM ");
	Serial.print("LIBRARY VERSION: ");
	Serial.println(DHT_LIB_VERSION);
	Serial.println();
	Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
	
	pinMode(LEDPIN,OUTPUT);
	vw_set_ptt_inverted(true);
	vw_set_tx_pin(12);
	vw_setup(2000);// speed of data transfer Kbps
	
	analogReference(INTERNAL);
	
	reading = analogRead(LM35PIN);
}

void loop()
{
	// READ DATA
	Serial.print("DHT11, \t");
	int chk = DHT.read11(DHT11_PIN);
	switch (chk)
	{
		case DHTLIB_OK:
		Serial.print("OK,\t");
		break;
		case DHTLIB_ERROR_CHECKSUM:
		Serial.print("Checksum error,\t");
		break;
		case DHTLIB_ERROR_TIMEOUT:
		Serial.print("Time out error,\t");
		break;
		case DHTLIB_ERROR_CONNECT:
		Serial.print("Connect error,\t");
		break;
		case DHTLIB_ERROR_ACK_L:
		Serial.print("Ack Low error,\t");
		break;
		case DHTLIB_ERROR_ACK_H:
		Serial.print("Ack High error,\t");
		break;
		default:
		Serial.print("Unknown error,\t");
		break;
	}
	// DISPLAY DATA
	Serial.print(DHT.humidity, 1);
	Serial.print(",\t");
	Serial.println(DHT.temperature, 1);
	
	//Print lm35 data
	reading = analogRead(LM35PIN);
	
	 tempC = reading/9.31;
	 Serial.print("TEMP LM35: ");
	 Serial.print(tempC,2);
	 Serial.println("C");
	 
	 uint8_t IntegerPart = (uint8_t)tempC;
	 uint8_t DecPart = 100 * (tempC - IntegerPart);
	 
	 uint8_t msg[2] = {IntegerPart,DecPart};

	 digitalWrite(LEDPIN, HIGH); // Flash a light to show transmitting
	 vw_send(msg, 1);
	 vw_wait_tx(); // Wait until the whole message is gone
	 digitalWrite(LEDPIN, LOW);

	delay(500);
}
