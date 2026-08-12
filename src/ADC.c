#include "lpc24xx.h"

// choose either ADC0.1 or ADC0.2
#define PINSEL_CLR ((3 << 16) | (3 << 18))
#define PINSEL_SET ((1 << 16) | (1 << 18))
#define PINMODE_CLR ((3 << 16) | (3 << 18))
#define PINMODE_SET ((2 << 16) | (2 << 18))
#define AD0CR_BASE (3 << 8)
#define AD0CR_PWR (1 << 21)
#define PCONP_BIT (1 << 12)
#define ADC_TIMEOUT 100000     // guard so a dead ADC doesn't hang the caller

void setupADC(void) {
    PCONP |= PCONP_BIT;     // Power on the ADC before touching its registers

    // Configure AD0.1 (P0.24) and AD0.2 (P0.25) both
    PINSEL1 &= ~PINSEL_CLR;
    PINSEL1 |= PINSEL_SET;

    PINMODE1 &= ~PINMODE_CLR;
    PINMODE1 |= PINMODE_SET;

    // Set the ADC to software controlled conversion mode, not burst mode
    AD0CR = AD0CR_BASE;
}

unsigned int readADCChannel(unsigned int channel) {
    unsigned long result;
    unsigned int guard = ADC_TIMEOUT;
    unsigned int sel;

    sel = 1u << channel;
    AD0CR = sel | AD0CR_BASE;
    // enables operational 
    AD0CR |= AD0CR_PWR;
    // start conversion
    AD0CR |= (1u << 24);

    if (channel == 1) {
        result = AD0DR1; 
    } else if (channel == 2) {
        result = AD0DR2; 
    }
    while (((result & (1UL << 31)) == 0) && --guard);
    return (unsigned int)((result >> 6) & 0x3FF);
}

// for streamlining
unsigned int readADC(void) {
    return readADCChannel(1);
}
