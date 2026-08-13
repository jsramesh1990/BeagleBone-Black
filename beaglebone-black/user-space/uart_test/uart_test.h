 /*
  * BeagleBone Black - UART Test Header
  *
  * File:
  *     uart_test.h
  *
  * Purpose:
  *     Common configuration definitions for the UART
  *     user-space test application.
  */

#ifndef UART_TEST_H
#define UART_TEST_H

/* ------------------------------------------------------------------------- */
/* UART Configuration                                                        */
/* ------------------------------------------------------------------------- */

/* Default UART device */
#define DEFAULT_UART_DEVICE          "/dev/ttyS1"

/* UART baud rate */
#define UART_BAUD_RATE              B115200

/* UART read timeout in deciseconds */
#define UART_READ_TIMEOUT_DS        10

/* UART receive/transmit buffer size */
#define UART_BUFFER_SIZE            256

/* Loopback receive timeout */
#define UART_LOOPBACK_TIMEOUT_SEC   2


/* ------------------------------------------------------------------------- */
/* UART Test Messages                                                        */
/* ------------------------------------------------------------------------- */

#define UART_TEST_MESSAGE           \
    "BeagleBone Black UART Test\r\n"

#define UART_LOOPBACK_MESSAGE       \
    "UART_LOOPBACK_TEST_123456\r\n"


/* ------------------------------------------------------------------------- */
/* UART Test Modes                                                           */
/* ------------------------------------------------------------------------- */

#define UART_MODE_CONFIG             "config"

#define UART_MODE_WRITE              "write"

#define UART_MODE_READ               "read"

#define UART_MODE_LOOPBACK           "loopback"


/* ------------------------------------------------------------------------- */
/* UART Return Values                                                        */
/* ------------------------------------------------------------------------- */

#define UART_TEST_SUCCESS            0

#define UART_TEST_ERROR              -1


#endif /* UART_TEST_H */
