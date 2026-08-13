/*
 * BeagleBone Black - USB Driver Header
 *
 * File:
 *     usb_driver.h
 */

#ifndef USB_DRIVER_H
#define USB_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define USB_DRIVER_NAME \
	"bbb_usb_driver"


/* ------------------------------------------------------------------------- */
/* Test USB Device IDs                                                       */
/* ------------------------------------------------------------------------- */

/*
 * These are example IDs.
 *
 * Replace them with the actual VID/PID of the USB device.
 */

#define USB_VENDOR_ID_TEST \
	0x1234

#define USB_PRODUCT_ID_TEST \
	0x5678


/* ------------------------------------------------------------------------- */
/* USB Buffer Configuration                                                  */
/* ------------------------------------------------------------------------- */

#define USB_BUFFER_SIZE \
	512


/* ------------------------------------------------------------------------- */
/* Status                                                                     */
/* ------------------------------------------------------------------------- */

#define USB_SUCCESS \
	0

#define USB_ERROR \
	-1


#endif /* USB_DRIVER_H */
