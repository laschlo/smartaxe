/*
 * stations.h
 *
 * Created: 2/15/2014 6:40:23 PM
 *  Author: SG0891784
 */ 


#ifndef STATIONS_H_
#define STATIONS_H_

#define ADDRESS_KITCHEN	{ 0x98, 0x76, 0x54, 0x32, 0x10 }
#define ADDRESS_CONTROLLER { 0xE4, 0xE4, 0xE4, 0xE4, 0xE4 }
	
//define positions in messagecontent
#define MESSAGE_MASK_LIGHTS		0
#define MESSAGE_MASK_PWM		1
#define MESSAGE_MASK_TEMP		2


#endif /* STATIONS_H_ */