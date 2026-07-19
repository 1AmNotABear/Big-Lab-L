# include "lpc24xx.h"
# define PINSEL_CLR (3 << 18)
# define PINSEL_SET (1 << 17)
# define AD0CR_CTRL (1 << 2) | (3 << 8) | (1 << 21) | (1 << 24)
# define PCONP_BIT (1 << 12)

void setupADC(void) {
    // Configure ADC
    PINSEL1 &= ~PINSEL_CLR; // Clear bits
    PINSEL1 |= PINSEL_SET;  // Set bit 17 
    PCONP |= PCONP_BIT;     // Power on

    // Set the ADC to continuous conversion mode
    AD0CR |= (1 << 27);
}

unsigned int readADC(void) {
    AD0CR |= (1 << 0);      // choose A0.0 for conversion
    AD0CR |= (1 << 16);

    while ((AD0DR0) & (1 << 31) == 0);

    return (AD0DR0 >> 6) & 0xFFF; // Return the converted value
}

