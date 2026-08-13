/*
 * BeagleBone Black - ADC Test Header
 *
 * File:
 *     adc_test.h
 *
 * Purpose:
 *     Common definitions and function declarations for the
 *     ADC user-space test application.
 */

#ifndef ADC_TEST_H
#define ADC_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * ADC Configuration
 *---------------------------------------------------------------------------*/

/* Default ADC channel */
#define DEFAULT_CHANNEL        0

/* Default number of samples */
#define DEFAULT_SAMPLES        10

/* BBB ADC reference voltage in millivolts */
#define ADC_REFERENCE_MV       1800.0

/* 12-bit ADC maximum raw value */
#define ADC_MAX_VALUE          4095.0

/* IIO ADC device */
#define IIO_DEVICE_PATH        "/sys/bus/iio/devices/iio:device0"

/* Maximum ADC sysfs path length */
#define PATH_SIZE              256


/*---------------------------------------------------------------------------
 * ADC Function Prototypes
 *---------------------------------------------------------------------------*/

/*
 * Build the sysfs path for the requested ADC channel.
 *
 * Example:
 *   Channel 0:
 *   /sys/bus/iio/devices/iio:device0/in_voltage0_raw
 */
static int build_adc_path(int channel,
                          char *path,
                          size_t path_size);


/*
 * Read raw ADC value from the IIO sysfs interface.
 *
 * Returns:
 *   0  - success
 *  -1  - failure
 */
static int read_adc_raw(int channel,
                        int *raw_value);


/*
 * Convert ADC raw value to voltage in millivolts.
 */
static double adc_to_voltage_mv(int raw_value);


/*
 * Convert ADC raw value to percentage.
 */
static double adc_to_percentage(int raw_value);


/*
 * Print one ADC sample result.
 */
static void print_adc_result(int sample,
                             int raw_value,
                             double voltage_mv,
                             double percentage);


/*
 * Print application usage information.
 */
static void print_usage(const char *program);


/*
 * Check whether the IIO ADC device exists.
 *
 * Returns:
 *   0  - device available
 *  -1  - device unavailable
 */
static int check_adc_device(void);


/*
 * Check whether the requested ADC channel exists.
 *
 * Returns:
 *   0  - channel available
 *  -1  - channel unavailable
 */
static int check_adc_channel(int channel);

#ifdef __cplusplus
}
#endif

#endif /* ADC_TEST_H */
