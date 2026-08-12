#include "temperature.h"
#include "ADC.h"

int read_temp(void) {
    unsigned int raw = readTempADC();
    return (int)((raw * 99UL) / 1023UL); // map 10-bit ADC to the 0-99 display range
}
