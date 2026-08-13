/*
 * BeagleBone Black - SPI Display Driver Header
 *
 * File:
 *     spi_display.h
 */

#ifndef SPI_DISPLAY_H
#define SPI_DISPLAY_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define SPI_DISPLAY_DRIVER_NAME       "bbb_spi_display"

#define SPI_DISPLAY_CLASS_NAME        "bbb_spi_display_class"

#define SPI_DISPLAY_DEVICE_NAME       "bbb_spi_display"


/* ------------------------------------------------------------------------- */
/* SPI Configuration                                                         */
/* ------------------------------------------------------------------------- */

/*
 * SPI Mode 0:
 *
 * CPOL = 0
 * CPHA = 0
 */
#define SPI_DISPLAY_MODE              SPI_MODE_0

#define SPI_DISPLAY_BITS_PER_WORD    8

#define SPI_DISPLAY_MAX_TRANSFER     4096


/* ------------------------------------------------------------------------- */
/* Display Protocol                                                         */
/* ------------------------------------------------------------------------- */

/*
 * Example control bytes.
 *
 * Actual values depend on the display controller.
 */
#define SPI_DISPLAY_COMMAND           0x00

#define SPI_DISPLAY_DATA              0x01


/* ------------------------------------------------------------------------- */
/* Display Commands                                                         */
/* ------------------------------------------------------------------------- */

#define SPI_DISPLAY_CMD_RESET         0x01

#define SPI_DISPLAY_CMD_DISPLAY_ON    0x29

#define SPI_DISPLAY_CMD_DISPLAY_OFF   0x28


/* ------------------------------------------------------------------------- */
/* Driver Status                                                            */
/* ------------------------------------------------------------------------- */

#define SPI_DISPLAY_SUCCESS           0

#define SPI_DISPLAY_ERROR             -1


#endif /* SPI_DISPLAY_H */
