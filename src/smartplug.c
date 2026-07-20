#include "lpc24xx.h"
#include "coffee_state.h"

void updateSmartplugState(HomeState *state){

    if(state.roomLights.coffee_state){
        FIO2SET = COFFEE_STATE;

    } else{
        FIO2CLR = COFFEE_STATE;
    }

}
