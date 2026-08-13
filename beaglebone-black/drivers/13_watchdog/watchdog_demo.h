/*
 * BeagleBone Black - Watchdog Demo Header
 *
 * File:
 *     watchdog_demo.h
 */

#ifndef WATCHDOG_DEMO_H
#define WATCHDOG_DEMO_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define WATCHDOG_DRIVER_NAME \
	"bbb_watchdog_demo"


/* ------------------------------------------------------------------------- */
/* Timeout Configuration                                                     */
/* ------------------------------------------------------------------------- */

#define WATCHDOG_MIN_TIMEOUT \
	1

#define WATCHDOG_MAX_TIMEOUT \
	120

#define WATCHDOG_DEFAULT_TIMEOUT \
	10


/* ------------------------------------------------------------------------- */
/* Driver Status                                                             */
/* ------------------------------------------------------------------------- */

#define WATCHDOG_SUCCESS \
	0

#define WATCHDOG_ERROR \
	-1


#endif /* WATCHDOG_DEMO_H */
