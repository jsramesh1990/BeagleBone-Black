/*
 * BeagleBone Black - CAN Demo Header
 *
 * File:
 *     can_demo.h
 */

#ifndef CAN_DEMO_H
#define CAN_DEMO_H

#define CAN_DEMO_NAME              "bbb_can_demo"

/* CAN configuration */
#define CAN_DEFAULT_BITRATE        500000
#define CAN_DEFAULT_INTERFACE      "can0"

/* CAN frame configuration */
#define CAN_STANDARD_ID_MASK       0x7FF
#define CAN_MAX_DATA_LENGTH        8

/* Example CAN IDs */
#define CAN_TX_ID                  0x100
#define CAN_RX_ID                  0x200

/* CAN test payload */
#define CAN_TEST_DATA_LENGTH       8

#define CAN_SUCCESS               0
#define CAN_ERROR                -1

#endif /* CAN_DEMO_H */
