/*
 * BeagleBone Black - Interrupt Test Application
 *
 * File:
 *     interrupt_test.c
 *
 * Purpose:
 *     User-space GPIO interrupt test using the Linux GPIO
 *     character-device interface and libgpiod.
 *
 * Usage:
 *     sudo ./interrupt_test <gpiochip> <line> <edge>
 *
 * Edge modes:
 *     rising
 *     falling
 *     both
 *
 * Examples:
 *     sudo ./interrupt_test /dev/gpiochip0 20 rising
 *     sudo ./interrupt_test /dev/gpiochip0 20 falling
 *     sudo ./interrupt_test /dev/gpiochip0 20 both
 *
 * The application waits for GPIO edge events and reports the
 * timestamp and event type.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <gpiod.h>

#include "interrupt_test.h"


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
    printf("BeagleBone Black GPIO Interrupt Test\n");
    printf("====================================\n\n");

    printf("Usage:\n");
    printf("  sudo %s <gpiochip> <line> <edge>\n\n",
           program);

    printf("Edge modes:\n");
    printf("  rising      Rising-edge interrupt\n");
    printf("  falling     Falling-edge interrupt\n");
    printf("  both        Rising + falling edge\n\n");

    printf("Examples:\n");
    printf("  sudo %s /dev/gpiochip0 20 rising\n",
           program);

    printf("  sudo %s /dev/gpiochip0 20 falling\n",
           program);

    printf("  sudo %s /dev/gpiochip0 20 both\n",
           program);

    printf("\nPress Ctrl+C to stop.\n\n");
}


/* ------------------------------------------------------------------------- */
/* GPIO Interrupt Request                                                    */
/* ------------------------------------------------------------------------- */

