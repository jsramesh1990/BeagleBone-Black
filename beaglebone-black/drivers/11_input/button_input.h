/*
 * BeagleBone Black - Button Input Driver Header
 *
 * File:
 *     button_input.h
 */

#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define BUTTON_DRIVER_NAME        "bbb_button_input"

#define BUTTON_INPUT_NAME         "BBB GPIO Button"


/* ------------------------------------------------------------------------- */
/* Button Configuration                                                      */
/* ------------------------------------------------------------------------- */

/*
 * GPIO debounce time.
 *
 * 20 ms is commonly used for mechanical push buttons.
 */
#define BUTTON_DEBOUNCE_US        20000


/* ------------------------------------------------------------------------- */
/* Button Input                                                              */
/* ------------------------------------------------------------------------- */

/*
 * Linux input event:
 *
 * BTN_0 = first generic button
 */
#define BUTTON_KEY_CODE           BTN_0


/* ------------------------------------------------------------------------- */
/* Driver Status                                                             */
/* ------------------------------------------------------------------------- */

#define BUTTON_SUCCESS            0

#define BUTTON_ERROR             -1


#endif /* BUTTON_INPUT_H */
