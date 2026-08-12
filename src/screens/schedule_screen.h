#ifndef SCHEDULE_SCREEN_H
#define SCHEDULE_SCREEN_H

typedef enum {
    SCHEDULE_IN_PROGRESS,
    SCHEDULE_SELECTED_HOME
} ScheduleResult;

/* Reached from COFFEE SCHEDULE. Lets the logged-in user (currentUser) set
   an hour/minute (24hr) at which the coffee machine turns on. Saved into
   currentUser->coffeeSchedule; checked once per simulated minute by
   checkScheduleEvents() (src/users.c). */
ScheduleResult schedule_screen_step(void);

#endif
