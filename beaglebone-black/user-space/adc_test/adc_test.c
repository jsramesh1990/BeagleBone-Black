/*
 * BeagleBone Black - ADC Test Application
 *
 * File:
 *     adc_test.c
 *
 * Purpose:
 *     Read ADC values from the BeagleBone Black ADC interface and
 *     display raw ADC value, voltage and converted percentage.
 *
 * Usage:
 *     sudo ./adc_test
 *     sudo ./adc_test <channel>
 *     sudo ./adc_test <channel> <samples>
 *
 * Examples:
 *     sudo ./adc_test
 *     sudo ./adc_test 0
 *     sudo ./adc_test 0 10
 *
 * Typical BBB ADC sysfs path:
 *     /sys/bus/iio/devices/iio:device0/in_voltageX_raw
 *
 * NOTE:
 *     The exact IIO device and ADC channel numbering should be verified
 *     on the target board.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "adc_test.h"

/* Default configuration */
#define DEFAULT_CHANNEL        0
#define DEFAULT_SAMPLES        10

/* BeagleBone Black ADC reference voltage */
#define ADC_REFERENCE_MV       1800.0

/* ADC resolution */
#define ADC_MAX_VALUE          4095.0

/* IIO ADC device */
#define IIO_DEVICE_PATH        "/sys/bus/iio/devices/iio:device0"

/* Maximum path length */
#define PATH_SIZE              256

/*
 * Build ADC raw-value path.
 */
static int build_adc_path(int channel, char *path, size_t path_size)
{
    int ret;

    if (path == NULL || path_size == 0) {
        return -1;
    }

    ret = snprintf(path,
                   path_size,
                   "%s/in_voltage%d_raw",
                   IIO_DEVICE_PATH,
                   channel);

    if (ret < 0 || (size_t)ret >= path_size) {
        return -1;
    }

    return 0;
}

/*
 * Read raw ADC value from sysfs.
 */
