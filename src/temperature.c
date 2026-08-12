#include "temperature.h"
#include "ADC.h"

int read_temp(void) {
    unsigned int raw = readTempADC();
    return -10 + (int)((raw * 60UL) / 1023UL); // map 10-bit ADC to a -10 to 50 range
}
