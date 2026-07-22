#include "lpc24xx.h"
#include "doorbell.h"

#define DOORBELL_BTN (1 << 10) // P0.10 for push button corresponding to the doorbell

// Chime notes
#define DS5 	3215/2
#define E5 		3030/2
#define FS5 	2725/2


struct tone {
	int duration;
	int pitch;
	int volume;
};

struct tone chime_data[] = {
	{1, DS5, 0x300},	
	{1, DS5, 0x1A0},	
	{1, E5, 0x300},
	{1, E5, 0x1A0},
	{1, FS5, 0x300},
	{1, FS5, 0x1A0},
};

// Function Prototype
void ringDoorbell(unsigned int duration, int period, int vol);
void udelay(unsigned int delay_in_us);

// Detects when the doorbell is actually pushed
void pushDoorbell(HomeState *state) {

    unsigned int btn_pressed = (FIO0PIN & DOORBELL_BTN); // Whether button pushed or not
	int rate = 5200; // Sets speed that the sounds plays

    // Setup DAC
	PINSEL1 &= ~(3 << 20); // Clears bits 21:20
	PINSEL1 |= (2 << 20); // Sets bits 21:20 to 0b10 (AOUT)

    if (btn_pressed != 0) {
        for (int k = 0; k < 5; k++) {
			for (int i = 0; i < 6; i++) { // Repeats loop pf chimes 5 times
				ringDoorbell(rate * chime_data[i].duration, chime_data[i].pitch, chime_data[i].volume); // Rings for 3 seconds with period of 1 second
			}
        }
    }

}


// Plays the chime for the doorbell considering duration and period
void ringDoorbell(unsigned int duration, int period, int vol) {

    unsigned int elapsed_time = 0;

    // High for half period and low for half period (square wave)
	unsigned int half_period = (unsigned int)period / 2;

    while (elapsed_time < duration) {
		
		DACR = ((unsigned int)vol << 6); // High for half-period
		udelay(half_period);
		
		DACR = 0; // Low for half-period
		udelay(half_period);		
		
		elapsed_time += period; // after one complete period done, adds on to track time
		
	}

	DACR = 0; // output silent when done
}


// Delays processor for 'delay_in_us' number of microseconds
void udelay(unsigned int delay_in_us) {
	if (delay_in_us == 0) return;
	
	T0TCR = 0x02; // Reset and disable timer
	
	T0PR = (Fpclk / 1000000); // PR = 72M / microsec

	T0TCR = 0x1; // sets enable high and reset low
	
	while (T0TC < delay_in_us) {
		// hold
	}
	
	T0TCR = 0x00; // disable timer as is
}
