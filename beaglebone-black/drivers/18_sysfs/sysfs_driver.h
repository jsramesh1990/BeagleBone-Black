/*
 * BeagleBone Black - Sysfs Driver Header
 *
 * File:
 *     sysfs_driver.h
 */

#ifndef SYSFS_DRIVER_H
#define SYSFS_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define SYSFS_DRIVER_NAME \
	"bbb_sysfs_driver"

#define SYSFS_DRIVER_VERSION \
	"1.0"


/* ------------------------------------------------------------------------- */
/* Default Configuration                                                    */
/* ------------------------------------------------------------------------- */

#define SYSFS_DEFAULT_VALUE \
	0


/* ------------------------------------------------------------------------- */
/* Sysfs Attribute Names                                                     */
/* ------------------------------------------------------------------------- */

#define SYSFS_VALUE_ATTR \
	"value"

#define SYSFS_ENABLE_ATTR \
	"enable"

#define SYSFS_STATUS_ATTR \
	"status"


#endif /* SYSFS_DRIVER_H */
