#include "lpc24xx.h"
#include "homestate.h"
#include "lights.h"

void setDirections(void);
void enablePeripherals(void);

int main(void) {

    setDirections();
    enablePeripherals();

    homeState.roomLights |= LIGHT_BEDROOM2;
    updateLightState(&homeState);

    while (1) {
        // idle here forever for now
    }

    return 0;
}