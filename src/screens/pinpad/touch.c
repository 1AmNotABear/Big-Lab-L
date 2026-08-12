#include "touch.h"
#include "lpc24xx.h"
#include "stdio.h"
#include "../../delay.h"
#include "../../lcd/lcd_grph.h"

#define CS_PIN            0x00100000        //P0.20
#define SCLK							(3 << 30)
#define MOSI							(3 << 4)
#define MISO							(3 << 2)
#define CS_TP							(3 << 8)

#define S0SPCR_val 				0x093C
#define S0SPCCR_val				0x24

#define ctrl_x						0xD8
#define ctrl_y						0x98

// touch feel tuning, shared by every screen that polls the panel via
// touch_poll_press()
#define TOUCH_THRESHOLD   2
#define DEBOUNCE_SAMPLES  4

unsigned char touch_read(unsigned char command);

void touch_init(void)
{
	//Implemented exactly from lab 6 
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
	//Implemented exactly from lab 6 
	//Read X co-ordinate from the touch screen controller
	*x = touch_read(0xD8);
	//Read Y co-ordinate from the touch screen controller
	*y = touch_read(0x98);
}

unsigned char touch_read(unsigned char command)
{
	unsigned short result;
	//Implemented exactly from lab 6 

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

void touch_debounce_init(TouchDebounceState *state)
{
	state->raw_touching = 0;
	state->stable_count = 0;
	state->confirmed_touching = 0;
}

int touch_poll_press(TouchDebounceState *state, int *screen_x, int *screen_y)
{
	unsigned char x = 0;
	unsigned char y = 0;
	unsigned char z1;
	unsigned char z2;
	int pressure;
	int touching;

	// small settle delay between samples - lets the resistive panel/SPI
	// reading stabilise
	mdelay(2);

	touch_read_xy((char *)&x, (char *)&y);
	z1 = touch_read(0xB8);
	z2 = touch_read(0xC8);

	// raw touch reads are 0-255; scale into screen-pixel space before
	// testing against button hitboxes
	*screen_x = ((int)x * DISPLAY_WIDTH) / 255;
	*screen_y = ((int)y * DISPLAY_HEIGHT) / 255;

	if (z1 == 0) {
		pressure = 0;
	} else {
		pressure = (int)(400.0f * ((float)x / 256.0f) *
			(((float)z2 / (float)z1) - 1.0f));
	}

	touching = (pressure > TOUCH_THRESHOLD) ? 1 : 0;

	// debounce: only accept a state change once it has been consistent
	// for DEBOUNCE_SAMPLES consecutive reads
	if (touching == state->raw_touching) {
		if (state->stable_count < DEBOUNCE_SAMPLES)
			state->stable_count++;
	} else {
		state->raw_touching = touching;
		state->stable_count = 1;
	}

	if (state->stable_count >= DEBOUNCE_SAMPLES &&
	    state->confirmed_touching != state->raw_touching) {
		state->confirmed_touching = state->raw_touching;
		return state->confirmed_touching ? 1 : 0;
	}

	return 0;
}

