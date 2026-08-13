/*
 * BeagleBone Black - GPIO Test Header
 *
 * File:
 *     gpio_test.h
 *
 * Purpose:
 *     Common configuration definitions for the GPIO user-space
 *     test application using Linux libgpiod.
 */

#ifndef GPIO_TEST_H
#define GPIO_TEST_H

/* ------------------------------------------------------------------------- */
/* GPIO Configuration                                                        */
/* ------------------------------------------------------------------------- */

/* Default GPIO chip */
#define DEFAULT_GPIO_CHIP        "/dev/gpiochip0"

/* GPIO consumer name */
#define GPIO_CONSUMER            "bbb-gpio-test"

/* GPIO sampling interval */
#define GPIO_SAMPLE_DELAY_US     100000

/* GPIO toggle interval */
#define GPIO_TOGGLE_DELAY_US     500000

/* Number of GPIO toggles */
#define GPIO_TOGGLE_COUNT        10


/* ------------------------------------------------------------------------- */
/* GPIO Test Modes                                                           */
/* ------------------------------------------------------------------------- */

#define GPIO_MODE_INPUT          "input"

#define GPIO_MODE_OUTPUT         "output"

#define GPIO_MODE_TOGGLE         "toggle"


/* ------------------------------------------------------------------------- */
/* GPIO Values                                                               */
/* ------------------------------------------------------------------------- */

#define GPIO_LOW                 0

#define GPIO_HIGH                1


/* ------------------------------------------------------------------------- */
/* Return Values                                                             */
/* ------------------------------------------------------------------------- */

#define GPIO_TEST_SUCCESS        0

#define GPIO_TEST_ERROR          -1

#endif /* GPIO_TEST_H */
