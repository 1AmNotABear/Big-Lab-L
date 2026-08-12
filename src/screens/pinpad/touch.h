void touch_read_xy(char *x, char *y);
void touch_init(void);
unsigned char touch_read(unsigned char command);

typedef struct {
    int raw_touching;
    int stable_count;
    int confirmed_touching;
} TouchDebounceState;

/* Resets a screen's debounce state - call once when the screen activates. */
void touch_debounce_init(TouchDebounceState *state);

/* Samples the touch panel once (includes the inter-sample settle delay) and
   advances the debounce state machine. Returns 1 if this call observed a
   freshly confirmed touch-down, with *screen_x/*screen_y set to the press
   location in screen-pixel space (0..DISPLAY_WIDTH/HEIGHT). Returns 0
   otherwise (no change yet, still debouncing, or a release edge). */
int touch_poll_press(TouchDebounceState *state, int *screen_x, int *screen_y);
