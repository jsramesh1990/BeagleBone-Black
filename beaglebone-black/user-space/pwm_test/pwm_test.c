/*
 * BeagleBone Black - PWM Test Application
 *
 * File:
 *     pwm_test.c
 *
 * Purpose:
 *     User-space PWM test using the Linux sysfs PWM interface.
 *
 * Usage:
 *     sudo ./pwm_test <chip> <channel> <mode>
 *
 * Modes:
 *     enable
 *     disable
 *     set
 *     sweep
 *
 * Examples:
 *     sudo ./pwm_test 0 0 enable
 *     sudo ./pwm_test 0 0 disable
 *     sudo ./pwm_test 0 0 set
 *     sudo ./pwm_test 0 0 sweep
 *
 * PWM values are specified in nanoseconds.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "pwm_test.h"


/* ------------------------------------------------------------------------- */
/* Utility Functions                                                         */
/* ------------------------------------------------------------------------- */

static void print_usage(const char *program)
{
    printf("\n");
    printf("BeagleBone Black PWM Test\n");
    printf("=========================\n\n");

    printf("Usage:\n");
    printf("  sudo %s <chip> <channel> <mode>\n\n", program);

    printf("Modes:\n");
    printf("  enable       Enable PWM output\n");
    printf("  disable      Disable PWM output\n");
    printf("  set          Set period and duty cycle\n");
    printf("  sweep        Test multiple duty cycles\n\n");

    printf("Examples:\n");
    printf("  sudo %s 0 0 enable\n", program);
    printf("  sudo %s 0 0 disable\n", program);
    printf("  sudo %s 0 0 set\n", program);
    printf("  sudo %s 0 0 sweep\n", program);

    printf("\n");
}


/* ------------------------------------------------------------------------- */
/* Build PWM Path                                                            */
/* ------------------------------------------------------------------------- */

