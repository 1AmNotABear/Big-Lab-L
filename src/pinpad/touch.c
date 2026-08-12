#include "touch.h"
#include "lpc24xx.h"
#include "stdio.h"

#define CS_PIN            0x00100000        //P0.20
#define SCLK							(3 << 30)
#define MOSI							(3 << 4)
#define MISO							(3 << 2)
#define CS_TP							(3 << 8)

#define S0SPCR_val 				0x093C
#define S0SPCCR_val				0x24

#define ctrl_x						0xD8
#define ctrl_y						0x98

unsigned char touch_read(unsigned char command);

void touch_init(void)
{
	//Implement this as you see fit
	//Remember to setup CS_TP as a GPIO output
	PINSEL0 &= ~SCLK;
	PINSEL0 |= SCLK;
	
	PINSEL1 &= ~ ( MOSI | MISO | CS_TP );		// automatically set CS_TP as GPIO output after clearing to 0
	PINSEL1 |= ( MOSI | MISO );
	
	FIO0DIR |= CS_PIN;		// config output
	FIO0SET = CS_PIN;		// sets as 1 
	
	S0SPCR = S0SPCR_val;
	S0SPCCR = S0SPCCR_val;
}

void touch_read_xy(char *x, char *y)
{
	//Read X co-ordinate from the touch screen controller
	*x = touch_read(0xD8);
	//Read Y co-ordinate from the touch screen controller
	*y = touch_read(0x98);
}

unsigned char touch_read(unsigned char command)
{
	unsigned short result;
	
	//Set CS_TP pin low to begin SPI transmission
	
	FIO0CLR = CS_PIN;
	
	//Transmit command byte on MOSI, ignore MISO (full read write cycle)
	S0SPDR = command;
	
	while ((S0SPSR & (1 << 7)) == 0);
	
	result = S0SPDR;
	
	//Transmit 0x00 on MOSI, read in requested result on MISO (another full read write cycle)
	
	S0SPDR = 0x00;
	
	while ((S0SPSR & (1 << 7)) == 0);
	
	result = S0SPDR;
	
	//Transmission complete, set CS_TP pin back to high
	
	FIO0SET = CS_PIN;
	
	//Return 8 bit result.
	return (unsigned char) result;
}	

