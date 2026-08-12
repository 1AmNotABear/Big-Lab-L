#ifndef COFFEE_SCREEN_H
#define COFFEE_SCREEN_H

typedef enum {
    COFFEE_IN_PROGRESS,
    COFFEE_SELECTED_HOME,
    COFFEE_SELECTED_SCHEDULE
} CoffeeResult;

/* Lets the user toggle the coffee machine (word turns yellow when on) via
   the on-screen COFFEE word, or reach the schedule screen via COFFEE
   SCHEDULE. State lives in homeState.roomLights (src/homestate.h), same bit
   as the smart plug. The physical override button (P0.11) can also turn it
   off while this screen is active. */
CoffeeResult coffee_screen_step(void);

#endif
