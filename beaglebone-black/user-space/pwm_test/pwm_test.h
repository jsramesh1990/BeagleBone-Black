/*
 * BeagleBone Black - PWM Test Header
 *
 * File:
 *     pwm_test.h
 *
 * Purpose:
 *     Common configuration definitions for the PWM user-space
 *     test application using the Linux PWM sysfs interface.
 */

#ifndef PWM_TEST_H
#define PWM_TEST_H

/* ------------------------------------------------------------------------- */
/* PWM Configuration                                                         */
/* ------------------------------------------------------------------------- */

/* Linux PWM sysfs base path */
#define PWM_BASE_PATH              "/sys/class/pwm"

/* Maximum PWM chip number supported by this application */
#define PWM_MAX_CHIP               32

/* Maximum PWM channel number supported by this application */
#define PWM_MAX_CHANNEL            32

/* Maximum generated PWM path length */
#define PWM_PATH_SIZE              256

/* Delay after exporting PWM channel */
#define PWM_EXPORT_DELAY_US        100000


/* ------------------------------------------------------------------------- */
/* PWM Attributes                                                            */
/* ------------------------------------------------------------------------- */

#define PWM_ATTRIBUTE_PERIOD       "period"

#define PWM_ATTRIBUTE_DUTY         "duty_cycle"

#define PWM_ATTRIBUTE_ENABLE       "enable"


/* ------------------------------------------------------------------------- */
/* PWM Modes                                                                 */
/* ------------------------------------------------------------------------- */

#define PWM_MODE_ENABLE            "enable"

#define PWM_MODE_DISABLE           "disable"

#define PWM_MODE_SET               "set"

#define PWM_MODE_SWEEP             "sweep"


/* ------------------------------------------------------------------------- */
/* Default PWM Configuration                                                 */
/* ------------------------------------------------------------------------- */

/*
 * Default period:
 *     20 ms = 50 Hz
 *
 * Useful as a basic PWM test frequency.
 */
#define PWM_DEFAULT_PERIOD_NS      20000000UL

/*
 * Default duty cycle:
 *     1 ms = 5% of a 20 ms period
 */
#define PWM_DEFAULT_DUTY_NS        1000000UL


/* ------------------------------------------------------------------------- */
/* PWM Sweep Configuration                                                   */
/* ------------------------------------------------------------------------- */

#define PWM_SWEEP_START_PERCENT    0

#define PWM_SWEEP_END_PERCENT      100

#define PWM_SWEEP_STEP_PERCENT     10

#define PWM_SWEEP_DELAY_US         200000


/* ------------------------------------------------------------------------- */
/* PWM Return Values                                                         */
/* ------------------------------------------------------------------------- */

#define PWM_TEST_SUCCESS           0

#define PWM_TEST_ERROR             -1


#endif /* PWM_TEST_H */
