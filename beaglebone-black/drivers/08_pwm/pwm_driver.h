/*
 * BeagleBone Black - PWM Driver Header
 *
 * File:
 *     pwm_driver.h
 */

#ifndef PWM_DRIVER_H
#define PWM_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define PWM_DRIVER_NAME            "bbb_pwm_driver"


/* ------------------------------------------------------------------------- */
/* PWM Configuration                                                        */
/* ------------------------------------------------------------------------- */

/*
 * Default PWM period:
 *
 * 20 ms = 50 Hz
 */
#define PWM_DEFAULT_PERIOD_NS      20000000ULL


/*
 * Default duty cycle:
 *
 * 50%
 */
#define PWM_DEFAULT_DUTY_PERCENT   50


/*
 * Minimum PWM period.
 */
#define PWM_MIN_PERIOD_NS          1000ULL


/*
 * Maximum PWM period.
 */
#define PWM_MAX_PERIOD_NS          1000000000ULL


/*
 * Maximum duty cycle.
 */
#define PWM_MAX_DUTY_NS            1000000000ULL


/* ------------------------------------------------------------------------- */
/* PWM Frequency Examples                                                    */
/* ------------------------------------------------------------------------- */

/*
 * 1 kHz PWM:
 *
 * 1 second / 1000 = 1 ms
 */
#define PWM_1KHZ_PERIOD_NS         1000000ULL


/*
 * 10 kHz PWM:
 */
#define PWM_10KHZ_PERIOD_NS        100000ULL


/*
 * 20 kHz PWM:
 */
#define PWM_20KHZ_PERIOD_NS        50000ULL


/* ------------------------------------------------------------------------- */
/* Driver Status                                                             */
/* ------------------------------------------------------------------------- */

#define PWM_SUCCESS                0

#define PWM_ERROR                  -1


#endif /* PWM_DRIVER_H */
