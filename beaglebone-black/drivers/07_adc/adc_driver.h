/*
 * BeagleBone Black - ADC Driver Header
 *
 * File:
 *     adc_driver.h
 */

#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define ADC_DRIVER_NAME             "bbb_adc_driver"

#define ADC_DEVICE_NAME             "bbb_adc"


/* ------------------------------------------------------------------------- */
/* ADC Configuration                                                        */
/* ------------------------------------------------------------------------- */

#define ADC_CHANNEL_COUNT           8

#define ADC_RESOLUTION              12

#define ADC_MAX_VALUE               ((1U << ADC_RESOLUTION) - 1)


/* ------------------------------------------------------------------------- */
/* ADC Register Offsets                                                     */
/* ------------------------------------------------------------------------- */

/*
 * Example register map.
 *
 * These offsets must be replaced with the actual ADC hardware
 * register offsets for the target SoC.
 */

#define ADC_CONTROL_REG             0x00

#define ADC_STATUS_REG              0x04

#define ADC_CHANNEL_SELECT_REG      0x08

#define ADC_DATA_REG                0x0C


/* ------------------------------------------------------------------------- */
/* ADC Control Register                                                     */
/* ------------------------------------------------------------------------- */

/*
 * Start ADC conversion.
 */
#define ADC_START_CONVERSION        (1U << 0)


/* ------------------------------------------------------------------------- */
/* ADC Status Register                                                      */
/* ------------------------------------------------------------------------- */

/*
 * Conversion completed.
 */
#define ADC_CONVERSION_DONE         (1U << 0)


/* ------------------------------------------------------------------------- */
/* ADC Data Register                                                        */
/* ------------------------------------------------------------------------- */

#define ADC_DATA_MASK               0x0FFF


/* ------------------------------------------------------------------------- */
/* ADC Timeout                                                              */
/* ------------------------------------------------------------------------- */

#define ADC_TIMEOUT                 1000000


/* ------------------------------------------------------------------------- */
/* Driver Status                                                            */
/* ------------------------------------------------------------------------- */

#define ADC_SUCCESS                 0

#define ADC_ERROR                   -1


#endif /* ADC_DRIVER_H */
