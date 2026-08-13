/*
 * BeagleBone Black - LED Driver Header
 *
 * File:
 *     led_driver.h
 */

#ifndef LED_DRIVER_H
#define LED_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define LED_DRIVER_NAME        "bbb_led_driver"


/* ------------------------------------------------------------------------- */
/* LED Configuration                                                         */
/* ------------------------------------------------------------------------- */

/*
 * Default LED state.
 */
#define LED_DEFAULT_OFF        0
#define LED_DEFAULT_ON         1


/* ------------------------------------------------------------------------- */
/* Driver Status                                                             */
/* ------------------------------------------------------------------------- */

#define LED_SUCCESS            0
#define LED_ERROR             -1


#endif /* LED_DRIVER_H */
