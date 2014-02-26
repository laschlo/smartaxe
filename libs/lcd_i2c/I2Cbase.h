#ifndef _I2C_Base
#define _I2C_Base

#include <stdint.h>
#include <util/twi.h>

#define I2CBUSCLOCK		100000UL

#define I2C_OK          0
#define I2C_STARTError	1
#define I2C_NoNACK		3
#define I2C_NoACK		4

extern uint8_t I2C_Error;
inline void I2C_SetError(uint8_t err) { I2C_Error=err;};

void I2C_Init();
void I2C_Start();
static inline void I2C_Stop() {TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);};	//Nadaj sygna³ STOP
void I2C_SetBusSpeed(uint16_t speed);	//Ustaw prêdkoœæ magistrali I2C (Prêdkoœæ wyniesie speed*100[Hz])
void I2C_SendAddr(uint8_t address);		//Wyœlij adres urz¹dzenia slave
void I2C_SendByte(uint8_t byte);		//Wyœlij bajt danych do slave
uint8_t I2C_ReceiveData_ACK();			//Odbierz bajt danych od slave, wysy³aj¹c ACK
uint8_t I2C_ReceiveData_NACK();			//Odbierz bajt danych od slave, nie wysy³aj ACK
static inline void I2C_WaitForComplete() {while (!(TWCR & _BV(TWINT)));};		//Zaczekaj na koniec bie¿¹cej operacji I2C
static inline void I2C_WaitTillStopWasSent() {while (TWCR & _BV(TWSTO));};	//Zaczekaj na koniec nadawania sygna³u STOP
void I2C_SendStartAndSelect(uint8_t addr);				//Wyœlij START i adres urz¹dzenia slave

#endif
