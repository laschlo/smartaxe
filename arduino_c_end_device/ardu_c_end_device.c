/*
 * ardu_c_end_device.c
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
#include "uart/uart.h"
#include "nRF/radio.h"
#include "nRF/stations.h"


#define	BTN_DDR			DDRC
#define BTN_PORT		PORTC
#define	BTN_PINPORT		PINC
#define BTN_PIN			PC0

#define LIGHT1_DDR		DDRC
#define LIGHT1_PORT		PORTC
#define LIGHT1_PIN		PC1
#define LIGHT1_OFF()	(LIGHT1_PORT |= _BV(LIGHT1_PIN))
#define LIGHT1_ON()		(LIGHT1_PORT &= ~_BV(LIGHT1_PIN))


void add_char( char *s, char  znak){
	unsigned char i;
	
	for(i=0;s[i];i++);
	s[i] = znak;
	s[i+1] = 0;
	
}

uint8_t sdigit[3] = {0 , 0 , 0};		
// Implementing integer value from 0 to 255
char *num2hex(unsigned char num)
{
	uint8_t idx;
	char hexval[]={"0123456789ABCDEF"};
	idx = 0;                     // Start with index 0
	while(num >= 16){            // Keep Looping for larger than 16
		idx++;                     // Increase index
		num -= 16;                 // Subtract number with 16
	}      

	sdigit[0]='0';               // Default first Digit to '0'
	if (idx > 0) sdigit[0]=hexval[idx];     // Put the First digit
	sdigit[1]=hexval[num];       // Put the Second Digit
	return sdigit;
}

typedef struct _device{
	uint8_t light;
	uint8_t pwm;
	uint8_t type;
	uint16_t temperature;
}device_t;

static uint8_t send_packet(device_t* tosend);
static void update_coils(void);

uint8_t controller_addr[5] = ADDRESS_CONTROLLER; // Receiver address
uint8_t my_addr[5] = ADDRESS_KITCHEN;

uint8_t rx_status;
radiopacket_t packet;
volatile uint8_t rxFlag = 0;
volatile uint8_t button_pressed;
volatile device_t my_device;


char buffer[10];

int main(void)
{	
	
	uart_init((UART_BAUD_SELECT((BAUD),F_CPU)));	
	sei();
	
	//Set Button
	BTN_DDR &= ~_BV(BTN_PIN);
	BTN_PORT |= _BV(BTN_PIN);
	
	//Enable button interrupt
	PCICR |= _BV(PCIE1);
	PCMSK1 |= _BV(PCINT8);	
	button_pressed = 0;

	
	//Set light1 pin
	LIGHT1_DDR |= _BV(LIGHT1_PIN);
	LIGHT1_OFF();
	my_device.light=0;
	my_device.pwm=255;
	my_device.temperature = 2545;
	
	 Radio_Init();

	 // configure the receive settings for radio pipe 0
	 Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
	 
	 // configure radio transceiver settings.
	 Radio_Configure(RADIO_1MBPS, RADIO_HIGHEST_POWER);	

	//Set TX address to arduino connected to OpenHab
	Radio_Set_Tx_Addr(controller_addr);
	 
	 while(1){		
		 		
		 
		 if(rxFlag){
			 rx_status = Radio_Receive(&packet); // Copy received packet to memory and store the result in rx_status.
			 if (rx_status == RADIO_RX_SUCCESS || rx_status == RADIO_RX_MORE_PACKETS) // Check if a packet is available.
			 {
				 if (packet.type != MESSAGE)				 
				 {
					 //Put an error message
				 }				 						 
				 my_device.light = packet.payload.message.messagecontent.light;
				 my_device.pwm = packet.payload.message.messagecontent.pwm;
				 update_coils();
				 uart_puts("RECEIVED UPDATE:")			 						 						 		;
				 uart_puts(NEWLINESTR);
				 
				 uart_puts("LIGHT: ");
				 uart_puts(itoa(my_device.light,buffer,10));
				 uart_puts(NEWLINESTR);
				 
				 uart_puts("PWM: ");
				 uart_puts(itoa(my_device.pwm,buffer,10));
				 uart_puts(NEWLINESTR);
			 }
			 rxFlag = 0;  // clear the flag.
		 }
		 
		 if(button_pressed){			 
			 uart_puts("Button Pressed!");
			 uart_puts(NEWLINESTR);					 
			 
			 if(my_device.light){				 
				 my_device.light = 0;
				 update_coils();
				 uart_puts("Light1 OFF");
				 uart_puts(NEWLINESTR);
			 }else{				 
				 my_device.light = 1;
				 update_coils();
				 uart_puts("Light1 ON");
				 uart_puts(NEWLINESTR);
			 }			 
			 button_pressed = 0;	
			 					 
			 
			 if(send_packet((device_t*) &my_device) == RADIO_TX_MAX_RT){
				 uart_puts("Packet did not reach target");
				 uart_puts(NEWLINESTR);
				 uart_puts("MAX number of retries!");
				 uart_puts(NEWLINESTR);
			 }else{
				 uart_puts("Message sent.");
				 uart_puts(NEWLINESTR);
			 }
			 
		 }
		 	 		 
	 }
}

// The radio_rxhandler is called by the radio IRQ pin interrupt routine when RX_DR is read in STATUS register.
void radio_rxhandler()
{
	rxFlag = 1;
}


ISR(PCINT1_vect){
	if(!(BTN_PINPORT & _BV(BTN_PIN))){		
		button_pressed = 1;
	}
}

static void update_coils(){
	if(my_device.light){
		LIGHT1_ON();
	}else{
		LIGHT1_OFF();
	}
}

static uint8_t send_packet(device_t* tosend){
	radiopacket_t packet; 
	packet.type = MESSAGE;
	memcpy(packet.payload.message.address, my_addr, RADIO_ADDRESS_LENGTH);
	packet.payload.message.messageid = 55;
	
	packet.payload.message.messagecontent.light = tosend->light;
	packet.payload.message.messagecontent.pwm = tosend->pwm;		
	packet.payload.message.messagecontent.temperature = tosend->temperature;

	
	
	if (Radio_Transmit(&packet, RADIO_WAIT_FOR_TX) == RADIO_TX_MAX_RT) // Transmitt packet.	
	{
		return RADIO_TX_MAX_RT;
	}
	else // Transmitted succesfully.
	{
		//uart_puts("Retransmission count: ");
		//uart_puts(itoa(Radio_Resend_Rate(),buffer,10));
		//uart_puts(NEWLINESTR);
		return RADIO_TX_SUCCESS;
	}
}