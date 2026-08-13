Hello, welcome to our smart home automation hub!

(Note we recommend the use of a stylus when testing our system)
(Also note if you would like to learn a bit about our code structure first skip to code structure section)

Our hub offers control over the following appliances in your home:

Smart Blinds
Smart plug (Coffee Machine) 
Temperature (Air Conditioning Temp Settings)
All Lights 
Door bell

with the unique feature of owner and guest profiles named admin and user in our code.

When you boot up the system you will see a login screen!

This is a screen for existing users and admin to enter their password to access their saved settings.

Note, we have not implemented the ability to add new users in this prototype.

We have already configured the code with 2 existing profiles with the following log in details:

A User Account with password: 4321

An Admin account with password: 1234

We will walk you through our suggested testing flow, you are then free to test things as you wish after:

1. Login as a user. You can try some incorrect codes to see how it handles that too.

Once logged in you are greeted with the user's home screen (the user has id 0001):

You have 4 main options:

Light: Go into the lights screen to control home lights. The screen uses touchscreen input to change the lights and is also used to display what home lights are on - symbolised with the buttons filled with the colour yellow.

Blind: Go in to manually override blind positions. Yellow buttons mean the blind is in that position. If override is set the smart blind algorithm outlined later in this readme is disabled. Only when the user deselects override will the smart algorithm go back on.

Coffee: Go in to turn on or off the coffee machine and set the coffee schedule (details for that screen given a bit later in the readme). Coffee machine is on when the coffee button is yellow.

Temp: A screen to set the AC temp of the property (note this set temp does not impact the actual state in any way). It also shows the current temp which you can use the red potentiometer to change this value (note when you move the potentiometer the screen will not refresh this value, you will need to return to home using the home button then click the ‘TEMP’ button to return to the temp screen to see the refreshed value). The temp screen also displays the admin's max high and low setting, where a user cannot exceed these bounds.

All settings and home state like lights, blinds and coffee machine are retained when the user backs out of the screen. When they go back into the screen you will find their settings are retained as they were before.

Note on the coffee schedule:

The coffee schedule screen is used to set when the coffee machine smart plug turns on automatically via a timer being polled. 1 second in the real world equates to 1 hour in the simulation. The current time is displayed on the bottom of the coffee screen so you can use this to set an appropriate scheduled time. The scheduler is used to automatically turn the smart plug on, but does not offer the ability to schedule turn off times. Note when the scheduled time hits, if you are on the coffee screen it will not automatically refresh, you will need to go back out and then back in to see the coffee machine turned on (depicted as a yellow button for on, grey for off).

Try out different screens yourself to see how it works.

Before looking at the admin screen let's look at some of the physical peripherals.

Try pressing the doorbell button (P.0.10) at any time throughout your smart home automation use, the doorbell will sound, alerting users someone has pressed the doorbell
At any time if you press the P0.11 button the coffee machine will be manually toggled (either on or off) you can see this state being changed when on the coffee screen.

Blinds:

The output of the blind state is also shown on the two tricolour LEDS, 1 triColour LED for each blind in the home. 

Red LED means the blind is rolled up (i.e. open)
Green LED means the blind is midway (half drawn)
Blue LED means the blind is rolled down (i.e. closed)

As either the user or the smart blind algorithm changes the blind states, both the blinds screen and tri-colour LEDs will show this change.

Smart Blind Algorithm:

The smart blinds rely on a combination of both light and temperature levels of the room. The threshold of light and temperature are defined below from our measured experience in the lab.

For Light Sensor ADC values reported:
Dim light is less than 7
Bright is more than 40
Mid is anywhere between 7 and 40 inclusive

You can adjust these thresholds if they are not working as expected during your testing. But we found it worked consistently for us like this.

To simulate Dim light you should fully enclose your hand over the light sensor, to simulate bright light use a phone flashlight directed at the light sensor and mid should be the ambient light in the lab.

For Temperature ADC (Converted to a scale between -10 and 50):
Cold is less than 15
Hot is more than 24

Remember you can change the input temperature by moving the red potentiometer. You will need to look at the temp screen's current temp (refreshed by leaving the screen and coming back in) to see the value you are updating to.

Then to save energy in the home the following smart blinds algorithm is adopted:

A dim and cold room makes the blinds go down (closed/blue).
	Easiest way to test is just turn potentiometer way down (anticlockwise) and then cover the light sensor.
A bright and cold room means the blinds go up (open - red). 
	Easiest way to test is to have the user set some non open blind state, turn potentiometer way down. Deselect override,  (nothing happens) then shine flashlight over light sensor, blinds will go red as conditions now met.
A dim and hot room means the blinds go up (open - red). 
	Same as the bright and cold test but do for dim and hot.
A mid and hot room means the blinds go to the mid position (mid - green). 
	Easiest way to test is to test is turn potentiometer way up (clockwise).

Any remaining hot and cold position keeps blinds in their existing state.
Remember, you can check the temperature currently set in the temp screen to help make it more clear what state you are in.

These combinations from above will allow the home automation system to save energy and maintain a comfortable temperature of the room. 

Now for the admin screen. 

You can also log in to the admin screen (go back a lot to get back to the pin pad screen).
Login using the password. You have two options, the ability to set global low and high temps on all users for their AC settings and the ability to go into user profiles, where you can directly access user profiles to view and modify their settings. You can also delete profiles (but be careful if you do as deleting will remove it and there is no way to get the user back without repowering the board).


Code Structure:
main.c -> Initialisation then main screen loop: login -> home -> screens
homestate.c/.h -> State object of all appliances in the home
users.c/.h -> Where users are stored and their password and settings for appliances
clock.c/.h -> Timer0 sim-clock + poll handling for blinds and schedules
ADC.c/.h -> light sensor + temp pot setup & reads
temperature.c/.h -> converts raw temp ADC reading to degCblinds.c/.h
doorbell.c/.h -> P0.10 button read + DAC-driven chime (speaker)
peripherals.c -> GPIO direction setup for tricolour LEDs
coffee_screen.c/.h -> Coffee screen (Should be moved into screens folder but we are too scared to make any code changes now that we can't re-verify physically on board so it stays here now)
screens/ -> All screens, one file per screen (pinpad login, home, lights, temp, blind, admin, control, profiles, schedule)
lcd/ -> LCD, just taken from Lab 6.

