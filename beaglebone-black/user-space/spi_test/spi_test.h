/*
 * BeagleBone Black - SPI Test Header
 *
 * File:
 *     spi_test.h
 *
 * Purpose:
 *     Common configuration definitions for the SPI
 *     user-space test application.
 */

#ifndef SPI_TEST_H
#define SPI_TEST_H

/* ------------------------------------------------------------------------- */
/* SPI Configuration                                                         */
/* ------------------------------------------------------------------------- */

/* Default SPI device */
#define DEFAULT_SPI_DEVICE          "/dev/spidev0.0"

/* SPI mode: CPOL = 0, CPHA = 0 */
#define SPI_DEFAULT_MODE            0

/* Bits per word */
#define SPI_DEFAULT_BITS            8

/* SPI clock speed */
#define SPI_DEFAULT_SPEED_HZ        1000000U

/* SPI test buffer size */
#define SPI_TEST_BUFFER_SIZE        8


/* ------------------------------------------------------------------------- */
/* SPI Test Data                                                             */
/* ------------------------------------------------------------------------- */

#define SPI_TEST_DATA_START         0x10

#define SPI_LOOPBACK_DATA_START     0xA0


/* ------------------------------------------------------------------------- */
/* SPI Test Modes                                                            */
/* ------------------------------------------------------------------------- */

#define SPI_MODE_READ               "read"

#define SPI_MODE_WRITE              "write"

#define SPI_MODE_TRANSFER           "transfer"

#define SPI_MODE_LOOPBACK           "loopback"


/* ------------------------------------------------------------------------- */
/* SPI Return Values                                                         */
/* ------------------------------------------------------------------------- */

#define SPI_TEST_SUCCESS            0

#define SPI_TEST_ERROR              -1


#endif /* SPI_TEST_H */
