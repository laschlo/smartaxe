/***
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * 
 * DESCRIPTION
 * This sketch provides a Dimmable LED Light using PWM and based Henrik Ekblad 
 * <henrik.ekblad@gmail.com> Vera Arduino Sensor project.  
 * Developed by Bruce Lacey, inspired by Hek's MySensor's example sketches.
 * 
 * The circuit uses a MOSFET for Pulse-Wave-Modulation to dim the attached LED or LED strip.  
 * The MOSFET Gate pin is connected to Arduino pin 3 (LED_PIN), the MOSFET Drain pin is connected
 * to the LED negative terminal and the MOSFET Source pin is connected to ground.  
 *
 * This sketch is extensible to support more than one MOSFET/PWM dimmer per circuit.
 *
 * REVISION HISTORY
 * Version 1.0 - February 15, 2014 - Bruce Lacey
 * Version 1.1 - August 13, 2014 - Converted to 1.4 (hek) 
 *
 ***/
#define SN "DimmableLED"
#define SV "1.1"

#include <MySensor.h> 
#include <SPI.h>
#include <Bounce2.h>

#define LED_PIN 2
#define BUTTON_PIN	3

#define NODE_ID 15
#define CHILD_ID 1

MySensor gw;


MyMessage lightMsg(CHILD_ID, V_LIGHT);
Bounce debouncer = Bounce();
boolean state = false;


/***
 * Dimmable LED initialization method
 */
void setup()  
{ 
  Serial.println( SN ); 
  gw.begin( incomingMessage, NODE_ID, false );
  
  // Register the LED Dimmable Light with the gateway
  gw.present( CHILD_ID, S_LIGHT );
  
  gw.sendSketchInfo(SN, SV);
  // Pull the gateway's current dim level - restore light level upon sendor node power-up
  //gw.request( 0, V_DIMMER );
  
  pinMode(BUTTON_PIN,INPUT);
  digitalWrite(BUTTON_PIN,HIGH);
  
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);
    
}

/***
 *  Dimmable LED main processing loop 
 */
void loop() 
{
  gw.process();  
  if (debouncer.update()){
	  if(!debouncer.read()){
		  if(state){
			  digitalWrite(LED_PIN,HIGH);
			  state = false;
			  Serial.println("Wylaczamy swiatelko");
		  }else{
			  digitalWrite(LED_PIN,LOW);
			  state = true;
			  Serial.println("Wlaczamy swiatelko");

		  }		
		  gw.send(lightMsg.set(state ? 1 : 0));
	  }	  
  }  
}



void incomingMessage(const MyMessage &message) {
  if (message.type == V_LIGHT) {
	  digitalWrite(LED_PIN,message.getBool()? LOW : HIGH);
	  
	  Serial.print("Incoming change for sensor:");
	  Serial.print(message.sensor);
	  Serial.print(", New status: ");
	  Serial.println(message.getBool());
  }
}
