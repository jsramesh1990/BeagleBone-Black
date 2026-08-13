/*
 * BeagleBone Black - Debugfs Driver Header
 *
 * File:
 *     debugfs_driver.h
 */

#ifndef DEBUGFS_DRIVER_H
#define DEBUGFS_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define DEBUGFS_DRIVER_NAME \
	"bbb_debugfs_driver"

#define DEBUGFS_DRIVER_VERSION \
	"1.0"


/* ------------------------------------------------------------------------- */
/* Debugfs Directory                                                         */
/* ------------------------------------------------------------------------- */

#define DEBUGFS_DIR_NAME \
	"bbb_debugfs"

#define DEBUGFS_DIR_PATH \
	"/sys/kernel/debug/bbb_debugfs"


/* ------------------------------------------------------------------------- */
/* Debugfs Entries                                                           */
/* ------------------------------------------------------------------------- */

#define DEBUGFS_VALUE_NAME \
	"value"

#define DEBUGFS_COUNTER_NAME \
	"counter"

#define DEBUGFS_ENABLE_NAME \
	"enable"

#define DEBUGFS_MESSAGE_NAME \
	"message"

#define DEBUGFS_STATUS_NAME \
	"status"


/* ------------------------------------------------------------------------- */
/* Default Values                                                            */
/* ------------------------------------------------------------------------- */

#define DEBUGFS_DEFAULT_VALUE \
	0

#define DEBUGFS_DEFAULT_ENABLE \
	false

#define DEBUGFS_DEFAULT_MESSAGE \
	"BeagleBone Black"


/* ------------------------------------------------------------------------- */
/* Buffer Sizes                                                              */
/* ------------------------------------------------------------------------- */

#define DEBUGFS_INPUT_SIZE \
	64

#define DEBUGFS_OUTPUT_SIZE \
	128

#define DEBUGFS_MESSAGE_SIZE \
	128

#define DEBUGFS_STATUS_SIZE \
	512


#endif /* DEBUGFS_DRIVER_H */
