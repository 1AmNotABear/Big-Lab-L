#include "lpc24xx.h"
#include "homestate.h"
#include "lights.h"
#include "blinds.h"
#include "ADC.h"

void setDirections(void);
void enablePeripherals(void);

int main(void) {

    setDirections();
    enablePeripherals();

    homeState.roomLights |= LIGHT_BEDROOM2;
    updateLightState(&homeState);
	
		homeState.blind1 = BLIND_MID_WAY;    // blind 1 half up  -> LED1 green
    homeState.blind2 = BLIND_ROLLED_UP;  // blind 2 full up  -> LED2 red
    updateBlindState(&homeState);
    setupADC();

    while (1) {
        // idle here forever for now
        
        
        // update blind state based on adc, the numbers are 
        // placeholders based on 10 bit adc.
        unsigned int adcValue = readADC();
        if (adcValue > 682) {
            homeState.blind1 = BLIND_ROLLED_UP;
        } else if (adcValue > 341) {
            homeState.blind1 = BLIND_MID_WAY;
        } else {
            homeState.blind1 = BLIND_ROLLED_DOWN;
        }
        updateBlindState(&homeState);
    }
    return 0;
}

