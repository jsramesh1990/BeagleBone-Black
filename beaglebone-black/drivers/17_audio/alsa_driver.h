/*
 * BeagleBone Black - ALSA Driver Header
 *
 * File:
 *     alsa_driver.h
 */

#ifndef ALSA_DRIVER_H
#define ALSA_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define ALSA_DRIVER_NAME \
	"bbb_alsa_driver"

#define ALSA_DRIVER_VERSION \
	"1.0"

#define ALSA_CARD_ID \
	"BBB_ALSA"

#define ALSA_CARD_NAME \
	"BeagleBone ALSA Demo"

#define ALSA_PCM_NAME \
	"BBB PCM"


/* ------------------------------------------------------------------------- */
/* PCM Configuration                                                         */
/* ------------------------------------------------------------------------- */

#define ALSA_PLAYBACK_STREAMS \
	1

#define ALSA_CAPTURE_STREAMS \
	1

#define ALSA_BUFFER_BYTES \
	(64 * 1024)

#define ALSA_PERIOD_BYTES_MIN \
	64

#define ALSA_PERIOD_BYTES_MAX \
	(16 * 1024)

#define ALSA_PERIODS_MAX \
	8


/* ------------------------------------------------------------------------- */
/* Sample Configuration                                                      */
/* ------------------------------------------------------------------------- */

#define ALSA_DEFAULT_RATE \
	48000

#define ALSA_DEFAULT_CHANNELS \
	2

#define ALSA_DEFAULT_SAMPLE_BITS \
	16


/* ------------------------------------------------------------------------- */
/* Status                                                                    */
/* ------------------------------------------------------------------------- */

#define ALSA_SUCCESS \
	0

#define ALSA_ERROR \
	-1


#endif /* ALSA_DRIVER_H */
