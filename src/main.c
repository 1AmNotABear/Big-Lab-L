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
#include "screens/schedule_screen.h"
#include "coffee_screen.h"
#include "users.h"
#include "ADC.h"
#include "clock.h"
#include "blinds.h"

void touch_init(void);
void setDirections(void);
void enablePeripherals(void);

// runs the home screen + LIGHT/BLIND/COFFEE/TEMP dispatch for currentUser,
// until BACK is pressed on the home screen. Used both for a regular user's
// own login and for an admin browsing a profile picked from USER PROFILE.
static void run_user_home(void)
{
    HomeResult homeResult;
    LightsResult lightsResult;
    TempResult tempResult;
    BlindResult blindResult;
    CoffeeResult coffeeResult;
    ScheduleResult scheduleResult;

    while (1) {
				// Starts with the user home screen options
        homeResult = home_screen_step(currentUser->userId);
        while (homeResult == HOME_IN_PROGRESS) {
            homeResult = home_screen_step(currentUser->userId);
        }
				// Select screen based on pressed option returned
        if (homeResult == HOME_SELECTED_LIGHT) {
            lightsResult = lights_screen_step();
            while (lightsResult == LIGHTS_IN_PROGRESS) {
                lightsResult = lights_screen_step();
            }
            // HOME was pressed, loop back and show the home screen again
        } else if (homeResult == HOME_SELECTED_TEMP) {
            tempResult = temp_screen_step();
            while (tempResult == TEMP_IN_PROGRESS) {
                tempResult = temp_screen_step();
            }
            // HOME was pressed, loop back and show the home screen again
        } else if (homeResult == HOME_SELECTED_BLIND) {
            blindResult = blind_screen_step();
            while (blindResult == BLIND_IN_PROGRESS) {
                blindResult = blind_screen_step();
            }
            // HOME was pressed, loop back and show the home screen again
        } else if (homeResult == HOME_SELECTED_COFFEE) {
            coffeeResult = coffee_screen_step();
            while (coffeeResult == COFFEE_IN_PROGRESS) {
                coffeeResult = coffee_screen_step();
            }

            if (coffeeResult == COFFEE_SELECTED_SCHEDULE) {
                scheduleResult = schedule_screen_step();
                while (scheduleResult == SCHEDULE_IN_PROGRESS) {
                    scheduleResult = schedule_screen_step();
                }
            }
            // HOME was pressed (on coffee or schedule screen), loop back and show the home screen again
        }

        if (!(homeResult == HOME_SELECTED_LIGHT || homeResult == HOME_SELECTED_TEMP ||
              homeResult == HOME_SELECTED_BLIND || homeResult == HOME_SELECTED_COFFEE)) {
            break;
        }
    }
}

// runs the admin screen (control panel vs. user profile) for the logged-in
// admin, until BACK is pressed there.
static void run_admin_home(void)
{
    AdminResult adminResult;
    ControlResult controlResult;
    ProfilesResult profilesResult;
    UserSettings *adminUser;
    int loggedOut;

    adminUser = currentUser;
    loggedOut = 0;

    while (1) {
				// Display default admin screen
        adminResult = admin_screen_step();
        while (adminResult == ADMIN_IN_PROGRESS) {
            adminResult = admin_screen_step();
        }

				// Branch on whether they go to the control screen or user profiles list screen
        if (adminResult == ADMIN_SELECTED_CONTROL) {
						// Control screen
            controlResult = control_screen_step();
            while (controlResult == CONTROL_IN_PROGRESS) {
                controlResult = control_screen_step();
            }
            // BACK was pressed, loop back and show the admin screen again
        } else if (adminResult == ADMIN_SELECTED_PROFILE) {
						// User profiles list screen
            profilesResult = profiles_screen_step();
            while (profilesResult == PROFILES_IN_PROGRESS) {
                profilesResult = profiles_screen_step();
            }

            if (profilesResult == PROFILES_SELECTED_USER) {
                // profiles_screen_step() already set currentUser to
                // the picked profile - browse their home screen. BACK
                // there restores the real admin and returns straight
                // to the admin screen (CONTROL / PROFILE choice).
                run_user_home();
                currentUser = adminUser;
            }
            // else HOME was pressed, loop back and show the admin screen again
        } else if (adminResult == ADMIN_SELECTED_BACK) {
            loggedOut = 1;
        }

        if (loggedOut) {
            break;
        }
    }
}

int main(void) {
    PinpadResult loginResult;

    // touch and lcd setups
    sdramInit();
    lcdInit(&lcd_config);
    lcdTurnOn();
    touch_init();

    // configure peripheral GPIO directions
    setDirections();
    enablePeripherals();

    // initialise the ADC channel used for read_temp()
    setupTempADC();

    // initialise the ADC channel used for the light sensor
    setupADC();

    // start the simulated clock (60x real time)
    clock_init();

    // infer the initial blind position from the current sensor readings
    checkBlindAutomation(&homeState);

    while (1) {
        // login gate: poll once per lap until it resolves. A denied attempt
        // shows the message briefly then automatically resets for another
        // try; only a correct password breaks out to the rest of the app.
        loginResult = pinpad_screen_step();
        if (loginResult != PINPAD_IN_PROGRESS) {
            mdelay(1000); // leave the ACCESS GRANTED/DENIED message up briefly
        }
        while (loginResult != PINPAD_ACCESS_GRANTED) {
            loginResult = pinpad_screen_step();
            if (loginResult != PINPAD_IN_PROGRESS) {
                mdelay(1000); // leave the ACCESS GRANTED/DENIED message up briefly
            }
        }

				// If admin we go to admin loop
        if (currentUser != NULL && currentUser->isAdmin) {
            // admin accounts are offered the admin screen (control panel vs.
            // user profile) right after login, until BACK is pressed there.
            run_admin_home();
            continue; // admin backed out, skip run_user_home and re-show the pin pad to log out
        }

        // regular users land on the homescreen. 
        run_user_home();
    }

    return 0;
}
