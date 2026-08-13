/*
 * BeagleBone Black - Character Driver Header
 */

#ifndef BBB_CHAR_DRIVER_H
#define BBB_CHAR_DRIVER_H

#include <linux/ioctl.h>

/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define CHAR_DRIVER_NAME             "bbb_char"

#define CHAR_DRIVER_CLASS            "bbb_char_class"

#define CHAR_DRIVER_DEVICE           "bbb_char"

#define CHAR_DRIVER_VERSION          1


/* ------------------------------------------------------------------------- */
/* Driver Buffer                                                             */
/* ------------------------------------------------------------------------- */

#define CHAR_DRIVER_BUFFER_SIZE      256

#define CHAR_DRIVER_DEFAULT_MESSAGE  \
        "BeagleBone Black Character Driver\n"


/* ------------------------------------------------------------------------- */
/* IOCTL Commands                                                            */
/* ------------------------------------------------------------------------- */

#define CHAR_IOCTL_MAGIC             'B'

#define CHAR_IOCTL_CLEAR             \
        _IO(CHAR_IOCTL_MAGIC, 0x01)

#define CHAR_IOCTL_GET_VALUE         \
        _IOR(CHAR_IOCTL_MAGIC, 0x02, int)


#endif /* BBB_CHAR_DRIVER_H */
