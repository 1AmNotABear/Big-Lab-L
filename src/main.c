#include "delay.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include "screens/pinpad_screen.h"
#include "screens/admin_screen.h"
#include "screens/homescreen.h"
#include "screens/control_screen.h"
#include "screens/lights_screen.h"
#include "screens/temp_screen.h"
#include "screens/blind_screen.h"
#include "screens/profiles_screen.h"
#include "users.h"
#include "ADC.h"

void touch_init(void);

// runs the home screen + LIGHT/BLIND/COFFEE/TEMP dispatch for currentUser,
// until BACK is pressed on the home screen. Used both for a regular user's
// own login and for an admin browsing a profile picked from USER PROFILE.
static void run_user_home(void)
{
    HomeResult homeResult;
    LightsResult lightsResult;
    TempResult tempResult;
    BlindResult blindResult;

    do {
        do {
            homeResult = home_screen_step(currentUser->userId);
        } while (homeResult == HOME_IN_PROGRESS);

        if (homeResult == HOME_SELECTED_LIGHT) {
            do {
                lightsResult = lights_screen_step();
            } while (lightsResult == LIGHTS_IN_PROGRESS);
            // HOME was pressed, loop back and show the home screen again
        } else if (homeResult == HOME_SELECTED_TEMP) {
            do {
                tempResult = temp_screen_step();
            } while (tempResult == TEMP_IN_PROGRESS);
            // HOME was pressed, loop back and show the home screen again
        } else if (homeResult == HOME_SELECTED_BLIND) {
            do {
                blindResult = blind_screen_step();
            } while (blindResult == BLIND_IN_PROGRESS);
            // HOME was pressed, loop back and show the home screen again
        } else if (homeResult != HOME_SELECTED_BACK) {
            // dedicated COFFEE screen doesn't exist yet - just
            // acknowledge the choice for now and go back to the pinpad.
            lcd_fontColor(WHITE, NAVY);
            switch (homeResult) {
                case HOME_SELECTED_COFFEE:
                    lcd_putString(68, 105, (unsigned char *)"COFFEE SELECTED");
                    break;
                default:
                    break;
            }
            mdelay(1000);
        }
    } while (homeResult == HOME_SELECTED_LIGHT || homeResult == HOME_SELECTED_TEMP ||
             homeResult == HOME_SELECTED_BLIND);
}

int main(void) {
    PinpadResult loginResult;
    AdminResult adminResult;
    ControlResult controlResult;
    ProfilesResult profilesResult;
    int impersonating;

    // setup the external memory used by the LCD
    sdramInit();

    // setup the LCD using the supplied configuration
    lcdInit(&lcd_config);

    // turn the LCD screen on
    lcdTurnOn();

    // initialise the touch controller
    touch_init();

    // initialise the ADC channel used for read_temp()
    setupTempADC();

    while (1) {
        // login gate: poll once per lap until it resolves. This is where
        // other non-blocking work (doorbell, sensors, ...) would later be
        // polled too, since pinpad_screen_step() no longer blocks. A denied
        // attempt shows the message briefly then automatically resets for
        // another try; only a correct password breaks out to the rest of
        // the app.
        do {
            loginResult = pinpad_screen_step();
            if (loginResult != PINPAD_IN_PROGRESS) {
                mdelay(1000); // leave the ACCESS GRANTED/DENIED message up briefly
            }
        } while (loginResult != PINPAD_ACCESS_GRANTED);

        if (currentUser != NULL && currentUser->isAdmin) {
            // admin accounts are offered the admin screen (control panel vs.
            // user profile) right after login. Polled the same non-blocking
            // way as the login gate above.
            impersonating = 0;

            do {
                do {
                    adminResult = admin_screen_step();
                } while (adminResult == ADMIN_IN_PROGRESS);

                if (adminResult == ADMIN_SELECTED_CONTROL) {
                    do {
                        controlResult = control_screen_step();
                    } while (controlResult == CONTROL_IN_PROGRESS);
                    // HOME was pressed, loop back and show the admin screen again
                } else if (adminResult == ADMIN_SELECTED_PROFILE) {
                    do {
                        profilesResult = profiles_screen_step();
                    } while (profilesResult == PROFILES_IN_PROGRESS);

                    if (profilesResult == PROFILES_SELECTED_USER) {
                        // profiles_screen_step() already set currentUser to
                        // the picked profile - break out and browse as them
                        impersonating = 1;
                    }
                    // else HOME was pressed, loop back and show the admin screen again
                }
            } while (!impersonating &&
                     (adminResult == ADMIN_SELECTED_CONTROL || adminResult == ADMIN_SELECTED_PROFILE));

            if (impersonating) {
                // BACK on this home screen falls through to the pinpad gate
                // above, same as any regular user - the admin re-enters a
                // PIN to get back into the admin panel.
                run_user_home();
            }

            continue;
        }

        // regular users land on the homescreen. Polled the same
        // non-blocking way as the login gate above.
        run_user_home();
    }

    return 0;
}
