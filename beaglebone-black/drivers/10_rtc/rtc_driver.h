/*
 * BeagleBone Black - RTC Driver Header
 *
 * File:
 *     rtc_driver.h
 */

#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define RTC_DRIVER_NAME             "bbb_rtc_driver"


/* ------------------------------------------------------------------------- */
/* RTC Register Offsets                                                     */
/* ------------------------------------------------------------------------- */

/*
 * Example RTC register map.
 *
 * These offsets are placeholders and must match the actual
 * RTC hardware register map.
 */

#define RTC_SECONDS_REG             0x00

#define RTC_MINUTES_REG             0x04

#define RTC_HOURS_REG               0x08

#define RTC_DAY_REG                 0x0C

#define RTC_MONTH_REG               0x10

#define RTC_YEAR_REG                0x14


/* ------------------------------------------------------------------------- */
/* RTC Alarm Registers                                                       */
/* ------------------------------------------------------------------------- */

#define RTC_ALARM_SECONDS_REG       0x18

#define RTC_ALARM_MINUTES_REG       0x1C

#define RTC_ALARM_HOURS_REG         0x20


/* ------------------------------------------------------------------------- */
/* RTC Control Register                                                      */
/* ------------------------------------------------------------------------- */

#define RTC_CONTROL_REG             0x24


/*
 * Alarm enable bit.
 */
#define RTC_ALARM_ENABLE            (1U << 0)


/* ------------------------------------------------------------------------- */
/* RTC Timestamp Range                                                       */
/* ------------------------------------------------------------------------- */

#define RTC_TIMESTAMP_MIN           0

#define RTC_TIMESTAMP_MAX           4102444799LL


/* ------------------------------------------------------------------------- */
/* Driver Status                                                             */
/* ------------------------------------------------------------------------- */

#define RTC_SUCCESS                 0

#define RTC_ERROR                  -1


#endif /* RTC_DRIVER_H */
