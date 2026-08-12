#ifndef ADC_H
#define ADC_H

#include "homestate.h"

void setupADC(void);
unsigned int readADC(void);

// AD0.2 / P0.25 - the "Analog Input" pot (R42) on the QVGA base board
void setupTempADC(void);
unsigned int readTempADC(void);

#endif
