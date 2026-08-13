/*
 * BeagleBone Black - CAN Filter Header
 *
 * File:
 *     can_filter.h
 */

#ifndef CAN_FILTER_H
#define CAN_FILTER_H

#define CAN_FILTER_NAME             "bbb_can_filter"

/*
 * Example filter IDs.
 */
#define CAN_FILTER_ID               0x200

/*
 * Standard CAN identifier:
 *
 * 11-bit ID
 */
#define CAN_FILTER_MASK             0x7FF

/*
 * Extended CAN identifier mask.
 */
#define CAN_EXTENDED_MASK           0x1FFFFFFF

#define CAN_FILTER_SUCCESS          0
#define CAN_FILTER_ERROR           -1

#endif /* CAN_FILTER_H */
