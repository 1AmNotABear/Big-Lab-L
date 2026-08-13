# include "lpc24xx.h"

void setupADC(void) {
    PCONP |= (1 << 12);     // Power on the ADC before touching its registers

    // Configure the pin as AD0.1
    PINSEL1 &= ~(3 << 16);  // clears PINSEL AD0[1] (P0.24)
    PINSEL1 |= (1 << 16);   // sets PINSEL AD0[1] to 01
    PINMODE1 &= ~(3 << 16); // clears PINMODE for P0.24
    PINMODE1 |= (2 << 16);  // P0.24: neither pull-up nor pull-down (analog input)

    // Set the ADC to software controlled conversion mode, not burst mode
    AD0CR = (1 << 1) | (3 << 8) | (1 << 21); // select AD0.1, set clock, power on
}

unsigned int readADC(void) {
    unsigned long result;

    // Full write (not |=) so DONE is cleared as the conversion is started
    AD0CR = (1 << 1) | (3 << 8) | (1 << 21) | (1 << 24);   // select AD0.1, set clock, power on, start conversion immediately

    // Read the register once per poll - reading it clears DONE
    result = AD0DR1;
    while (((result & (1UL << 31)) == 0)) {
        result = AD0DR1;
    }

    return (unsigned int)((result >> 6) & 0x3FF); // Return the converted value
}

void setupTempADC(void) {
    PCONP |= (1 << 12);     // Power on the ADC before touching its registers

    // Configure the pin as AD0.2 (the "Analog Input" pot, R42)
    PINSEL1 &= ~(3 << 18);  // clears PINSEL AD0[2] (P0.25)
    PINSEL1 |= (1 << 18);   // sets PINSEL AD0[2] to 01
    PINMODE1 &= ~(3 << 18); // clears PINMODE for P0.25
    PINMODE1 |= (2 << 18);  // P0.25: neither pull-up nor pull-down (analog input)

    AD0CR = (1 << 2) | (3 << 8) | (1 << 21); // select AD0.2, set clock, power on
}

unsigned int readTempADC(void) {
    unsigned long result;

    AD0CR = (1 << 2) | (3 << 8) | (1 << 21) | (1 << 24);   // select AD0.2, set clock, power on, start conversion immediately

    result = AD0DR2;
    while (((result & (1UL << 31)) == 0)) {
        result = AD0DR2;
    }

    return (unsigned int)((result >> 6) & 0x3FF); // Return the converted value
}
