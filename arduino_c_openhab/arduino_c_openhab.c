/*
 * arduino_c_openhab.c
 *
 * Created: 1/18/2014 8:20:49 PM
 *  Author: SG0891784
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#define NEWLINESTR "\r\n"

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include "uart.h"
#include "radio.h"
#include "stations.h"


//#include "lcd_i2c/lcd.h"


#define TRUE		1
#define FALSE		0


void add_char( char *s, char  znak){
	unsigned char i;
	
	for(i=0;s[i];i++);
	s[i] = znak;
	s[i+1] = 0;
	
}

uint8_t my_addr[5] = ADDRESS_CONTROLLER;
uint8_t station_addr[5] = ADDRESS_KITCHEN; // Transmitter address
	
radiopacket_t packet; 
volatile uint8_t rxFlag = 0;
RADIO_RX_STATUS rx_status;


int main(void)
{
	uart_init((UART_BAUD_SELECT((BAUD),F_CPU)));
	sei();
	
	Radio_Init();

	// configure the receive settings for radio pipe 0
	Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
	
	// configure radio transceiver settings.
	Radio_Configure(RADIO_1MBPS, RADIO_HIGHEST_POWER);

		
	uint8_t licznik=1;
	char buffer[10];
	uint8_t zn;
	char frame[20];
	char * device[10];
	
	uint8_t device_addr;
	uint8_t device_pwm;
	uint8_t device_lights;
	int16_t device_temp;
	

    while(1)
    {		
		
		if (rxFlag)
		{
			rx_status = Radio_Receive(&packet); // Copy received packet to memory and store the result in rx_status.
			if (rx_status == RADIO_RX_SUCCESS || rx_status == RADIO_RX_MORE_PACKETS) // Check if a packet is available.
			{							
				if (packet.type != MESSAGE)
				{
					//Put an error message
				}
				
				device_addr = packet.payload.message.address[4];											
				device_lights = packet.payload.message.messagecontent.light;
				device_pwm = packet.payload.message.messagecontent.pwm;									
				device_temp = packet.payload.message.messagecontent.temperature;				
				
				frame[0]=0;
				strcat(frame,itoa(device_addr,buffer,10));
				strcat(frame,";");
				strcat(frame,itoa(device_temp,buffer,10));
				strcat(frame,";");
				strcat(frame,itoa(device_lights,buffer,10));
				strcat(frame,";");
				strcat(frame,itoa(device_pwm,buffer,10));
				uart_puts(frame);
												
			}
			rxFlag = 0;  // clear the flag.									
		}				
		
		
		if(uart_available()){			
			frame[0]=0;						
			zn = uart_getc();			
			while((zn != '#') && (zn != UART_NO_DATA) ){
				add_char(frame,zn);
				zn = uart_getc();
			}			
			
			char * p;
			licznik = 0;
			p = strtok(frame,";");
			while (p){
				device[licznik] = p;
				p = strtok(NULL,";");
				licznik++;
			}	
			
			device_addr = atoi(device[0]);
			device_lights = atoi(device[1]);
			device_pwm = atoi(device[2]);
			
			//send packet to the station with last byte address set in device_addr
			
			//Set TX address to arduino connected to Remote station
			station_addr[4] = device_addr;
			Radio_Set_Tx_Addr(station_addr);
			
			packet.type = MESSAGE;
			memcpy(packet.payload.message.address, my_addr, RADIO_ADDRESS_LENGTH);
			packet.payload.message.messageid = 56;
			
			packet.payload.message.messagecontent.light = device_lights;
			packet.payload.message.messagecontent.pwm = device_pwm;
					
			if (Radio_Transmit(&packet, RADIO_WAIT_FOR_TX) == RADIO_TX_MAX_RT) // Transmit packet.
			{
				uart_puts("MAX NUMBER OF RETRIES");
			}
			else // Transmitted succesfully.
			{
				//uart_puts("Retransmission count: ");
				//uart_puts(itoa(Radio_Resend_Rate(),buffer,10));
				//uart_puts(NEWLINESTR);
				//return RADIO_TX_SUCCESS;
			}
			
		}		
	}
}

void radio_rxhandler(){
	rxFlag = 1;
}
