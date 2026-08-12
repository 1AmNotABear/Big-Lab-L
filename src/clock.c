#include "clock.h"
#include "lpc24xx.h"
#include "clocks.h"
#include "users.h"
#include "homestate.h"
#include "blinds.h"
#include "doorbell.h"

// 1 real second = 1 sim minute (60x speedup)
#define MS_PER_SIM_MINUTE 1000

int simHour = 0;
int simMinute = 0;

static unsigned long lastTick = 0;

void clock_init(void) {
    T0TCR = 0x02; // Reset and disable timer

    T0PR = (PCLK_TIMER0 / 1000) - 1; // PR = 36M / 1000, prescaler divides by (PR + 1) -> T0TC counts ms

    T0TCR = 0x1; // enable, running free (never reset again)

    lastTick = T0TC;
}

void clock_poll(void) {
    unsigned long now = T0TC;

    // Polled every loop tick regardless of which screen is active, so the
    // doorbell works no matter what's on screen.
    pushDoorbell(&homeState);

    while (now - lastTick >= MS_PER_SIM_MINUTE) {
        lastTick += MS_PER_SIM_MINUTE;

        simMinute++;
        if (simMinute >= 60) {
            simMinute = 0;
            simHour++;
            if (simHour >= 24) {
                simHour = 0;
            }
        }

        checkScheduleEvents();
        checkBlindAutomation(&homeState);
    }
}
