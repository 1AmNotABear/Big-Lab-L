#include "lpc24xx.h"
#include "homestate.h"
#include "lights.h"
#include "blinds.h"
#include "TEMT6000.h"
#include "ADC.h"

void setDirections(void);
void enablePeripherals(void);

int main(void) {

    unsigned int adcValue;
    int light_lux;

    setDirections();
    enablePeripherals();

    homeState.roomLights |= LIGHT_BEDROOM2;
    updateLightState(&homeState);
	
		homeState.blind1 = BLIND_MID_WAY;    // blind 1 half up  -> LED1 green
    homeState.blind2 = BLIND_ROLLED_UP;  // blind 2 full up  -> LED2 red
    updateBlindState(&homeState);
    setupADC();

    unsigned int temp_raw = 0;
    float temperature_c = 0;

    while (1) {
        // update blind state based on adc, the numbers are
        // placeholders based on 10 bit adc.
        adcValue = readADC();
        if (adcValue > 310) {
            homeState.blind1 = BLIND_ROLLED_UP;
        } else if (adcValue > 150) {
            homeState.blind1 = BLIND_MID_WAY;
        } else {
            homeState.blind1 = BLIND_ROLLED_DOWN;
        }
        updateBlindState(&homeState);

        // Read the light sensor on AD0.1
        light_lux = light();

        // Read the red Potentiometer for temperature modelling;
        temp_raw = readADCChannel(2);
        temperature_c = ((float)temp_raw / 1023.0f) * 100.0f - 20.0f;
    }
    return 0;
}