static int build_pwm_path(char *path,
                          size_t path_size,
                          unsigned int chip,
                          unsigned int channel,
                          const char *file)
{
    int ret;

    ret = snprintf(path,
                   path_size,
                   "%s/pwmchip%u/pwm%u/%s",
                   PWM_BASE_PATH,
                   chip,
                   channel,
                   file);

    if (ret < 0 ||
        (size_t)ret >= path_size) {

        fprintf(stderr,
                "ERROR: PWM path is too long.\n");

        return -1;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Write PWM Attribute                                                       */
/* ------------------------------------------------------------------------- */

static int write_pwm_attribute(unsigned int chip,
                               unsigned int channel,
                               const char *attribute,
                               unsigned long value)
{
    char path[PWM_PATH_SIZE];

    FILE *file;

    if (build_pwm_path(path,
                       sizeof(path),
                       chip,
                       channel,
                       attribute) != 0) {

        return -1;
    }

    file = fopen(path, "w");

    if (file == NULL) {

        fprintf(stderr,
                "ERROR: Unable to open %s: %s\n",
                path,
                strerror(errno));

        return -1;
    }

    if (fprintf(file,
                "%lu\n",
                value) < 0) {

        fprintf(stderr,
                "ERROR: Unable to write %s: %s\n",
                path,
                strerror(errno));

        fclose(file);

        return -1;
    }

    fclose(file);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Read PWM Attribute                                                        */
/* ------------------------------------------------------------------------- */

static int read_pwm_attribute(unsigned int chip,
                              unsigned int channel,
                              const char *attribute,
                              unsigned long *value)
{
    char path[PWM_PATH_SIZE];

    FILE *file;

    if (value == NULL) {
        return -1;
    }

    if (build_pwm_path(path,
                       sizeof(path),
                       chip,
                       channel,
                       attribute) != 0) {

        return -1;
    }

    file = fopen(path, "r");

    if (file == NULL) {

        fprintf(stderr,
                "ERROR: Unable to open %s: %s\n",
                path,
                strerror(errno));

        return -1;
    }

    if (fscanf(file,
               "%lu",
               value) != 1) {

        fprintf(stderr,
                "ERROR: Unable to read %s.\n",
                path);

        fclose(file);

        return -1;
    }

    fclose(file);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Check PWM Export                                                          */
/* ------------------------------------------------------------------------- */

static int is_pwm_exported(unsigned int chip,
                           unsigned int channel)
{
    char path[PWM_PATH_SIZE];

    int ret;

    ret = snprintf(path,
                   sizeof(path),
                   "%s/pwmchip%u/pwm%u",
                   PWM_BASE_PATH,
                   chip,
                   channel);

    if (ret < 0 ||
        (size_t)ret >= sizeof(path)) {

        return 0;
    }

    return access(path, F_OK) == 0;
}


/* ------------------------------------------------------------------------- */
/* Export PWM                                                                */
/* ------------------------------------------------------------------------- */

static int export_pwm(unsigned int chip,
                      unsigned int channel)
{
    char path[PWM_PATH_SIZE];

    FILE *file;

    int ret;

    /*
     * If the PWM channel already exists, it is already exported.
     */
    if (is_pwm_exported(chip, channel)) {
        return 0;
    }

    ret = snprintf(path,
                   sizeof(path),
                   "%s/pwmchip%u/export",
                   PWM_BASE_PATH,
                   chip);

    if (ret < 0 ||
        (size_t)ret >= sizeof(path)) {

        return -1;
    }

    file = fopen(path, "w");

    if (file == NULL) {

        fprintf(stderr,
                "ERROR: Unable to open PWM export: %s\n",
                strerror(errno));

        return -1;
    }

    if (fprintf(file,
                "%u\n",
                channel) < 0) {

        fprintf(stderr,
                "ERROR: Unable to export PWM channel %u: %s\n",
                channel,
                strerror(errno));

        fclose(file);

        return -1;
    }

    fclose(file);

    /*
     * Allow sysfs to create the PWM channel directory.
     */
    usleep(PWM_EXPORT_DELAY_US);

    if (!is_pwm_exported(chip, channel)) {

        fprintf(stderr,
                "ERROR: PWM channel was not exported.\n");

        return -1;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Unexport PWM                                                              */
/* ------------------------------------------------------------------------- */

static int unexport_pwm(unsigned int chip,
                        unsigned int channel)
{
    char path[PWM_PATH_SIZE];

    FILE *file;

    int ret;

    ret = snprintf(path,
                   sizeof(path),
                   "%s/pwmchip%u/unexport",
                   PWM_BASE_PATH,
                   chip);

    if (ret < 0 ||
        (size_t)ret >= sizeof(path)) {

        return -1;
    }

    file = fopen(path, "w");

    if (file == NULL) {

        return -1;
    }

    fprintf(file,
            "%u\n",
            channel);

    fclose(file);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Enable PWM                                                                */
/* ------------------------------------------------------------------------- */

static int enable_pwm(unsigned int chip,
                      unsigned int channel)
{
    printf("\n");
    printf("============================================================\n");
    printf(" PWM ENABLE TEST\n");
    printf("============================================================\n");

    if (export_pwm(chip, channel) != 0) {
        return -1;
    }

    if (write_pwm_attribute(chip,
                            channel,
                            PWM_ATTRIBUTE_ENABLE,
                            1) != 0) {

        return -1;
    }

    printf("PWM Chip    : %u\n", chip);
    printf("PWM Channel : %u\n", channel);
    printf("Status      : ENABLED\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Disable PWM                                                               */
/* ------------------------------------------------------------------------- */

static int disable_pwm(unsigned int chip,
                       unsigned int channel)
{
    printf("\n");
    printf("============================================================\n");
    printf(" PWM DISABLE TEST\n");
    printf("============================================================\n");

    if (!is_pwm_exported(chip, channel)) {

        printf("PWM channel is not exported.\n");

        return 0;
    }

    if (write_pwm_attribute(chip,
                            channel,
                            PWM_ATTRIBUTE_ENABLE,
                            0) != 0) {

        return -1;
    }

    printf("PWM Chip    : %u\n", chip);
    printf("PWM Channel : %u\n", channel);
    printf("Status      : DISABLED\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Set PWM                                                                   */
/* ------------------------------------------------------------------------- */

static int set_pwm(unsigned int chip,
                   unsigned int channel)
{
    unsigned long period;
    unsigned long duty_cycle;

    printf("\n");
    printf("============================================================\n");
    printf(" PWM CONFIGURATION TEST\n");
    printf("============================================================\n");

    if (export_pwm(chip, channel) != 0) {
        return -1;
    }

    period = PWM_DEFAULT_PERIOD_NS;
    duty_cycle = PWM_DEFAULT_DUTY_NS;

    /*
     * Disable before changing period/duty.
     */
    if (write_pwm_attribute(chip,
                            channel,
                            PWM_ATTRIBUTE_ENABLE,
                            0) != 0) {

        return -1;
    }

    /*
     * Configure period.
     */
    if (write_pwm_attribute(chip,
                            channel,
                            PWM_ATTRIBUTE_PERIOD,
                            period) != 0) {

        return -1;
    }

    /*
     * Duty cycle must not exceed period.
     */
    if (duty_cycle > period) {

        fprintf(stderr,
                "ERROR: Duty cycle cannot exceed period.\n");

        return -1;
    }

    if (write_pwm_attribute(chip,
                            channel,
                            PWM_ATTRIBUTE_DUTY,
                            duty_cycle) != 0) {

        return -1;
    }

    /*
     * Enable PWM.
     */
    if (write_pwm_attribute(chip,
                            channel,
                            PWM_ATTRIBUTE_ENABLE,
                            1) != 0) {

        return -1;
    }

    printf("PWM Chip    : %u\n", chip);
    printf("PWM Channel : %u\n", channel);
    printf("Period      : %lu ns\n", period);
    printf("Duty Cycle  : %lu ns\n", duty_cycle);
    printf("Status      : ENABLED\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* PWM Sweep                                                                 */
/* ------------------------------------------------------------------------- */

static int sweep_pwm(unsigned int chip,
                     unsigned int channel)
{
    unsigned long period;
    unsigned long duty_cycle;

    unsigned int duty_percent;

    printf("\n");
    printf("============================================================\n");
    printf(" PWM DUTY-CYCLE SWEEP TEST\n");
    printf("============================================================\n");

    if (export_pwm(chip, channel) != 0) {
        return -1;
    }

    period = PWM_DEFAULT_PERIOD_NS;

    /*
     * Disable before changing period.
     */
    if (write_pwm_attribute(chip,
                            channel,
                            PWM_ATTRIBUTE_ENABLE,
                            0) != 0) {

        return -1;
    }

    if (write_pwm_attribute(chip,
                            channel,
                            PWM_ATTRIBUTE_PERIOD,
                            period) != 0) {

        return -1;
    }

    /*
     * Enable PWM.
     */
    if (write_pwm_attribute(chip,
                            channel,
                            PWM_ATTRIBUTE_ENABLE,
                            1) != 0) {

        return -1;
    }

    /*
     * Sweep duty cycle from 0% to 100%.
     */
    for (duty_percent = PWM_SWEEP_START_PERCENT;
         duty_percent <= PWM_SWEEP_END_PERCENT;
         duty_percent += PWM_SWEEP_STEP_PERCENT) {

        duty_cycle =
            (period * duty_percent) / 100UL;

        if (write_pwm_attribute(chip,
                                channel,
                                PWM_ATTRIBUTE_DUTY,
                                duty_cycle) != 0) {

            write_pwm_attribute(chip,
                                channel,
                                PWM_ATTRIBUTE_ENABLE,
                                0);

            return -1;
        }

        printf("Duty Cycle: %3u%% -> %lu ns\n",
               duty_percent,
               duty_cycle);

        usleep(PWM_SWEEP_DELAY_US);
    }

    /*
     * Return to configured default duty cycle.
     */
    duty_cycle = PWM_DEFAULT_DUTY_NS;

    write_pwm_attribute(chip,
                        channel,
                        PWM_ATTRIBUTE_DUTY,
                        duty_cycle);

    printf("------------------------------------------------------------\n");
    printf("PWM sweep test: PASS\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    unsigned int chip;
    unsigned int channel;

    char *end;

    const char *mode;

    unsigned long parsed_value;

    int result = EXIT_FAILURE;

    /*
     * Validate arguments.
     */
    if (argc != 4) {

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }

    /*
     * Parse PWM chip number.
     */
    errno = 0;

    parsed_value = strtoul(argv[1],
                           &end,
                           10);

    if (errno != 0 ||
        *end != '\0' ||
        parsed_value > PWM_MAX_CHIP) {

        fprintf(stderr,
                "ERROR: Invalid PWM chip: %s\n",
                argv[1]);

        return EXIT_FAILURE;
    }

    chip = (unsigned int)parsed_value;

    /*
     * Parse PWM channel.
     */
    errno = 0;

    parsed_value = strtoul(argv[2],
                           &end,
                           10);

    if (errno != 0 ||
        *end != '\0' ||
        parsed_value > PWM_MAX_CHANNEL) {

        fprintf(stderr,
                "ERROR: Invalid PWM channel: %s\n",
                argv[2]);

        return EXIT_FAILURE;
    }

    channel = (unsigned int)parsed_value;

    mode = argv[3];

    printf("\n");
    printf("============================================================\n");
    printf(" BeagleBone Black - PWM Test\n");
    printf("============================================================\n");

    printf("PWM Chip    : %u\n", chip);
    printf("PWM Channel : %u\n", channel);
    printf("Mode        : %s\n", mode);

    /*
     * Execute requested operation.
     */
    if (strcmp(mode, PWM_MODE_ENABLE) == 0) {

        result = enable_pwm(chip,
                            channel);

    } else if (strcmp(mode, PWM_MODE_DISABLE) == 0) {

        result = disable_pwm(chip,
                             channel);

    } else if (strcmp(mode, PWM_MODE_SET) == 0) {

        result = set_pwm(chip,
                         channel);

    } else if (strcmp(mode, PWM_MODE_SWEEP) == 0) {

        result = sweep_pwm(chip,
                           channel);

    } else {

        fprintf(stderr,
                "ERROR: Unknown PWM mode: %s\n",
                mode);

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }

    printf("\n");

    if (result == 0) {

        printf("============================================================\n");
        printf(" PWM TEST: PASS\n");
        printf("============================================================\n");

        return EXIT_SUCCESS;
    }

    printf("============================================================\n");
    printf(" PWM TEST: FAIL\n");
    printf("============================================================\n");

    return EXIT_FAILURE;
}
