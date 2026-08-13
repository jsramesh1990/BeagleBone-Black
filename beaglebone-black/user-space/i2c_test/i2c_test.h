/*
 * BeagleBone Black - I2C Test Header
 *
 * File:
 *     i2c_test.h
 *
 * Purpose:
 *     Common configuration definitions for the I2C user-space
 *     test application using the Linux I2C-dev interface.
 */

#ifndef I2C_TEST_H
#define I2C_TEST_H

/* ------------------------------------------------------------------------- */
/* I2C Configuration                                                         */
/* ------------------------------------------------------------------------- */

/* Default I2C device */
#define DEFAULT_I2C_DEVICE        "/dev/i2c-2"

/* Default I2C slave address */
#define DEFAULT_I2C_ADDRESS       0x50

/* Maximum valid 7-bit I2C address */
#define I2C_MAX_ADDRESS           0x7F

/* I2C scan range */
#define I2C_SCAN_START            0x03
#define I2C_SCAN_END              0x77

/* I2C test buffer size */
#define I2C_TEST_BUFFER_SIZE      8

/* Delay between I2C operations */
#define I2C_TEST_DELAY_US         1000


/* ------------------------------------------------------------------------- */
/* I2C Test Modes                                                            */
/* ------------------------------------------------------------------------- */

#define I2C_MODE_SCAN             "scan"

#define I2C_MODE_READ             "read"

#define I2C_MODE_WRITE            "write"

#define I2C_MODE_LOOPBACK         "loopback"


/* ------------------------------------------------------------------------- */
/* I2C Test Data                                                             */
/* ------------------------------------------------------------------------- */

#define I2C_TEST_DATA_START       0x10

#define I2C_LOOPBACK_DATA_START   0xA0


/* ------------------------------------------------------------------------- */
/* I2C Return Values                                                         */
/* ------------------------------------------------------------------------- */

#define I2C_TEST_SUCCESS          0

#define I2C_TEST_ERROR             -1


#endif /* I2C_TEST_H */
