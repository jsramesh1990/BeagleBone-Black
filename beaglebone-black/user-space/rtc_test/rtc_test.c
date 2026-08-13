/*
 * BeagleBone Black - RTC Test Application
 *
 * File:
 *     rtc_test.c
 *
 * Purpose:
 *     User-space RTC test using the Linux RTC character-device
 *     interface.
 *
 * Usage:
 *     sudo ./rtc_test <rtc_device> <mode>
 *
 * Modes:
 *     read
 *     set
 *     alarm
 *     periodic
 *
 * Examples:
 *     sudo ./rtc_test /dev/rtc0 read
 *     sudo ./rtc_test /dev/rtc0 set
 *     sudo ./rtc_test /dev/rtc0 alarm
 *     sudo ./rtc_test /dev/rtc0 periodic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>

#include "rtc_test.h"


/* ------------------------------------------------------------------------- */
/* Usage                                                                     */
/* ------------------------------------------------------------------------- */

static void print_usage(const char *program)
{
    printf("\n");
    printf("BeagleBone Black RTC Test\n");
    printf("=========================\n\n");

    printf("Usage:\n");
    printf("  sudo %s <rtc_device> <mode>\n\n", program);

    printf("Modes:\n");
    printf("  read       Read current RTC time\n");
    printf("  set        Display current RTC time\n");
    printf("  alarm      Test RTC alarm interrupt\n");
    printf("  periodic   Test periodic RTC interrupt\n\n");

    printf("Examples:\n");
    printf("  sudo %s /dev/rtc0 read\n", program);
    printf("  sudo %s /dev/rtc0 set\n", program);
    printf("  sudo %s /dev/rtc0 alarm\n", program);
    printf("  sudo %s /dev/rtc0 periodic\n", program);

    printf("\n");
}


/* ------------------------------------------------------------------------- */
/* Open RTC                                                                  */
/* ------------------------------------------------------------------------- */

static int open_rtc(const char *device)
{
    int fd;

    fd = open(device, O_RDWR);

    if (fd < 0) {
        fprintf(stderr,
                "ERROR: Unable to open %s: %s\n",
                device,
                strerror(errno));

        return -1;
    }

    return fd;
}


/* ------------------------------------------------------------------------- */
/* Read RTC Time                                                             */
/* ------------------------------------------------------------------------- */

