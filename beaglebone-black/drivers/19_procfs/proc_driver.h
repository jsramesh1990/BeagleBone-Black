/*
 * BeagleBone Black - Procfs Driver Header
 *
 * File:
 *     proc_driver.h
 */

#ifndef PROC_DRIVER_H
#define PROC_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define PROC_DRIVER_NAME \
	"bbb_proc_driver"

#define PROC_DRIVER_VERSION \
	"1.0"


/* ------------------------------------------------------------------------- */
/* Proc Directory                                                            */
/* ------------------------------------------------------------------------- */

#define PROC_DIR_NAME \
	"bbb_proc"


/* ------------------------------------------------------------------------- */
/* Proc Entries                                                              */
/* ------------------------------------------------------------------------- */

#define PROC_VALUE_NAME \
	"value"

#define PROC_ENABLE_NAME \
	"enable"

#define PROC_STATUS_NAME \
	"status"

#define PROC_MESSAGE_NAME \
	"message"


/* ------------------------------------------------------------------------- */
/* Configuration                                                             */
/* ------------------------------------------------------------------------- */

#define PROC_DEFAULT_VALUE \
	0

#define PROC_DEFAULT_ENABLE \
	false

#define PROC_DEFAULT_MESSAGE \
	"BeagleBone Black"


/* ------------------------------------------------------------------------- */
/* Buffer Sizes                                                              */
/* ------------------------------------------------------------------------- */

#define PROC_OUTPUT_SIZE \
	128

#define PROC_INPUT_SIZE \
	64

#define PROC_MESSAGE_SIZE \
	128


#endif /* PROC_DRIVER_H */
