/*
 * BeagleBone Black - Ethernet Driver Header
 *
 * File:
 *     ethernet_demo.h
 */

#ifndef ETHERNET_DEMO_H
#define ETHERNET_DEMO_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define ETHERNET_DRIVER_NAME \
	"bbb_ethernet_demo"

#define ETHERNET_DRIVER_VERSION \
	"1.0"


/* ------------------------------------------------------------------------- */
/* Ethernet Configuration                                                    */
/* ------------------------------------------------------------------------- */

#define ETHERNET_MTU \
	1500

#define ETHERNET_TX_TIMEOUT_MS \
	5000


/* ------------------------------------------------------------------------- */
/* MAC Address                                                               */
/* ------------------------------------------------------------------------- */

#define ETHERNET_MAC_LENGTH \
	6


/* ------------------------------------------------------------------------- */
/* Status                                                                    */
/* ------------------------------------------------------------------------- */

#define ETHERNET_SUCCESS \
	0

#define ETHERNET_ERROR \
	-1


#endif /* ETHERNET_DEMO_H */
