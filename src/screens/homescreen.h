#ifndef HOMESCREEN_H
#define HOMESCREEN_H

typedef enum {
    HOME_IN_PROGRESS,
    HOME_SELECTED_BACK,
    HOME_SELECTED_LIGHT,
    HOME_SELECTED_BLIND,
    HOME_SELECTED_COFFEE,
    HOME_SELECTED_TEMP
} HomeResult;

/* userId is the 4-digit id (plus null terminator) of the logged-in user,
   e.g. currentUser->userId - displayed as "HELLO ####!" */
HomeResult home_screen_step(const char *userId);

#endif
