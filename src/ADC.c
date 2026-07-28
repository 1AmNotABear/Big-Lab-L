# include "lpc24xx.h"
# define PINSEL_CLR (3 << 16)   // clears PINSEL AD0[1] (P0.24)
# define PINSEL_SET (1 << 16)   // sets PINSEL AD0[1] to 01
# define PINMODE_CLR (3 << 16)  // clears PINMODE for P0.24
# define PINMODE_SET (2 << 16)  // P0.24: neither pull-up nor pull-down (analog input)
# define AD0CR_CTRL ((1 << 1) | (3 << 8) | (1 << 21)) // select AD0.1, set clock, power on
# define PCONP_BIT (1 << 12)
# define ADC_TIMEOUT 100000     // guard so a dead ADC doesn't hang the caller

void setupADC(void) {
    PCONP |= PCONP_BIT;     // Power on the ADC before touching its registers

    // Configure the pin as AD0.1 with no pull-up (the default pull-up loads the sensor)
    PINSEL1 &= ~PINSEL_CLR;
    PINSEL1 |= PINSEL_SET;
    PINMODE1 &= ~PINMODE_CLR;
    PINMODE1 |= PINMODE_SET;

    // Set the ADC to software controlled conversion mode, not burst mode
    AD0CR = AD0CR_CTRL;
}

unsigned int readADC(void) {
    unsigned long result;
    unsigned int guard = ADC_TIMEOUT;

    // Full write (not |=) so DONE is cleared as the conversion is started
    AD0CR = AD0CR_CTRL | (1 << 24);   // start conversion on AD0.1 immediately

    // Read the register once per poll - reading it clears DONE
    do {
        result = AD0DR1;
    } while (((result & (1UL << 31)) == 0) && --guard);

    return (unsigned int)((result >> 6) & 0x3FF); // Return the converted value
}
