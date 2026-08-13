/*
 * BeagleBone Black - GPIO Test Application
 *
 * File:
 *     gpio_test.c
 *
 * Purpose:
 *     User-space GPIO test application using the Linux libgpiod
 *     character-device interface.
 *
 * Usage:
 *     sudo ./gpio_test <gpiochip> <line> <mode>
 *
 * Modes:
 *     input
 *     output
 *     toggle
 *
 * Examples:
 *     sudo ./gpio_test /dev/gpiochip0 20 input
 *     sudo ./gpio_test /dev/gpiochip0 20 output
 *     sudo ./gpio_test /dev/gpiochip0 20 toggle
 *
 * Note:
 *     GPIO line numbers are GPIO-chip offsets, not physical header
 *     pin numbers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include <gpiod.h>

#include "gpio_test.h"

/* ------------------------------------------------------------------------- */
/* Global State                                                              */
/* ------------------------------------------------------------------------- */

static volatile sig_atomic_t stop_test = 0;


/* ------------------------------------------------------------------------- */
/* Signal Handler                                                            */
/* ------------------------------------------------------------------------- */

static void signal_handler(int signal_number)
{
    (void)signal_number;

    stop_test = 1;
}


/* ------------------------------------------------------------------------- */
/* Usage                                                                     */
/* ------------------------------------------------------------------------- */

static void print_usage(const char *program)
{
    printf("\n");
    printf("BeagleBone Black GPIO Test\n");
    printf("==========================\n\n");

    printf("Usage:\n");
    printf("  sudo %s <gpiochip> <line> <mode>\n\n",
           program);

    printf("Modes:\n");
    printf("  input       Read GPIO input continuously\n");
    printf("  output      Set GPIO output HIGH/LOW\n");
    printf("  toggle      Toggle GPIO output\n\n");

    printf("Examples:\n");
    printf("  sudo %s /dev/gpiochip0 20 input\n",
           program);

    printf("  sudo %s /dev/gpiochip0 20 output\n",
           program);

    printf("  sudo %s /dev/gpiochip0 20 toggle\n",
           program);

    printf("\n");
}


/* ------------------------------------------------------------------------- */
/* GPIO Chip Check                                                           */
/* ------------------------------------------------------------------------- */

static struct gpiod_chip *open_gpio_chip(const char *chip_path)
{
    struct gpiod_chip *chip;

    chip = gpiod_chip_open(chip_path);

    if (chip == NULL) {

        fprintf(stderr,
                "ERROR: Unable to open GPIO chip %s: %s\n",
                chip_path,
                strerror(errno));

        return NULL;
    }

    return chip;
}


/* ------------------------------------------------------------------------- */
/* GPIO Line Check                                                           */
/* ------------------------------------------------------------------------- */

static struct gpiod_line *get_gpio_line(struct gpiod_chip *chip,
                                        unsigned int line_number)
{
    struct gpiod_line *line;

    line = gpiod_chip_get_line(chip, line_number);

    if (line == NULL) {

        fprintf(stderr,
                "ERROR: GPIO line %u not available: %s\n",
                line_number,
                strerror(errno));

        return NULL;
    }

    return line;
}


/* ------------------------------------------------------------------------- */
/* GPIO Input Test                                                           */
/* ------------------------------------------------------------------------- */

