#include "lpc24xx.h"
#include "coffee_state.h"

void updateSmartplugState(HomeState *state){

    if(state->roomLights & COFFEE_STATUS){
        FIO2SET = COFFEE_STATUS;

    } else{
        FIO2CLR = COFFEE_STATUS;
    }

}
