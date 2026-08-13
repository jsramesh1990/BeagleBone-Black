/*
 * BeagleBone Black - SPI Sensor Driver Header
 *
 * File:
 *     spi_sensor.h
 */

#ifndef SPI_SENSOR_H
#define SPI_SENSOR_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define SPI_SENSOR_DRIVER_NAME        "bbb_spi_sensor"

#define SPI_SENSOR_CLASS_NAME         "bbb_spi_sensor_class"

#define SPI_SENSOR_DEVICE_NAME        "bbb_spi_sensor"


/* ------------------------------------------------------------------------- */
/* SPI Configuration                                                         */
/* ------------------------------------------------------------------------- */

#define SPI_SENSOR_MODE               SPI_MODE_0

#define SPI_SENSOR_BITS_PER_WORD     8

#define SPI_SENSOR_MAX_TRANSFER      256


/* ------------------------------------------------------------------------- */
/* SPI Sensor Register Map                                                  */
/* ------------------------------------------------------------------------- */

/*
 * Example register addresses.
 *
 * Replace these values with the actual sensor datasheet values.
 */

#define SPI_SENSOR_WHO_AM_I           0x00

#define SPI_SENSOR_CONFIG             0x01

#define SPI_SENSOR_DATA_HIGH          0x02

#define SPI_SENSOR_DATA_LOW           0x03


/* ------------------------------------------------------------------------- */
/* SPI Read/Write Protocol                                                  */
/* ------------------------------------------------------------------------- */

/*
 * Example protocol:
 *
 * Bit 7 = 1 -> Read
 * Bit 7 = 0 -> Write
 */
#define SPI_SENSOR_READ_BIT           0x80


/* ------------------------------------------------------------------------- */
/* User-Space Write Structure                                               */
/* ------------------------------------------------------------------------- */

struct spi_sensor_write_data {
    unsigned char reg;
    unsigned char value;
};


/* ------------------------------------------------------------------------- */
/* Driver Status                                                            */
/* ------------------------------------------------------------------------- */

#define SPI_SENSOR_SUCCESS            0

#define SPI_SENSOR_ERROR              -1


#endif /* SPI_SENSOR_H */
