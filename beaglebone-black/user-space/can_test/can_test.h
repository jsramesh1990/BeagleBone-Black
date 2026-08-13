/*
 * BeagleBone Black - CAN Test Header
 *
 * File:
 *     can_test.h
 *
 * Purpose:
 *     Common configuration definitions for the CAN user-space
 *     test application using Linux SocketCAN.
 */

#ifndef CAN_TEST_H
#define CAN_TEST_H

/*---------------------------------------------------------------------------
 * CAN Configuration
 *---------------------------------------------------------------------------*/

/* Default CAN interface */
#define DEFAULT_CAN_INTERFACE     "can0"

/* Standard CAN identifier */
#define CAN_TEST_ID               0x123

/* CAN frame data length */
#define CAN_TEST_DLC              8

/* Number of frames for TX/RX testing */
#define CAN_TEST_FRAME_COUNT      10

/* Delay between transmitted frames in microseconds */
#define CAN_TEST_DELAY_US         100000

/* Default CAN bitrate */
#define DEFAULT_CAN_BITRATE       500000


/*---------------------------------------------------------------------------
 * CAN Test Data
 *---------------------------------------------------------------------------*/

/* Test payload bytes */
#define CAN_TEST_DATA0            0x11
#define CAN_TEST_DATA1            0x22
#define CAN_TEST_DATA2            0x33
#define CAN_TEST_DATA3            0x44
#define CAN_TEST_DATA4            0x55
#define CAN_TEST_DATA5            0x66
#define CAN_TEST_DATA6            0x77
#define CAN_TEST_DATA7            0x88


/*---------------------------------------------------------------------------
 * CAN Test Modes
 *---------------------------------------------------------------------------*/

#define CAN_MODE_TX               "tx"
#define CAN_MODE_RX               "rx"
#define CAN_MODE_LOOPBACK         "loopback"

#endif /* CAN_TEST_H */
