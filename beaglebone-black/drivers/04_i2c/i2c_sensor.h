/*
 * BeagleBone Black - I2C Sensor Driver Header
 *
 * File:
 *     i2c_sensor.h
 */

#ifndef I2C_SENSOR_H
#define I2C_SENSOR_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define SENSOR_DRIVER_NAME          "bbb_i2c_sensor"

#define SENSOR_CLASS_NAME           "bbb_i2c_sensor_class"

#define SENSOR_DEVICE_NAME          "bbb_i2c_sensor"

#define SENSOR_I2C_DEVICE_NAME      "bbb_i2c_sensor"


/* ------------------------------------------------------------------------- */
/* Example I2C Sensor Address                                                */
/* ------------------------------------------------------------------------- */

#define SENSOR_I2C_ADDRESS          0x48


/* ------------------------------------------------------------------------- */
/* Sensor Registers                                                          */
/* ------------------------------------------------------------------------- */

/*
 * Example register map.
 *
 * These values are placeholders for a generic I2C sensor.
 * Replace them with the actual sensor datasheet values.
 */

#define SENSOR_REG_WHO_AM_I         0x00

#define SENSOR_REG_CONFIG           0x01

#define SENSOR_REG_DATA_HIGH        0x02

#define SENSOR_REG_DATA_LOW         0x03


/* ------------------------------------------------------------------------- */
/* Sensor Configuration                                                      */
/* ------------------------------------------------------------------------- */

#define SENSOR_CONFIG_DEFAULT       0x01


/* ------------------------------------------------------------------------- */
/* Sensor Data                                                               */
/* ------------------------------------------------------------------------- */

struct sensor_write_data {
    unsigned char reg;
    unsigned char value;
};


/* ------------------------------------------------------------------------- */
/* Driver Return Values                                                      */
/* ------------------------------------------------------------------------- */

#define SENSOR_SUCCESS              0

#define SENSOR_ERROR                -1


#endif /* I2C_SENSOR_H */
