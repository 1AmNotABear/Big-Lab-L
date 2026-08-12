#ifndef USERS_H
#define USERS_H

#include <stddef.h>

#include "homestate.h"

#define MAX_BLIND_ACTIONS 10
#define NUM_USERS 2

typedef struct {
    int hour;    // 0-23
    int minute;  // 0-59
} ScheduleTime;

typedef struct {
    ScheduleTime  time;
    int           blindNumber;  // 1 = blind1, 2 = blind2
    BlindPosition targetState;  // BLIND_ROLLED_UP / BLIND_MID_WAY / BLIND_ROLLED_DOWN
} BlindScheduleEntry;

typedef struct {
    ScheduleTime time;     // time the coffee machine turns on
    int          enabled;  // 0 = inactive, 1 = active
} CoffeeSchedule;

typedef struct {
    char userId[5];    // 4 digits + null terminator
    char password[5];  // 4 digits + null terminator
    int  isAdmin;       // 0 = regular user, 1 = admin
    int  active;         // 0 = deleted (hidden from profiles list, can't log in), 1 = normal

    BlindScheduleEntry blindSchedule[MAX_BLIND_ACTIONS];
    int                blindActionCount;  // how many of the 10 slots are used

    CoffeeSchedule coffeeSchedule;

    int tempHighLimit;
    int tempLowLimit;
} UserSettings;

/* Pre-populated users, defined in users.c: index 0 is the admin (id 0000,
   password 1234), index 1 is a regular user (id 0001, password 4321). */
extern UserSettings users[NUM_USERS];

/* Set by findUserByPassword() on a successful match - the user the pinpad
   screen last logged in. NULL until someone logs in. */
extern UserSettings *currentUser;

/* Returns a pointer to the user whose password matches, or NULL if none match
   (deleted/inactive users are skipped, so they can no longer log in). */
UserSettings *findUserByPassword(const char *password);

#endif
