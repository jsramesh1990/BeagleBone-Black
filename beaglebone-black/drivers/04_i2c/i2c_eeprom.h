/*
 * BeagleBone Black - I2C EEPROM Driver Header
 *
 * File:
 *     i2c_eeprom.h
 */

#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H

/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define EEPROM_DRIVER_NAME          "bbb_i2c_eeprom"

#define EEPROM_CLASS_NAME           "bbb_i2c_eeprom_class"

#define EEPROM_DEVICE_NAME         "bbb_i2c_eeprom"

#define EEPROM_I2C_DEVICE_NAME     "24c256"


/* ------------------------------------------------------------------------- */
/* EEPROM Configuration                                                      */
/* ------------------------------------------------------------------------- */

/*
 * Example EEPROM:
 *
 *     AT24C256
 *
 * 256 Kbit = 32 KB
 */
#define EEPROM_SIZE                 (32 * 1024)

/*
 * Maximum transfer used by the example driver.
 */
#define EEPROM_MAX_TRANSFER        32


/* ------------------------------------------------------------------------- */
/* EEPROM I2C Address                                                        */
/* ------------------------------------------------------------------------- */

#define EEPROM_I2C_ADDRESS         0x50


/* ------------------------------------------------------------------------- */
/* EEPROM Page Configuration                                                 */
/* ------------------------------------------------------------------------- */

#define EEPROM_PAGE_SIZE           64

#define EEPROM_WRITE_DELAY_MS      10


/* ------------------------------------------------------------------------- */
/* Driver Return Values                                                      */
/* ------------------------------------------------------------------------- */

#define EEPROM_SUCCESS             0

#define EEPROM_ERROR               -1


#endif /* I2C_EEPROM_H */
