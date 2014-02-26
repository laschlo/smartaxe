/*
 * spi.h
 *
 *  Created on: 24-Jan-2009
 *      Author: Neil MacMillan
 *
 *  Macros and prototypes for using the AT90 as an SPI master.
 *
 *  References:
 *    Engelke, Stefan.  AVR-Lib/SPI.  http://www.tinkerer.eu/AVRLib/SPI
 *    Tanner, Justin.  LED Wheel: Code.  http://webhome.csc.uvic.ca/~mcheng/samples/led_wheel/wheel_demo.zip
 *    Atmel.  AT90USB1287 Hardware Manual.  http://www.atmel.com/dyn/resources/prod_documents/doc7593.pdf
 *
 *    All the hard work for this was done by [Engelke].
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdio.h>
#include <avr/io.h>
#include <stdlib.h>

#define SPI_DDR		DDRB	// DDR of SPI port
#define SPI_PORT	PORTB	// SPI port
#define SPI_MOSI	PB3	// MOSI pin (Master out, Slave in)
#define SPI_MISO	PB4	// MISO pin (Master in, Slave out)
#define SPI_SCK		PB5	// SCK pin (SPI clock)
#define SPI_SS		PB2 //CSN/SS pin (Chip Select-Not)

/**
 * Initialize the SPI port as a master.
 * This function must be called once before using the SPI interface.
 */
void SPI_Init();

/**
 * Write a block of data to the slave, and read the data returned from the slave into a buffer.
 * The data and buffer memory blocks must be the same length.
 * \param data A pointer to the contiguous memory block to write to the slave.
 * \param buffer A pointer to the contiguous memory block to which the SPI data are to be written.
 * \param len The length of the memory blocks, in bytes.
 */
void SPI_ReadWrite_Block (uint8_t * data, uint8_t * buffer, uint8_t len);

/**
 * Write a block to the slave.
 * \param data A pointer to the contiguous memory block to write to the slave.
 * \param len The length of the block to write, in bytes.
 */
void SPI_Write_Block (uint8_t * data, uint8_t len);

/**
 * Write a byte to the slave, and get the return byte from the slave.
 */
uint8_t SPI_Write_Byte (uint8_t data);



#endif /* SPI_H_ */