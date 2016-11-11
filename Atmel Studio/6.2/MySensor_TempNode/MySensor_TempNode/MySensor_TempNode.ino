// Example sketch showing how to send in OneWire temperature readings
#include <MySensor.h>
#include <SPI.h>

#define NODE_ID	11
#define LM35_ID	1
#define LM35_PIN A0

unsigned long SLEEP_TIME = 60000; // Sleep time between reads (in milliseconds)

MySensor gw(9,10); //CE=9 CSN=10

// Initialize temperature message
MyMessage msg(LM35_ID,V_TEMP);

float temp;
uint16_t reading;
void setup()

{	
	analogReference(INTERNAL);
	reading = analogRead(LM35_PIN); //for some reason first reading is always bad
	
	gw.begin(incomingMessage, NODE_ID, false);//just in case I need something from gateway. this won't be battery powered sensor
	gw.sendSketchInfo("Temperature Sensor", "1.0");	
	gw.present(LM35_ID, S_TEMP); //present sensor to controller
}

void loop()

{
	/*without this st=fail happens all the time
	* It just needs to clear in-buffer somehow?
	* without it gateway does not even get ack messages (i guess?)
	*/
	gw.process();
	
	
	reading = analogRead(LM35_PIN);
	temp = reading/9.31;
	
	//#ifdef DEBUG
	//Serial.print("TEMPERATURE: ");
	//Serial.print(temp);
	//Serial.print("*C");
	//Serial.println();
	//#endif
	
	
	gw.send(msg.setSensor(LM35_ID).set(temp,1));	
	gw.sleep(SLEEP_TIME);	
		
}

void incomingMessage(const MyMessage &message) {
}