static int read_adc_raw(int channel, int *raw_value)
{
    char path[PATH_SIZE];
    FILE *fp;

    if (raw_value == NULL) {
        return -1;
    }

    if (build_adc_path(channel, path, sizeof(path)) != 0) {
        fprintf(stderr, "ERROR: Failed to build ADC path\n");
        return -1;
    }

    fp = fopen(path, "r");

    if (fp == NULL) {
        fprintf(stderr,
                "ERROR: Cannot open ADC channel %d\n"
                "Path: %s\n"
                "Reason: %s\n",
                channel,
                path,
                strerror(errno));

        return -1;
    }

    if (fscanf(fp, "%d", raw_value) != 1) {
        fprintf(stderr,
                "ERROR: Failed to read ADC value from %s\n",
                path);

        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}

/*
 * Convert raw ADC value to millivolts.
 */
static double adc_to_voltage_mv(int raw_value)
{
    return ((double)raw_value / ADC_MAX_VALUE) *
           ADC_REFERENCE_MV;
}

/*
 * Convert raw ADC value to percentage.
 */
static double adc_to_percentage(int raw_value)
{
    return ((double)raw_value / ADC_MAX_VALUE) * 100.0;
}

/*
 * Print ADC information.
 */
static void print_adc_result(int sample,
                            int raw_value,
                            double voltage_mv,
                            double percentage)
{
    printf("Sample %-4d | "
           "Raw: %-5d | "
           "Voltage: %7.2f mV | "
           "Level: %6.2f%%\n",
           sample,
           raw_value,
           voltage_mv,
           percentage);
}

/*
 * Print program usage.
 */
static void print_usage(const char *program)
{
    printf("\n");
    printf("BeagleBone Black ADC Test\n");
    printf("=========================\n");
    printf("\n");

    printf("Usage:\n");
    printf("  sudo %s [channel] [samples]\n", program);

    printf("\nExamples:\n");
    printf("  sudo %s\n", program);
    printf("  sudo %s 0\n", program);
    printf("  sudo %s 0 10\n", program);

    printf("\nArguments:\n");
    printf("  channel   ADC channel number (default: %d)\n",
           DEFAULT_CHANNEL);

    printf("  samples   Number of samples (default: %d)\n",
           DEFAULT_SAMPLES);

    printf("\n");
}

/*
 * Check whether the ADC IIO device exists.
 */
static int check_adc_device(void)
{
    if (access(IIO_DEVICE_PATH, F_OK) != 0) {

        fprintf(stderr,
                "ERROR: ADC IIO device not found.\n");

        fprintf(stderr,
                "Expected path:\n"
                "  %s\n",
                IIO_DEVICE_PATH);

        fprintf(stderr,
                "\nCheck the IIO devices using:\n"
                "  ls /sys/bus/iio/devices/\n");

        return -1;
    }

    return 0;
}

/*
 * Check ADC channel availability.
 */
static int check_adc_channel(int channel)
{
    char path[PATH_SIZE];

    if (build_adc_path(channel, path, sizeof(path)) != 0) {
        return -1;
    }

    if (access(path, R_OK) != 0) {

        fprintf(stderr,
                "ERROR: ADC channel %d is not available.\n",
                channel);

        fprintf(stderr,
                "Expected file:\n"
                "  %s\n",
                path);

        fprintf(stderr,
                "\nAvailable ADC channels can be checked with:\n"
                "  ls %s/in_voltage*_raw\n",
                IIO_DEVICE_PATH);

        return -1;
    }

    return 0;
}

/*
 * Main application.
 */
int main(int argc, char *argv[])
{
    int channel = DEFAULT_CHANNEL;
    int samples = DEFAULT_SAMPLES;

    int raw_value;
    int i;

    double voltage_mv;
    double percentage;

    double sum = 0.0;
    double average;

    /*
     * Parse channel argument.
     */
    if (argc >= 2) {

        channel = atoi(argv[1]);

        if (channel < 0) {

            fprintf(stderr,
                    "ERROR: Invalid ADC channel.\n");

            print_usage(argv[0]);

            return EXIT_FAILURE;
        }
    }

    /*
     * Parse sample count.
     */
    if (argc >= 3) {

        samples = atoi(argv[2]);

        if (samples <= 0) {

            fprintf(stderr,
                    "ERROR: Invalid sample count.\n");

            print_usage(argv[0]);

            return EXIT_FAILURE;
        }
    }

    /*
     * Too many arguments.
     */
    if (argc > 3) {

        fprintf(stderr,
                "ERROR: Too many arguments.\n");

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }

    /*
     * Display configuration.
     */
    printf("\n");
    printf("============================================================\n");
    printf(" BeagleBone Black - ADC Test\n");
    printf("============================================================\n");

    printf("ADC Device      : %s\n", IIO_DEVICE_PATH);
    printf("ADC Channel     : %d\n", channel);
    printf("Samples         : %d\n", samples);
    printf("Reference       : %.0f mV\n", ADC_REFERENCE_MV);
    printf("Resolution      : %.0f\n", ADC_MAX_VALUE + 1);
    printf("============================================================\n");
    printf("\n");

    /*
     * Check IIO ADC device.
     */
    if (check_adc_device() != 0) {
        return EXIT_FAILURE;
    }

    /*
     * Check requested ADC channel.
     */
    if (check_adc_channel(channel) != 0) {
        return EXIT_FAILURE;
    }

    /*
     * Read ADC samples.
     */
    printf("ADC Samples:\n");
    printf("------------------------------------------------------------\n");

    for (i = 0; i < samples; i++) {

        if (read_adc_raw(channel, &raw_value) != 0) {

            fprintf(stderr,
                    "ERROR: ADC read failed at sample %d\n",
                    i + 1);

            return EXIT_FAILURE;
        }

        /*
         * Validate ADC range.
         */
        if (raw_value < 0 ||
            raw_value > (int)ADC_MAX_VALUE) {

            fprintf(stderr,
                    "WARNING: ADC raw value %d is outside "
                    "expected range.\n",
                    raw_value);
        }

        voltage_mv = adc_to_voltage_mv(raw_value);

        percentage = adc_to_percentage(raw_value);

        print_adc_result(i + 1,
                         raw_value,
                         voltage_mv,
                         percentage);

        sum += raw_value;

        /*
         * Small delay between samples.
         */
        usleep(100000);
    }

    /*
     * Calculate average.
     */
    average = sum / samples;

    voltage_mv = adc_to_voltage_mv((int)average);

    percentage = adc_to_percentage((int)average);

    /*
     * Print summary.
     */
    printf("\n");
    printf("============================================================\n");
    printf(" ADC TEST SUMMARY\n");
    printf("============================================================\n");

    printf("Channel         : %d\n", channel);
    printf("Samples         : %d\n", samples);

    printf("Average Raw     : %.2f\n", average);

    printf("Average Voltage : %.2f mV\n",
           voltage_mv);

    printf("Average Level   : %.2f%%\n",
           percentage);

    printf("Status          : PASS\n");

    printf("============================================================\n");
    printf("\n");

    return EXIT_SUCCESS;
}
