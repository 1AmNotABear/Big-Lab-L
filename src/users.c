#include "users.h"
#include "clock.h"
#include <string.h>

UserSettings users[NUM_USERS] = {
    {
        "0000",         // userId
        "1234",         // password
        1,              // isAdmin
        1,              // active
        { { { 0, 0 }, 0, BLIND_ROLLED_UP } },  // blindSchedule (blank)
        0,              // blindActionCount
        { { 0, 0 }, 0 },  // coffeeSchedule (blank, disabled)
        30,             // tempHighLimit
        15              // tempLowLimit
    },
    {
        "0001",
        "4321",
        0,
        1,
        { { { 0, 0 }, 0, BLIND_ROLLED_UP } },
        0,
        { { 0, 0 }, 0 },
        0,
        0
    }
};

UserSettings *currentUser = NULL;

UserSettings *findUserByPassword(const char *password)
{
    int i;
    for (i = 0; i < NUM_USERS; i++) {
        if (users[i].active && strcmp(users[i].password, password) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

void checkScheduleEvents(void)
{
    if (currentUser == NULL) return;

    if (currentUser->coffeeSchedule.enabled &&
        currentUser->coffeeSchedule.time.hour == simHour &&
        currentUser->coffeeSchedule.time.minute == simMinute) {
        homeState.roomLights |= COFFEE_STATUS;
    }
}
