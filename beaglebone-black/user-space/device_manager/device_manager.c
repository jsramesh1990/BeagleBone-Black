/*
 * BeagleBone Black - Device Manager
 *
 * File:
 *     device_manager.c
 *
 * Purpose:
 *     User-space utility for discovering, monitoring and displaying
 *     BeagleBone Black peripheral devices exposed through Linux
 *     device interfaces.
 *
 * Supported interfaces:
 *     GPIO
 *     I2C
 *     SPI
 *     UART
 *     CAN
 *     PWM
 *     ADC / IIO
 *     RTC
 *
 * Usage:
 *     ./device_manager
 *     ./device_manager list
 *     ./device_manager status
 *     ./device_manager scan
 *
 * Note:
 *     This application does not directly control hardware. It provides
 *     a user-space overview of Linux-exposed peripheral interfaces.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "device_manager.h"

/* ------------------------------------------------------------------------- */
/* Utility Functions                                                         */
/* ------------------------------------------------------------------------- */

/*
 * Check whether a path exists.
 */
static int path_exists(const char *path)
{
    struct stat st;

    if (path == NULL) {
        return 0;
    }

    return (stat(path, &st) == 0);
}


/*
 * Print a section header.
 */
static void print_section(const char *title)
{
    printf("\n");
    printf("============================================================\n");
    printf(" %s\n", title);
    printf("============================================================\n");
}


/*
 * Print a directory's entries.
 */
static void list_directory(const char *path)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);

    if (dir == NULL) {
        printf("  Not available: %s\n", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        printf("  %-30s\n", entry->d_name);
    }

    closedir(dir);
}


/* ------------------------------------------------------------------------- */
/* GPIO                                                                     */
/* ------------------------------------------------------------------------- */

static void check_gpio(void)
{
    print_section("GPIO");

    if (path_exists("/dev/gpiochip0")) {

        printf("Status : AVAILABLE\n");
        printf("Device : /dev/gpiochip0\n");

        printf("\nGPIO chips:\n");
        list_directory("/dev");

    } else {

        printf("Status : NOT DETECTED\n");
        printf("Expected: /dev/gpiochip*\n");
    }
}


/* ------------------------------------------------------------------------- */
/* I2C                                                                      */
/* ------------------------------------------------------------------------- */

static void check_i2c(void)
{
    print_section("I2C");

    if (path_exists("/dev/i2c-0")) {

        printf("Status : AVAILABLE\n");

        printf("\nI2C devices:\n");
        list_directory("/dev");

    } else {

        printf("Status : CHECKING AVAILABLE I2C DEVICES\n");

        list_directory("/dev");
    }
}


/* ------------------------------------------------------------------------- */
/* SPI                                                                      */
/* ------------------------------------------------------------------------- */

static void check_spi(void)
{
    DIR *dir;
    struct dirent *entry;
    int found = 0;

    print_section("SPI");

    dir = opendir("/dev");

    if (dir == NULL) {

        printf("Status : Unable to access /dev\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (strncmp(entry->d_name,
                    "spidev",
                    6) == 0) {

            printf("  /dev/%s\n",
                   entry->d_name);

            found = 1;
        }
    }

    closedir(dir);

    if (found) {
        printf("Status : AVAILABLE\n");
    } else {
        printf("Status : NOT DETECTED\n");
    }
}


/* ------------------------------------------------------------------------- */
/* UART                                                                     */
/* ------------------------------------------------------------------------- */

static void check_uart(void)
{
    DIR *dir;
    struct dirent *entry;
    int found = 0;

    print_section("UART");

    dir = opendir("/dev");

    if (dir == NULL) {

        printf("Status : Unable to access /dev\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (strncmp(entry->d_name,
                    "ttyS",
                    4) == 0 ||
            strncmp(entry->d_name,
                    "ttyO",
                    4) == 0 ||
            strncmp(entry->d_name,
                    "ttyUSB",
                    6) == 0 ||
            strncmp(entry->d_name,
                    "ttyACM",
                    6) == 0) {

            printf("  /dev/%s\n",
                   entry->d_name);

            found = 1;
        }
    }

    closedir(dir);

    if (found) {
        printf("Status : AVAILABLE\n");
    } else {
        printf("Status : NOT DETECTED\n");
    }
}


/* ------------------------------------------------------------------------- */
/* CAN                                                                      */
/* ------------------------------------------------------------------------- */

static void check_can(void)
{
    DIR *dir;
    struct dirent *entry;
    int found = 0;

    print_section("CAN");

    /*
     * CAN interfaces are normally represented as network interfaces
     * rather than /dev nodes.
     */
    dir = opendir("/sys/class/net");

    if (dir == NULL) {

        printf("Status : Unable to access network interfaces\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (strncmp(entry->d_name,
                    "can",
                    3) == 0) {

            printf("  Interface: %s\n",
                   entry->d_name);

            found = 1;
        }
    }

    closedir(dir);

    if (found) {
        printf("Status : AVAILABLE\n");
    } else {
        printf("Status : NOT DETECTED\n");
    }
}


/* ------------------------------------------------------------------------- */
/* PWM                                                                      */
/* ------------------------------------------------------------------------- */

static void check_pwm(void)
{
    print_section("PWM");

    if (path_exists("/sys/class/pwm")) {

        printf("Status : PWM SUBSYSTEM AVAILABLE\n");
        printf("Path   : /sys/class/pwm\n");

        list_directory("/sys/class/pwm");

    } else {

        printf("Status : NOT DETECTED\n");
    }
}


