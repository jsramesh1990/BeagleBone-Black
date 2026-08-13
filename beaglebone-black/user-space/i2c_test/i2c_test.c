/*
 * BeagleBone Black - I2C Test Application
 *
 * File:
 *     i2c_test.c
 *
 * Purpose:
 *     User-space I2C test application using the Linux I2C-dev
 *     interface.
 *
 * Usage:
 *     sudo ./i2c_test <i2c_device> <slave_address> <mode>
 *
 * Modes:
 *     scan
 *     read
 *     write
 *     loopback
 *
 * Examples:
 *     sudo ./i2c_test /dev/i2c-2 0x50 scan
 *     sudo ./i2c_test /dev/i2c-2 0x50 read
 *     sudo ./i2c_test /dev/i2c-2 0x50 write
 *
 * Note:
 *     The I2C bus number and slave address depend on the
 *     BeagleBone Black Device Tree and connected hardware.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include "i2c_test.h"


/* ------------------------------------------------------------------------- */
/* Utility Functions                                                         */
/* ------------------------------------------------------------------------- */

static void print_usage(const char *program)
{
    printf("\n");
    printf("BeagleBone Black I2C Test\n");
    printf("=========================\n\n");

    printf("Usage:\n");
    printf("  sudo %s <device> <address> <mode>\n\n",
           program);

    printf("Modes:\n");
    printf("  scan       Scan the I2C bus\n");
    printf("  read       Read data from I2C slave\n");
    printf("  write      Write test data to I2C slave\n");
    printf("  loopback   Write then read test data\n\n");

    printf("Examples:\n");
    printf("  sudo %s /dev/i2c-2 0x50 scan\n",
           program);

    printf("  sudo %s /dev/i2c-2 0x50 read\n",
           program);

    printf("  sudo %s /dev/i2c-2 0x50 write\n",
           program);

    printf("  sudo %s /dev/i2c-2 0x50 loopback\n",
           program);

    printf("\n");
}


/*
 * Parse I2C slave address.
 */