static int request_interrupt(struct gpiod_line *line,
                             const char *edge)
{
    if (strcmp(edge, INTERRUPT_EDGE_RISING) == 0) {

        if (gpiod_line_request_rising_edge_events(
                line,
                INTERRUPT_CONSUMER) < 0) {

            fprintf(stderr,
                    "ERROR: Failed to request rising-edge events: %s\n",
                    strerror(errno));

            return -1;
        }

    } else if (strcmp(edge, INTERRUPT_EDGE_FALLING) == 0) {

        if (gpiod_line_request_falling_edge_events(
                line,
                INTERRUPT_CONSUMER) < 0) {

            fprintf(stderr,
                    "ERROR: Failed to request falling-edge events: %s\n",
                    strerror(errno));

            return -1;
        }

    } else if (strcmp(edge, INTERRUPT_EDGE_BOTH) == 0) {

        if (gpiod_line_request_both_edges_events(
                line,
                INTERRUPT_CONSUMER) < 0) {

            fprintf(stderr,
                    "ERROR: Failed to request both-edge events: %s\n",
                    strerror(errno));

            return -1;
        }

    } else {

        fprintf(stderr,
                "ERROR: Invalid interrupt edge: %s\n",
                edge);

        return -1;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Print Interrupt Event                                                     */
/* ------------------------------------------------------------------------- */

static void print_event(const struct gpiod_line_event *event,
                        unsigned long event_count)
{
    const char *event_type;

    if (event->event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
        event_type = "RISING";
    } else if (event->event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
        event_type = "FALLING";
    } else {
        event_type = "UNKNOWN";
    }

    printf("IRQ %5lu | %-8s | timestamp = %lld.%09lld\n",
           event_count,
           event_type,
           (long long)event->ts.tv_sec,
           (long long)event->ts.tv_nsec);

    fflush(stdout);
}


/* ------------------------------------------------------------------------- */
/* Interrupt Test                                                            */
/* ------------------------------------------------------------------------- */

static int run_interrupt_test(struct gpiod_line *line,
                              unsigned int line_number,
                              const char *edge)
{
    struct gpiod_line_event event;

    unsigned long event_count = 0;

    int result;

    printf("\n");
    printf("============================================================\n");
    printf(" GPIO INTERRUPT TEST\n");
    printf("============================================================\n");

    printf("GPIO Line : %u\n", line_number);
    printf("Edge      : %s\n", edge);
    printf("Consumer  : %s\n", INTERRUPT_CONSUMER);

    printf("------------------------------------------------------------\n");
    printf("Waiting for GPIO interrupt events...\n");
    printf("Press Ctrl+C to stop.\n\n");

    /*
     * Configure GPIO line for interrupt events.
     */
    if (request_interrupt(line, edge) != 0) {
        return -1;
    }

    /*
     * Wait for GPIO events.
     */
    while (!stop_test) {

        /*
         * Wait for an event with timeout.
         *
         * The timeout allows the loop to periodically check
         * stop_test instead of blocking forever.
         */
        result = gpiod_line_event_wait(
            line,
            INTERRUPT_WAIT_TIMEOUT_NS);

        if (result < 0) {

            fprintf(stderr,
                    "ERROR: GPIO event wait failed: %s\n",
                    strerror(errno));

            gpiod_line_release(line);

            return -1;
        }

        if (result == 0) {
            /*
             * Timeout - no interrupt received.
             */
            continue;
        }

        /*
         * Read the GPIO event.
         */
        if (gpiod_line_event_read(line,
                                  &event) < 0) {

            fprintf(stderr,
                    "ERROR: Failed to read GPIO event: %s\n",
                    strerror(errno));

            gpiod_line_release(line);

            return -1;
        }

        event_count++;

        print_event(&event,
                    event_count);

        /*
         * Stop automatically after the configured
         * number of interrupts.
         */
        if (event_count >= INTERRUPT_EVENT_COUNT) {
            break;
        }
    }

    /*
     * Release GPIO line.
     */
    gpiod_line_release(line);

    printf("\n");
    printf("------------------------------------------------------------\n");

    if (stop_test) {
        printf("Interrupt test stopped by user.\n");
    } else {
        printf("Interrupt events received: %lu\n",
               event_count);
        printf("GPIO interrupt test: PASS\n");
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    const char *chip_path;
    const char *edge;

    unsigned int line_number;

    struct gpiod_chip *chip;
    struct gpiod_line *line;

    char *end;
    unsigned long parsed_line;

    int result;

    /*
     * Validate command-line arguments.
     */
    if (argc != 4) {

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }

    chip_path = argv[1];
    edge = argv[3];

    /*
     * Parse GPIO line number.
     */
    errno = 0;

    parsed_line = strtoul(argv[2],
                          &end,
                          10);

    if (errno != 0 ||
        *end != '\0' ||
        parsed_line > INTERRUPT_MAX_GPIO_LINE) {

        fprintf(stderr,
                "ERROR: Invalid GPIO line: %s\n",
                argv[2]);

        return EXIT_FAILURE;
    }

    line_number = (unsigned int)parsed_line;

    /*
     * Register signal handlers.
     */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /*
     * Validate edge mode before opening hardware.
     */
    if (strcmp(edge, INTERRUPT_EDGE_RISING) != 0 &&
        strcmp(edge, INTERRUPT_EDGE_FALLING) != 0 &&
        strcmp(edge, INTERRUPT_EDGE_BOTH) != 0) {

        fprintf(stderr,
                "ERROR: Invalid edge mode: %s\n",
                edge);

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }

    /*
     * Header.
     */
    printf("\n");
    printf("============================================================\n");
    printf(" BeagleBone Black - Interrupt Test\n");
    printf("============================================================\n");

    printf("GPIO Chip : %s\n", chip_path);
    printf("GPIO Line : %u\n", line_number);
    printf("Edge      : %s\n", edge);

    /*
     * Open GPIO chip.
     */
    chip = gpiod_chip_open(chip_path);

    if (chip == NULL) {

        fprintf(stderr,
                "ERROR: Unable to open GPIO chip %s: %s\n",
                chip_path,
                strerror(errno));

        return EXIT_FAILURE;
    }

    /*
     * Get GPIO line.
     */
    line = gpiod_chip_get_line(chip,
                               line_number);

    if (line == NULL) {

        fprintf(stderr,
                "ERROR: Unable to get GPIO line %u: %s\n",
                line_number,
                strerror(errno));

        gpiod_chip_close(chip);

        return EXIT_FAILURE;
    }

    /*
     * Run interrupt test.
     */
    result = run_interrupt_test(line,
                                line_number,
                                edge);

    /*
     * Close GPIO chip.
     */
    gpiod_chip_close(chip);

    /*
     * Final result.
     */
    printf("\n");

    if (result == 0) {

        printf("============================================================\n");
        printf(" INTERRUPT TEST: PASS\n");
        printf("============================================================\n");

        return EXIT_SUCCESS;
    }

    printf("============================================================\n");
    printf(" INTERRUPT TEST: FAIL\n");
    printf("============================================================\n");

    return EXIT_FAILURE;
}