/* ------------------------------------------------------------------------- */
/* ADC / IIO                                                                */
/* ------------------------------------------------------------------------- */

static void check_adc(void)
{
    print_section("ADC / IIO");

    if (path_exists("/sys/bus/iio/devices")) {

        printf("Status : IIO SUBSYSTEM AVAILABLE\n");
        printf("Path   : /sys/bus/iio/devices\n");

        list_directory("/sys/bus/iio/devices");

    } else {

        printf("Status : NOT DETECTED\n");
    }
}


/* ------------------------------------------------------------------------- */
/* RTC                                                                      */
/* ------------------------------------------------------------------------- */

static void check_rtc(void)
{
    DIR *dir;
    struct dirent *entry;
    int found = 0;

    print_section("RTC");

    dir = opendir("/dev");

    if (dir == NULL) {

        printf("Status : Unable to access /dev\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (strncmp(entry->d_name,
                    "rtc",
                    3) == 0) {

            printf("  /dev/%s\n",
                   entry->d_name);

            found = 1;
        }
    }

    closedir(dir);

    if (found) {
        printf("Status : AVAILABLE\n");
    } else {
        printf("Status : NOT DETECTED\n");
    }
}


/* ------------------------------------------------------------------------- */
/* Device List                                                              */
/* ------------------------------------------------------------------------- */

static void list_devices(void)
{
    print_section("BEAGLEBONE BLACK DEVICE LIST");

    check_gpio();
    check_i2c();
    check_spi();
    check_uart();
    check_can();
    check_pwm();
    check_adc();
    check_rtc();
}


/* ------------------------------------------------------------------------- */
/* System Status                                                            */
/* ------------------------------------------------------------------------- */

static void show_status(void)
{
    print_section("SYSTEM STATUS");

    printf("Hostname:\n");

    if (system("hostname") != 0) {
        printf("Unable to read hostname.\n");
    }

    printf("\nKernel:\n");

    if (system("uname -r") != 0) {
        printf("Unable to read kernel version.\n");
    }

    printf("\nArchitecture:\n");

    if (system("uname -m") != 0) {
        printf("Unable to read architecture.\n");
    }

    printf("\nUptime:\n");

    if (system("uptime") != 0) {
        printf("Unable to read uptime.\n");
    }

    printf("\nMemory:\n");

    if (system("free -h") != 0) {
        printf("Unable to read memory information.\n");
    }
}


/* ------------------------------------------------------------------------- */
/* Peripheral Scan                                                          */
/* ------------------------------------------------------------------------- */

static void scan_devices(void)
{
    print_section("PERIPHERAL SCAN");

    printf("Scanning Linux device interfaces...\n");

    printf("\n/dev entries:\n");
    list_directory("/dev");

    printf("\nNetwork interfaces:\n");
    list_directory("/sys/class/net");

    printf("\nIIO devices:\n");
    list_directory("/sys/bus/iio/devices");

    printf("\nPWM devices:\n");
    list_directory("/sys/class/pwm");

    printf("\nGPIO chips:\n");
    list_directory("/dev");
}


/* ------------------------------------------------------------------------- */
/* Help                                                                     */
/* ------------------------------------------------------------------------- */

static void print_usage(const char *program)
{
    printf("\n");
    printf("BeagleBone Black Device Manager\n");
    printf("================================\n\n");

    printf("Usage:\n");
    printf("  %s <command>\n\n", program);

    printf("Commands:\n");
    printf("  list       List supported peripheral devices\n");
    printf("  status     Display system status\n");
    printf("  scan       Scan Linux device interfaces\n");
    printf("  help       Display this help message\n");

    printf("\nExamples:\n");
    printf("  %s list\n", program);
    printf("  %s status\n", program);
    printf("  %s scan\n", program);

    printf("\n");
}


/* ------------------------------------------------------------------------- */
/* Main                                                                     */
/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    printf("\n");
    printf("============================================================\n");
    printf(" BeagleBone Black - Device Manager\n");
    printf("============================================================\n");

    /*
     * No command:
     * Display all peripheral information.
     */
    if (argc == 1) {

        list_devices();

        printf("\nDevice manager completed.\n\n");

        return EXIT_SUCCESS;
    }

    /*
     * Validate argument count.
     */
    if (argc != 2) {

        fprintf(stderr,
                "ERROR: Invalid number of arguments.\n");

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }

    /*
     * List command.
     */
    if (strcmp(argv[1], "list") == 0) {

        list_devices();

        return EXIT_SUCCESS;
    }

    /*
     * Status command.
     */
    if (strcmp(argv[1], "status") == 0) {

        show_status();

        return EXIT_SUCCESS;
    }

    /*
     * Scan command.
     */
    if (strcmp(argv[1], "scan") == 0) {

        scan_devices();

        return EXIT_SUCCESS;
    }

    /*
     * Help command.
     */
    if (strcmp(argv[1], "help") == 0) {

        print_usage(argv[0]);

        return EXIT_SUCCESS;
    }

    /*
     * Unknown command.
     */
    fprintf(stderr,
            "ERROR: Unknown command: %s\n",
            argv[1]);

    print_usage(argv[0]);

    return EXIT_FAILURE;
}
