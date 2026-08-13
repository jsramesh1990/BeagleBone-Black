/*
 * BeagleBone Black - UART Driver Header
 *
 * File:
 *     uart_driver.h
 *
 * Purpose:
 *     Common definitions for the UART platform driver.
 */

#ifndef UART_DRIVER_H
#define UART_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define UART_DRIVER_NAME        "bbb_uart_driver"

#define UART_CLASS_NAME         "bbb_uart_class"

#define UART_DEVICE_NAME        "bbb_uart"


/* ------------------------------------------------------------------------- */
/* UART Register Offsets                                                     */
/* ------------------------------------------------------------------------- */

/*
 * Example UART register offsets.
 *
 * These offsets must match the UART hardware/IP block being used.
 */

#define UART_RBR_REG            0x00    /* Receive Buffer Register */

#define UART_THR_REG            0x00    /* Transmit Holding Register */

#define UART_LSR_REG            0x14    /* Line Status Register */

#define UART_LCR_REG            0x0C    /* Line Control Register */


/* ------------------------------------------------------------------------- */
/* Line Status Register Bits                                                 */
/* ------------------------------------------------------------------------- */

/*
 * LSR bit 0:
 * Receiver Data Ready
 */
#define UART_LSR_DATA_READY     (1U << 0)


/*
 * LSR bit 5:
 * Transmitter Holding Register Empty
 */
#define UART_LSR_THRE           (1U << 5)


/* ------------------------------------------------------------------------- */
/* Line Control Register                                                     */
/* ------------------------------------------------------------------------- */

/*
 * 8 data bits
 * 1 stop bit
 * No parity
 *
 * UART configuration:
 *     8-N-1
 */
#define UART_LCR_8N1             0x03


/* ------------------------------------------------------------------------- */
/* Timeout                                                                   */
/* ------------------------------------------------------------------------- */

#define UART_TIMEOUT             1000000


/* ------------------------------------------------------------------------- */
/* Driver Status                                                             */
/* ------------------------------------------------------------------------- */

#define UART_SUCCESS             0

#define UART_ERROR               -1


#endif /* UART_DRIVER_H */
