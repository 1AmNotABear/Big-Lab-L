# include "lpc24xx.h"
# define PINSEL_CLR (3 << 18)   // clears PINSEL AD0[2]
# define PINSEL_SET (1 << 18)   // sets PINSEL AD0[2] to 01
# define AD0CR_CTRL (1 << 2) | (3 << 8) | (1 << 21) // select AD0.2, set clock as  enable
# define PCONP_BIT (1 << 12)

void setupADC(void) {
    // Configure ADC
    PINSEL1 &= ~PINSEL_CLR; // Clear bits
    PINSEL1 |= PINSEL_SET;  // Set bit 17 
    PCONP |= PCONP_BIT;     // Power on

    // Set the ADC to continuous conversion mode, and not burst mode
    AD0CR = AD0CR_CTRL;
}

unsigned int readADC(void) {
    AD0CR |= (1 << 2);      // choose A0.2 for conversion

    AD0CR |= (1 << 24);    // start conversion immediately
    while ((AD0DR2 & (1 << 31)) == 0);
    return (AD0DR2 >> 6) & 0x3FF; // Return the converted value
    
}
