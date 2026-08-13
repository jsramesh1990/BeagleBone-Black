/*
 * BeagleBone Black - RTC Test Header
 *
 * File:
 *     rtc_test.h
 *
 * Purpose:
 *     Common configuration definitions for the RTC
 *     user-space test application.
 */

#ifndef RTC_TEST_H
#define RTC_TEST_H

/* ------------------------------------------------------------------------- */
/* RTC Configuration                                                         */
/* ------------------------------------------------------------------------- */

/* Default Linux RTC device */
#define DEFAULT_RTC_DEVICE          "/dev/rtc0"

/* Number of periodic RTC interrupts to test */
#define RTC_PERIODIC_EVENT_COUNT    10


/* ------------------------------------------------------------------------- */
/* RTC Test Modes                                                            */
/* ------------------------------------------------------------------------- */

#define RTC_MODE_READ               "read"

#define RTC_MODE_SET                "set"

#define RTC_MODE_ALARM              "alarm"

#define RTC_MODE_PERIODIC           "periodic"


/* ------------------------------------------------------------------------- */
/* RTC Return Values                                                         */
/* ------------------------------------------------------------------------- */

#define RTC_TEST_SUCCESS            0

#define RTC_TEST_ERROR              -1


#endif /* RTC_TEST_H */
