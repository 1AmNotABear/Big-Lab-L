#include "ADC.h"
#include "lpc24xx.h"

int light(void) {
    // 1. Read existing ADC from ADC0.1
    unsigned int adc_raw = readADC(); 

    // 2. Convert raw data to voltage
    float voltage = ((float)adc_raw / 1023.0f) * 3.3f;

    // 3. Convert voltage to lux using 10k resistor
    float lux = (voltage * 1000.0f) * 2.0f;

    return (int)lux; 
}