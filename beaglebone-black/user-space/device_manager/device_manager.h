/*
 * BeagleBone Black - Device Manager Header
 *
 * File:
 *     device_manager.h
 *
 * Purpose:
 *     Common definitions for the BeagleBone Black user-space
 *     device manager application.
 */

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

/* ------------------------------------------------------------------------- */
/* Device Manager Configuration                                              */
/* ------------------------------------------------------------------------- */

/* Maximum path length used by the device manager */
#define DEVICE_PATH_SIZE       256

/* Maximum device name length */
#define DEVICE_NAME_SIZE       128

/* ------------------------------------------------------------------------- */
/* Linux Device Paths                                                         */
/* ------------------------------------------------------------------------- */

#define DEV_PATH               "/dev"

#define GPIO_DEVICE_PATH       "/dev/gpiochip"

#define I2C_DEVICE_PATH        "/dev/i2c-"

#define SPI_DEVICE_PATH        "/dev/spidev"

#define UART_DEVICE_PATH       "/dev/tty"

#define CAN_DEVICE_PATH        "/sys/class/net"

#define PWM_DEVICE_PATH        "/sys/class/pwm"

#define ADC_DEVICE_PATH        "/sys/bus/iio/devices"

#define RTC_DEVICE_PATH        "/dev/rtc"

/* ------------------------------------------------------------------------- */
/* Device Types                                                              */
/* ------------------------------------------------------------------------- */

#define DEVICE_TYPE_GPIO       "GPIO"
#define DEVICE_TYPE_I2C        "I2C"
#define DEVICE_TYPE_SPI        "SPI"
#define DEVICE_TYPE_UART       "UART"
#define DEVICE_TYPE_CAN        "CAN"
#define DEVICE_TYPE_PWM        "PWM"
#define DEVICE_TYPE_ADC        "ADC"
#define DEVICE_TYPE_RTC        "RTC"

/* ------------------------------------------------------------------------- */
/* Device Manager Commands                                                    */
/* ------------------------------------------------------------------------- */

#define COMMAND_LIST           "list"
#define COMMAND_STATUS         "status"
#define COMMAND_SCAN           "scan"
#define COMMAND_HELP           "help"

/* ------------------------------------------------------------------------- */
/* Return Values                                                              */
/* ------------------------------------------------------------------------- */

#define DEVICE_MANAGER_SUCCESS 0
#define DEVICE_MANAGER_ERROR   -1

#endif /* DEVICE_MANAGER_H */