static int test_input(struct gpiod_line *line,
                      unsigned int line_number)
{
    int value;

    printf("\n");
    printf("============================================================\n");
    printf(" GPIO INPUT TEST\n");
    printf("============================================================\n");

    printf("GPIO Line : %u\n", line_number);
    printf("Mode      : INPUT\n");
    printf("Press Ctrl+C to stop.\n");
    printf("------------------------------------------------------------\n");

    if (gpiod_line_request_input(line,
                                 GPIO_CONSUMER) < 0) {

        fprintf(stderr,
                "ERROR: Failed to request GPIO line %u as input: %s\n",
                line_number,
                strerror(errno));

        return -1;
    }

    while (!stop_test) {

        value = gpiod_line_get_value(line);

        if (value < 0) {

            fprintf(stderr,
                    "ERROR: Failed to read GPIO line %u: %s\n",
                    line_number,
                    strerror(errno));

            gpiod_line_release(line);

            return -1;
        }

        printf("\rGPIO %u = %s (%d)",
               line_number,
               value ? "HIGH" : "LOW",
               value);

        fflush(stdout);

        usleep(GPIO_SAMPLE_DELAY_US);
    }

    printf("\n");

    gpiod_line_release(line);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* GPIO Output Test                                                          */
/* ------------------------------------------------------------------------- */

static int test_output(struct gpiod_line *line,
                       unsigned int line_number)
{
    int value;

    printf("\n");
    printf("============================================================\n");
    printf(" GPIO OUTPUT TEST\n");
    printf("============================================================\n");

    printf("GPIO Line : %u\n", line_number);
    printf("Mode      : OUTPUT\n");
    printf("------------------------------------------------------------\n");

    if (gpiod_line_request_output(line,
                                  GPIO_CONSUMER,
                                  0) < 0) {

        fprintf(stderr,
                "ERROR: Failed to request GPIO line %u as output: %s\n",
                line_number,
                strerror(errno));

        return -1;
    }

    /*
     * Drive LOW.
     */
    value = 0;

    if (gpiod_line_set_value(line, value) < 0) {

        fprintf(stderr,
                "ERROR: Failed to set GPIO LOW: %s\n",
                strerror(errno));

        gpiod_line_release(line);

        return -1;
    }

    printf("GPIO %u -> LOW\n", line_number);

    sleep(1);

    /*
     * Drive HIGH.
     */
    value = 1;

    if (gpiod_line_set_value(line, value) < 0) {

        fprintf(stderr,
                "ERROR: Failed to set GPIO HIGH: %s\n",
                strerror(errno));

        gpiod_line_release(line);

        return -1;
    }

    printf("GPIO %u -> HIGH\n", line_number);

    sleep(1);

    /*
     * Return GPIO to LOW.
     */
    if (gpiod_line_set_value(line, 0) < 0) {

        fprintf(stderr,
                "ERROR: Failed to restore GPIO LOW: %s\n",
                strerror(errno));

        gpiod_line_release(line);

        return -1;
    }

    printf("GPIO %u -> LOW\n", line_number);

    gpiod_line_release(line);

    printf("------------------------------------------------------------\n");
    printf("GPIO output test: PASS\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* GPIO Toggle Test                                                          */
/* ------------------------------------------------------------------------- */

static int test_toggle(struct gpiod_line *line,
                       unsigned int line_number)
{
    int value = 0;
    int i;

    printf("\n");
    printf("============================================================\n");
    printf(" GPIO TOGGLE TEST\n");
    printf("============================================================\n");

    printf("GPIO Line : %u\n", line_number);
    printf("Mode      : TOGGLE\n");
    printf("Toggles   : %d\n",
           GPIO_TOGGLE_COUNT);

    printf("------------------------------------------------------------\n");

    if (gpiod_line_request_output(line,
                                  GPIO_CONSUMER,
                                  0) < 0) {

        fprintf(stderr,
                "ERROR: Failed to request GPIO line %u: %s\n",
                line_number,
                strerror(errno));

        return -1;
    }

    for (i = 0;
         i < GPIO_TOGGLE_COUNT && !stop_test;
         i++) {

        value = !value;

        if (gpiod_line_set_value(line,
                                 value) < 0) {

            fprintf(stderr,
                    "ERROR: Failed to set GPIO line %u: %s\n",
                    line_number,
                    strerror(errno));

            gpiod_line_release(line);

            return -1;
        }

        printf("Toggle %3d -> GPIO %u = %s\n",
               i + 1,
               line_number,
               value ? "HIGH" : "LOW");

        usleep(GPIO_TOGGLE_DELAY_US);
    }

    /*
     * Return GPIO to LOW.
     */
    gpiod_line_set_value(line, 0);

    gpiod_line_release(line);

    printf("------------------------------------------------------------\n");

    if (!stop_test) {
        printf("GPIO toggle test: PASS\n");
    } else {
        printf("GPIO toggle test: STOPPED\n");
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    const char *chip_path;
    const char *mode;

    unsigned int line_number;

    struct gpiod_chip *chip;
    struct gpiod_line *gpio_line;

    int result = EXIT_FAILURE;

    /*
     * Validate command-line arguments.
     */
    if (argc != 4) {

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }

    chip_path = argv[1];
    line_number = (unsigned int)strtoul(argv[2],
                                        NULL,
                                        10);
    mode = argv[3];

    /*
     * Register Ctrl+C handler.
     */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /*
     * Header.
     */
    printf("\n");
    printf("============================================================\n");
    printf(" BeagleBone Black - GPIO Test\n");
    printf("============================================================\n");

    printf("GPIO Chip : %s\n", chip_path);
    printf("GPIO Line : %u\n", line_number);
    printf("Mode      : %s\n", mode);

    /*
     * Open GPIO chip.
     */
    chip = open_gpio_chip(chip_path);

    if (chip == NULL) {
        return EXIT_FAILURE;
    }

    /*
     * Get GPIO line.
     */
    gpio_line = get_gpio_line(chip,
                              line_number);

    if (gpio_line == NULL) {

        gpiod_chip_close(chip);

        return EXIT_FAILURE;
    }

    /*
     * Execute requested test.
     */
    if (strcmp(mode, GPIO_MODE_INPUT) == 0) {

        result = test_input(gpio_line,
                             line_number);

    } else if (strcmp(mode, GPIO_MODE_OUTPUT) == 0) {

        result = test_output(gpio_line,
                              line_number);

    } else if (strcmp(mode, GPIO_MODE_TOGGLE) == 0) {

        result = test_toggle(gpio_line,
                             line_number);

    } else {

        fprintf(stderr,
                "ERROR: Unknown GPIO mode '%s'\n",
                mode);

        print_usage(argv[0]);
    }

    /*
     * Close GPIO chip.
     */
    gpiod_chip_close(chip);

    /*
     * Final status.
     */
    printf("\n");

    if (result == 0) {

        printf("============================================================\n");
        printf(" GPIO TEST: PASS\n");
        printf("============================================================\n");

        return EXIT_SUCCESS;
    }

    printf("============================================================\n");
    printf(" GPIO TEST: FAIL\n");
    printf("============================================================\n");

    return EXIT_FAILURE;
}
