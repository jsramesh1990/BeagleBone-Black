/*
 * BeagleBone Black - Interrupt Test Header
 *
 * File:
 *     interrupt_test.h
 *
 * Purpose:
 *     Common configuration definitions for the GPIO interrupt
 *     user-space test application using Linux libgpiod.
 */

#ifndef INTERRUPT_TEST_H
#define INTERRUPT_TEST_H

/* ------------------------------------------------------------------------- */
/* Interrupt Configuration                                                  */
/* ------------------------------------------------------------------------- */

/* GPIO consumer name */
#define INTERRUPT_CONSUMER          "bbb-interrupt-test"

/* Maximum GPIO line supported by this test application */
#define INTERRUPT_MAX_GPIO_LINE    255

/*
 * Number of interrupt events after which the test terminates.
 */
#define INTERRUPT_EVENT_COUNT      10

/*
 * Event wait timeout.
 *
 * gpiod_line_event_wait() expects a timespec.
 * This value represents 1 second.
 */
#define INTERRUPT_WAIT_TIMEOUT_NS  1000000000L


/* ------------------------------------------------------------------------- */
/* Interrupt Edge Modes                                                     */
/* ------------------------------------------------------------------------- */

#define INTERRUPT_EDGE_RISING      "rising"

#define INTERRUPT_EDGE_FALLING     "falling"

#define INTERRUPT_EDGE_BOTH        "both"


/* ------------------------------------------------------------------------- */
/* Interrupt Event Types                                                     */
/* ------------------------------------------------------------------------- */

#define INTERRUPT_EVENT_RISING     1

#define INTERRUPT_EVENT_FALLING    2


/* ------------------------------------------------------------------------- */
/* Return Values                                                             */
/* ------------------------------------------------------------------------- */

#define INTERRUPT_TEST_SUCCESS     0

#define INTERRUPT_TEST_ERROR       -1


#endif /* INTERRUPT_TEST_H */