static int read_rtc_time(int fd)
{
    struct rtc_time rtc_tm;

    memset(&rtc_tm, 0, sizeof(rtc_tm));

    if (ioctl(fd, RTC_RD_TIME, &rtc_tm) < 0) {
        fprintf(stderr,
                "ERROR: RTC_RD_TIME failed: %s\n",
                strerror(errno));

        return -1;
    }

    printf("\n");
    printf("============================================================\n");
    printf(" RTC TIME\n");
    printf("============================================================\n");

    printf("Date : %04d-%02d-%02d\n",
           rtc_tm.tm_year + 1900,
           rtc_tm.tm_mon + 1,
           rtc_tm.tm_mday);

    printf("Time : %02d:%02d:%02d\n",
           rtc_tm.tm_hour,
           rtc_tm.tm_min,
           rtc_tm.tm_sec);

    printf("============================================================\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* RTC Information                                                           */
/* ------------------------------------------------------------------------- */

static int read_rtc_info(int fd)
{
    struct rtc_time rtc_tm;

    memset(&rtc_tm, 0, sizeof(rtc_tm));

    if (ioctl(fd, RTC_RD_TIME, &rtc_tm) < 0) {
        return -1;
    }

    printf("RTC Driver Status\n");
    printf("-----------------\n");
    printf("RTC device is accessible\n");
    printf("Current RTC time: %04d-%02d-%02d %02d:%02d:%02d\n",
           rtc_tm.tm_year + 1900,
           rtc_tm.tm_mon + 1,
           rtc_tm.tm_mday,
           rtc_tm.tm_hour,
           rtc_tm.tm_min,
           rtc_tm.tm_sec);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* RTC Alarm Test                                                            */
/* ------------------------------------------------------------------------- */

static int rtc_alarm_test(int fd)
{
    struct rtc_time rtc_tm;

    unsigned long data;

    printf("\n");
    printf("============================================================\n");
    printf(" RTC ALARM TEST\n");
    printf("============================================================\n");

    /*
     * Read current RTC time.
     */
    memset(&rtc_tm, 0, sizeof(rtc_tm));

    if (ioctl(fd, RTC_RD_TIME, &rtc_tm) < 0) {
        fprintf(stderr,
                "ERROR: Failed to read RTC time: %s\n",
                strerror(errno));

        return -1;
    }

    /*
     * Disable any existing alarm.
     */
    if (ioctl(fd, RTC_AIE_OFF, 0) < 0) {
        fprintf(stderr,
                "ERROR: RTC_AIE_OFF failed: %s\n",
                strerror(errno));

        return -1;
    }

    /*
     * Set alarm one minute ahead.
     *
     * RTC alarm support is hardware dependent. The exact alarm
     * fields supported by the RTC driver may vary.
     */
    rtc_tm.tm_min++;

    if (rtc_tm.tm_min >= 60) {
        rtc_tm.tm_min = 0;
    }

    if (ioctl(fd, RTC_ALM_SET, &rtc_tm) < 0) {
        fprintf(stderr,
                "ERROR: RTC_ALM_SET failed: %s\n",
                strerror(errno));

        return -1;
    }

    /*
     * Enable alarm interrupt.
     */
    if (ioctl(fd, RTC_AIE_ON, 0) < 0) {
        fprintf(stderr,
                "ERROR: RTC_AIE_ON failed: %s\n",
                strerror(errno));

        return -1;
    }

    printf("RTC alarm enabled.\n");
    printf("Waiting for RTC alarm interrupt...\n");

    /*
     * RTC interrupt event is returned by read().
     */
    if (read(fd, &data, sizeof(data)) < 0) {
        fprintf(stderr,
                "ERROR: RTC alarm read failed: %s\n",
                strerror(errno));

        ioctl(fd, RTC_AIE_OFF, 0);

        return -1;
    }

    printf("RTC alarm interrupt received.\n");

    /*
     * Disable alarm.
     */
    ioctl(fd, RTC_AIE_OFF, 0);

    printf("RTC alarm test: PASS\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Periodic Interrupt Test                                                   */
/* ------------------------------------------------------------------------- */

static int rtc_periodic_test(int fd)
{
    unsigned long data;
    int i;

    printf("\n");
    printf("============================================================\n");
    printf(" RTC PERIODIC INTERRUPT TEST\n");
    printf("============================================================\n");

    /*
     * Enable periodic interrupts.
     */
    if (ioctl(fd,
              RTC_PIE_ON,
              0) < 0) {

        fprintf(stderr,
                "ERROR: RTC_PIE_ON failed: %s\n",
                strerror(errno));

        return -1;
    }

    /*
     * Read several periodic events.
     */
    for (i = 0;
         i < RTC_PERIODIC_EVENT_COUNT;
         i++) {

        if (read(fd,
                 &data,
                 sizeof(data)) < 0) {

            fprintf(stderr,
                    "ERROR: RTC periodic read failed: %s\n",
                    strerror(errno));

            ioctl(fd, RTC_PIE_OFF, 0);

            return -1;
        }

        printf("Periodic RTC interrupt %d received\n",
               i + 1);
    }

    /*
     * Disable periodic interrupts.
     */
    if (ioctl(fd,
              RTC_PIE_OFF,
              0) < 0) {

        fprintf(stderr,
                "ERROR: RTC_PIE_OFF failed: %s\n",
                strerror(errno));

        return -1;
    }

    printf("RTC periodic interrupt test: PASS\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    const char *device;
    const char *mode;

    int fd;
    int result = EXIT_FAILURE;

    if (argc != 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    device = argv[1];
    mode = argv[2];

    printf("\n");
    printf("============================================================\n");
    printf(" BeagleBone Black - RTC Test\n");
    printf("============================================================\n");

    printf("RTC Device : %s\n", device);
    printf("Mode       : %s\n", mode);

    /*
     * Open RTC character device.
     */
    fd = open_rtc(device);

    if (fd < 0) {
        return EXIT_FAILURE;
    }

    /*
     * Execute selected test.
     */
    if (strcmp(mode, RTC_MODE_READ) == 0) {

        result = read_rtc_time(fd);

    } else if (strcmp(mode, RTC_MODE_SET) == 0) {

        /*
         * This mode verifies that the RTC can be accessed.
         * Actual RTC setting should normally be done using
         * the system date/time tools or a dedicated implementation.
         */
        result = read_rtc_info(fd);

    } else if (strcmp(mode, RTC_MODE_ALARM) == 0) {

        result = rtc_alarm_test(fd);

    } else if (strcmp(mode, RTC_MODE_PERIODIC) == 0) {

        result = rtc_periodic_test(fd);

    } else {

        fprintf(stderr,
                "ERROR: Unknown RTC mode: %s\n",
                mode);

        print_usage(argv[0]);
    }

    close(fd);

    printf("\n");

    if (result == 0) {
        printf("============================================================\n");
        printf(" RTC TEST: PASS\n");
        printf("============================================================\n");

        return EXIT_SUCCESS;
    }

    printf("============================================================\n");
    printf(" RTC TEST: FAIL\n");
    printf("============================================================\n");

    return EXIT_FAILURE;
}
