#ifndef PROFILES_SCREEN_H
#define PROFILES_SCREEN_H

typedef enum {
    PROFILES_IN_PROGRESS,
    PROFILES_SELECTED_HOME,
    PROFILES_SELECTED_USER
} ProfilesResult;

/* Lists active non-admin users (src/users.c) for the admin to manage: tapping
   a row signs in as that user (sets currentUser, same as pinpad_screen.c does
   on login) so the admin can browse their pages; tapping DEL sets that user's
   active flag to 0 so they're hidden here and can no longer log in. */
ProfilesResult profiles_screen_step(void);

#endif
