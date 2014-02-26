/*
 * Library to drive a HD44780 compatible display connected to I2C (aka TWI) bus
 * using a PCF8574 remote 8-bit I/O expander.
 *
 * The display should be connected like this:
 *
 * PCF8574 | HD44780
 *      P0 = D4
 *      P1 = D5
 *      P2 = D6
 *      P3 = D7
 *      P4 = RS
 *      P5 = RW
 *      P6 = EN
 *      P7 = Backlight switch (using BC327)
 *
 * Commands / data is sent to the I/O expander and the display is driven in
 * 4-bit mode. This library does NOT implement check of the busy flag!
 *
 * Partly based on Peter Fleury's LCD library.
 */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stdlib.h>

#include "lcd.h"
#include "I2CBase.h"

/*
 * local functions
 */

/*************************************************************************
 delay loop for small accurate delays: 16-bit counter, 4 cycles/loop
*************************************************************************/
static inline void _delayFourCycles(unsigned int __count)
{
    if ( __count == 0 )
        __asm__ __volatile__( "rjmp 1f\n 1:" );    // 2 cycles
    else
        __asm__ __volatile__ (
            "1: sbiw %0,1" "\n\t"
            "brne 1b"                              // 4 cycles/loop
            : "=w" (__count)
            : "0" (__count)
           );
}


/*************************************************************************
delay for a minimum of <us> microseconds
the number of loops is calculated at compile-time from MCU clock frequency
*************************************************************************/

#define delay(us)  _delayFourCycles( ( ( 1*(F_CPU/4000) )*us)/1000 )


#define PCF8574ADDR 0x40

#define I2C_RS _BV(0)
#define I2C_RW _BV(1)
#define I2C_EN _BV(2)
#define I2C_BACKLIGHT _BV(3)


/*************************************************************************
Sends one byte to the display using I2C bus.
Input:   data = byte to be sent, df = flag if data (1) or command (0)
Returns: none
*************************************************************************/
static void
lcd_write(unsigned char data, unsigned char df)
{
    unsigned char dataBits;
	
	I2C_SendStartAndSelect(PCF8574ADDR);
	
	
    dataBits = data & 0xf0;          // Output high nibble
    if (df)
        dataBits |= I2C_RS;   
		
	dataBits |= I2C_BACKLIGHT;
	 
    I2C_SendByte(dataBits);
    I2C_SendByte(dataBits | I2C_EN);
    I2C_SendByte(dataBits);

    dataBits = (data << 4) & 0xf0;                 // Output low nibble
    if (df)
        dataBits |= I2C_RS;
    
	dataBits |= I2C_BACKLIGHT;
	
    I2C_SendByte(dataBits);
    I2C_SendByte(dataBits | I2C_EN);
    I2C_SendByte(dataBits);

    I2C_Stop();
	I2C_WaitTillStopWasSent();
    delay(64);                              // Minimum execution time
}

/*
 * PUBLIC FUNCTIONS
 */

/*************************************************************************
Send LCD controller instruction command
Input:   instruction to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_command(unsigned char cmd)
{
    lcd_write(cmd, 0);
}

/*************************************************************************
Send data byte to LCD controller
Input:   data to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_data(unsigned char data)
{
    lcd_write(data, 1);
}

/*************************************************************************
Clear display
*************************************************************************/
void lcd_clrscr(void)
{
    lcd_command(1<<LCD_CLR);
    delay(1500);
}

/*************************************************************************
Display string without auto linefeed
Input:    string to be displayed
Returns:  none
*************************************************************************/
void lcd_puts(const char *s)
{
    while ( (*s) )
        lcd_data(*s++);
}

/*************************************************************************
Display string from program memory without auto linefeed
Input:     string from program memory be be displayed
Returns:   none
*************************************************************************/
void lcd_puts_p(const char *progmem_s)
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) ) {
        lcd_data(c);
    }
}


/*************************************************************************
Initialize I2C and display
*************************************************************************/
void lcd_init(void)
{
    I2C_Init();
	uint8_t dataBits;	

    delay(16000);               /* wait 16ms or more after power-on */

    I2C_SendStartAndSelect(PCF8574ADDR);
	
	dataBits = _BV(LCD_FUNCTION) | _BV(LCD_FUNCTION_8BIT);
	
	I2C_SendByte(dataBits);
	I2C_SendByte(dataBits | I2C_EN);
	I2C_SendByte(dataBits);	    
    delay(4992);
	
    I2C_SendByte(dataBits | I2C_EN);
    I2C_SendByte(dataBits);   
    delay(64);
	
    I2C_SendByte(dataBits | I2C_EN);
    I2C_SendByte(dataBits);
    delay(64);
	
	dataBits = _BV(LCD_FUNCTION);
	
    I2C_SendByte(dataBits);
    I2C_SendByte(dataBits | I2C_EN);
    I2C_SendByte(dataBits);

	I2C_Stop();
	I2C_WaitTillStopWasSent();

    lcd_command(LCD_FUNCTION_4BIT_2LINES);  /* function set: display lines */
    lcd_command(LCD_DISP_ON);   /* Display on, Cursor on, Blink off */
    lcd_clrscr();
}
