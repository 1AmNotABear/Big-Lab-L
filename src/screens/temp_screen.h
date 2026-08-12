#ifndef TEMP_SCREEN_H
#define TEMP_SCREEN_H

typedef enum {
    TEMP_IN_PROGRESS,
    TEMP_SELECTED_HOME
} TempResult;

/* Shows the current temp (read_temp()) and lets the user adjust a set point
   within the admin's HI/LO limits (users[0], see src/users.h). The set point
   and the OFF toggle are stored in homeState so they persist across visits;
   neither one has any real effect on hardware. */
TempResult temp_screen_step(void);

#endif