static int parse_address(const char *address_string,
                         uint8_t *address)
{
    char *end;
    unsigned long value;

    if (address_string == NULL || address == NULL) {
        return -1;
    }

    errno = 0;

    value = strtoul(address_string,
                    &end,
                    0);

    if (errno != 0 ||
        *end != '\0' ||
        value > I2C_MAX_ADDRESS) {

        fprintf(stderr,
                "ERROR: Invalid I2C address: %s\n",
                address_string);

        return -1;
    }

    *address = (uint8_t)value;

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Open I2C Device                                                           */
/* ------------------------------------------------------------------------- */

static int open_i2c_device(const char *device)
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
/* Select I2C Slave                                                          */
/* ------------------------------------------------------------------------- */

static int select_slave(int fd,
                        uint8_t address)
{
    if (ioctl(fd,
              I2C_SLAVE,
              address) < 0) {

        fprintf(stderr,
                "ERROR: Unable to select I2C slave 0x%02X: %s\n",
                address,
                strerror(errno));

        return -1;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* I2C Bus Scan                                                              */
/* ------------------------------------------------------------------------- */

static int scan_i2c_bus(int fd)
{
    int address;
    int result;
    int found = 0;

    printf("\n");
    printf("============================================================\n");
    printf(" I2C BUS SCAN\n");
    printf("============================================================\n");

    printf("Address   Status\n");
    printf("----------------\n");

    /*
     * Scan the valid 7-bit I2C address range.
     */
    for (address = I2C_SCAN_START;
         address <= I2C_SCAN_END;
         address++) {

        /*
         * Set slave address.
         */
        result = ioctl(fd,
                       I2C_SLAVE,
                       address);

        if (result < 0) {
            continue;
        }

        /*
         * A zero-length SMBus quick transaction is commonly used
         * for bus probing.
         */
        result = i2c_smbus_write_quick(fd,
                                       I2C_SMBUS_WRITE);

        if (result >= 0) {

            printf("0x%02X     FOUND\n",
                   address);

            found++;
        }
    }

    printf("\n");

    if (found == 0) {
        printf("No I2C devices detected.\n");
    } else {
        printf("Devices found: %d\n",
               found);
    }

    return found;
}


/* ------------------------------------------------------------------------- */
/* I2C Read                                                                  */
/* ------------------------------------------------------------------------- */

static int read_i2c_data(int fd,
                         uint8_t address)
{
    uint8_t buffer[I2C_TEST_BUFFER_SIZE];

    ssize_t bytes_read;
    int i;

    if (select_slave(fd, address) != 0) {
        return -1;
    }

    memset(buffer,
           0,
           sizeof(buffer));

    /*
     * Read test data from the selected slave.
     */
    bytes_read = read(fd,
                      buffer,
                      sizeof(buffer));

    if (bytes_read < 0) {

        fprintf(stderr,
                "ERROR: I2C read failed: %s\n",
                strerror(errno));

        return -1;
    }

    printf("\n");
    printf("I2C READ\n");
    printf("--------\n");

    printf("Slave Address : 0x%02X\n",
           address);

    printf("Bytes Read    : %zd\n",
           bytes_read);

    printf("Data          : ");

    for (i = 0; i < bytes_read; i++) {
        printf("%02X ",
               buffer[i]);
    }

    printf("\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* I2C Write                                                                 */
/* ------------------------------------------------------------------------- */

static int write_i2c_data(int fd,
                          uint8_t address)
{
    uint8_t buffer[I2C_TEST_BUFFER_SIZE];

    ssize_t bytes_written;
    int i;

    if (select_slave(fd, address) != 0) {
        return -1;
    }

    /*
     * Prepare test pattern.
     */
    for (i = 0;
         i < I2C_TEST_BUFFER_SIZE;
         i++) {

        buffer[i] = (uint8_t)(0x10 + i);
    }

    /*
     * Write data.
     */
    bytes_written = write(fd,
                          buffer,
                          sizeof(buffer));

    if (bytes_written < 0) {

        fprintf(stderr,
                "ERROR: I2C write failed: %s\n",
                strerror(errno));

        return -1;
    }

    if ((size_t)bytes_written != sizeof(buffer)) {

        fprintf(stderr,
                "ERROR: Incomplete I2C write.\n");

        return -1;
    }

    printf("\n");
    printf("I2C WRITE\n");
    printf("---------\n");

    printf("Slave Address : 0x%02X\n",
           address);

    printf("Bytes Written : %zd\n",
           bytes_written);

    printf("Data          : ");

    for (i = 0; i < bytes_written; i++) {
        printf("%02X ",
               buffer[i]);
    }

    printf("\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* I2C Write-Read Test                                                       */
/* ------------------------------------------------------------------------- */

static int loopback_test(int fd,
                         uint8_t address)
{
    uint8_t write_buffer[I2C_TEST_BUFFER_SIZE];
    uint8_t read_buffer[I2C_TEST_BUFFER_SIZE];

    ssize_t bytes_written;
    ssize_t bytes_read;

    int i;

    if (select_slave(fd, address) != 0) {
        return -1;
    }

    /*
     * Prepare test pattern.
     */
    for (i = 0;
         i < I2C_TEST_BUFFER_SIZE;
         i++) {

        write_buffer[i] = (uint8_t)(0xA0 + i);
    }

    memset(read_buffer,
           0,
           sizeof(read_buffer));

    printf("\n");
    printf("============================================================\n");
    printf(" I2C WRITE / READ TEST\n");
    printf("============================================================\n");

    /*
     * Write test pattern.
     */
    bytes_written = write(fd,
                          write_buffer,
                          sizeof(write_buffer));

    if (bytes_written < 0) {

        fprintf(stderr,
                "ERROR: I2C write failed: %s\n",
                strerror(errno));

        return -1;
    }

    if ((size_t)bytes_written != sizeof(write_buffer)) {

        fprintf(stderr,
                "ERROR: Incomplete I2C write.\n");

        return -1;
    }

    printf("TX Data: ");

    for (i = 0; i < bytes_written; i++) {
        printf("%02X ",
               write_buffer[i]);
    }

    printf("\n");

    /*
     * Small delay for devices that need time between operations.
     */
    usleep(I2C_TEST_DELAY_US);

    /*
     * Read data back.
     *
     * IMPORTANT:
     * This assumes the connected I2C device supports reading back
     * the written data. A normal EEPROM/sensor may require a
     * device-specific register/address transaction instead.
     */
    bytes_read = read(fd,
                      read_buffer,
                      sizeof(read_buffer));

    if (bytes_read < 0) {

        fprintf(stderr,
                "ERROR: I2C read failed: %s\n",
                strerror(errno));

        return -1;
    }

    printf("RX Data: ");

    for (i = 0; i < bytes_read; i++) {
        printf("%02X ",
               read_buffer[i]);
    }

    printf("\n");

    /*
     * Validate data.
     */
    if (bytes_read != I2C_TEST_BUFFER_SIZE ||
        memcmp(write_buffer,
               read_buffer,
               I2C_TEST_BUFFER_SIZE) != 0) {

        printf("RESULT: FAIL\n");

        return -1;
    }

    printf("RESULT: PASS\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    const char *device;
    const char *mode;

    uint8_t address;

    int fd;
    int result = EXIT_FAILURE;

    /*
     * Validate arguments.
     */
    if (argc != 4) {

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }

    device = argv[1];
    mode = argv[3];

    /*
     * Parse slave address.
     */
    if (parse_address(argv[2],
                      &address) != 0) {

        return EXIT_FAILURE;
    }

    printf("\n");
    printf("============================================================\n");
    printf(" BeagleBone Black - I2C Test\n");
    printf("============================================================\n");

    printf("I2C Device   : %s\n",
           device);

    printf("Slave Address: 0x%02X\n",
           address);

    printf("Mode         : %s\n",
           mode);

    /*
     * Open I2C device.
     */
    fd = open_i2c_device(device);

    if (fd < 0) {
        return EXIT_FAILURE;
    }

    /*
     * Execute requested test.
     */
    if (strcmp(mode, I2C_MODE_SCAN) == 0) {

        /*
         * Scan does not depend on the address supplied.
         */
        result = (scan_i2c_bus(fd) >= 0)
                     ? EXIT_SUCCESS
                     : EXIT_FAILURE;

    } else if (strcmp(mode, I2C_MODE_READ) == 0) {

        result = (read_i2c_data(fd,
                                address) == 0)
                     ? EXIT_SUCCESS
                     : EXIT_FAILURE;

    } else if (strcmp(mode, I2C_MODE_WRITE) == 0) {

        result = (write_i2c_data(fd,
                                 address) == 0)
                     ? EXIT_SUCCESS
                     : EXIT_FAILURE;

    } else if (strcmp(mode, I2C_MODE_LOOPBACK) == 0) {

        result = (loopback_test(fd,
                                address) == 0)
                     ? EXIT_SUCCESS
                     : EXIT_FAILURE;

    } else {

        fprintf(stderr,
                "ERROR: Unknown I2C mode: %s\n",
                mode);

        print_usage(argv[0]);
    }

    close(fd);

    printf("\n");

    if (result == EXIT_SUCCESS) {

        printf("============================================================\n");
        printf(" I2C TEST: PASS\n");
        printf("============================================================\n");

    } else {

        printf("============================================================\n");
        printf(" I2C TEST: FAIL\n");
        printf("============================================================\n");
    }

    return result;
}
