#ifndef ADMIN_SCREEN_H
#define ADMIN_SCREEN_H

typedef enum {
    ADMIN_IN_PROGRESS,
    ADMIN_SELECTED_CONTROL,
    ADMIN_SELECTED_PROFILE,
    ADMIN_SELECTED_BACK
} AdminResult;

AdminResult admin_screen_step(void);

#endif
