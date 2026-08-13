/*
 * BeagleBone Black - SPI Test Application
 *
 * File:
 *     spi_test.c
 *
 * Purpose:
 *     User-space SPI test application using the Linux spidev
 *     interface.
 *
 * Usage:
 *     sudo ./spi_test <spi_device> <mode>
 *
 * Modes:
 *     read
 *     write
 *     transfer
 *     loopback
 *
 * Examples:
 *     sudo ./spi_test /dev/spidev0.0 read
 *     sudo ./spi_test /dev/spidev0.0 write
 *     sudo ./spi_test /dev/spidev0.0 transfer
 *     sudo ./spi_test /dev/spidev0.0 loopback
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/spi/spidev.h>

#include "spi_test.h"


/* ------------------------------------------------------------------------- */
/* Usage                                                                     */
/* ------------------------------------------------------------------------- */

static void print_usage(const char *program)
{
    printf("\n");
    printf("BeagleBone Black SPI Test\n");
    printf("=========================\n\n");

    printf("Usage:\n");
    printf("  sudo %s <spi_device> <mode>\n\n", program);

    printf("Modes:\n");
    printf("  read       Perform SPI read test\n");
    printf("  write      Perform SPI write test\n");
    printf("  transfer   Perform SPI full-duplex transfer\n");
    printf("  loopback   Perform SPI loopback test\n\n");

    printf("Examples:\n");
    printf("  sudo %s /dev/spidev0.0 read\n", program);
    printf("  sudo %s /dev/spidev0.0 write\n", program);
    printf("  sudo %s /dev/spidev0.0 transfer\n", program);
    printf("  sudo %s /dev/spidev0.0 loopback\n", program);

    printf("\n");
}


/* ------------------------------------------------------------------------- */
/* Open SPI Device                                                           */
/* ------------------------------------------------------------------------- */

static int open_spi_device(const char *device)
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
/* Configure SPI                                                              */
/* ------------------------------------------------------------------------- */

static int configure_spi(int fd)
{
    uint8_t mode = SPI_DEFAULT_MODE;
    uint8_t bits = SPI_DEFAULT_BITS;
    uint32_t speed = SPI_DEFAULT_SPEED_HZ;

    /*
     * Configure SPI mode.
     */
    if (ioctl(fd,
              SPI_IOC_WR_MODE,
              &mode) < 0) {

        fprintf(stderr,
                "ERROR: SPI mode configuration failed: %s\n",
                strerror(errno));

        return -1;
    }

    /*
     * Configure bits per word.
     */
    if (ioctl(fd,
              SPI_IOC_WR_BITS_PER_WORD,
              &bits) < 0) {

        fprintf(stderr,
                "ERROR: SPI bits-per-word configuration failed: %s\n",
                strerror(errno));

        return -1;
    }

    /*
     * Configure SPI clock speed.
     */
    if (ioctl(fd,
              SPI_IOC_WR_MAX_SPEED_HZ,
              &speed) < 0) {

        fprintf(stderr,
                "ERROR: SPI speed configuration failed: %s\n",
                strerror(errno));

        return -1;
    }

    printf("SPI Mode       : %u\n", mode);
    printf("Bits/Word      : %u\n", bits);
    printf("Speed          : %u Hz\n", speed);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Display Buffer                                                            */
/* ------------------------------------------------------------------------- */

static void print_buffer(const char *name,
                         const uint8_t *buffer,
                         size_t length)
{
    size_t i;

    printf("%s: ", name);

    for (i = 0; i < length; i++) {
        printf("%02X ", buffer[i]);
    }

    printf("\n");
}


/* ------------------------------------------------------------------------- */
/* SPI Transfer                                                              */
/* ------------------------------------------------------------------------- */

static int spi_transfer(int fd,
                        const uint8_t *tx_buffer,
                        uint8_t *rx_buffer,
                        size_t length)
{
    struct spi_ioc_transfer transfer;

    memset(&transfer,
           0,
           sizeof(transfer));

    transfer.tx_buf =
        (unsigned long)tx_buffer;

    transfer.rx_buf =
        (unsigned long)rx_buffer;

    transfer.len =
        length;

    transfer.speed_hz =
        SPI_DEFAULT_SPEED_HZ;

    transfer.bits_per_word =
        SPI_DEFAULT_BITS;

    if (ioctl(fd,
              SPI_IOC_MESSAGE(1),
              &transfer) < 0) {

        fprintf(stderr,
                "ERROR: SPI transfer failed: %s\n",
                strerror(errno));

        return -1;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* SPI Write Test                                                            */
/* ------------------------------------------------------------------------- */

static int spi_write_test(int fd)
{
    uint8_t tx_buffer[SPI_TEST_BUFFER_SIZE];

    size_t i;

    printf("\n");
    printf("============================================================\n");
    printf(" SPI WRITE TEST\n");
    printf("============================================================\n");

    /*
     * Prepare test pattern.
     */
    for (i = 0;
         i < SPI_TEST_BUFFER_SIZE;
         i++) {

        tx_buffer[i] =
            SPI_TEST_DATA_START + i;
    }

    print_buffer("TX",
                 tx_buffer,
                 sizeof(tx_buffer));

    /*
     * SPI write using full-duplex transfer.
     */
    if (spi_transfer(fd,
                     tx_buffer,
                     NULL,
                     sizeof(tx_buffer)) != 0) {

        return -1;
    }

    printf("SPI write completed successfully.\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* SPI Read Test                                                             */
/* ------------------------------------------------------------------------- */

static int spi_read_test(int fd)
{
    uint8_t tx_buffer[SPI_TEST_BUFFER_SIZE];
    uint8_t rx_buffer[SPI_TEST_BUFFER_SIZE];

    size_t i;

    printf("\n");
    printf("============================================================\n");
    printf(" SPI READ TEST\n");
    printf("============================================================\n");

    memset(tx_buffer,
           0xFF,
           sizeof(tx_buffer));

    memset(rx_buffer,
           0,
           sizeof(rx_buffer));

    /*
     * Generate clock by transmitting dummy bytes.
     */
    if (spi_transfer(fd,
                     tx_buffer,
                     rx_buffer,
                     sizeof(rx_buffer)) != 0) {

        return -1;
    }

    print_buffer("RX",
                 rx_buffer,
                 sizeof(rx_buffer));

    printf("SPI read completed successfully.\n");

    /*
     * Note:
     * The returned data depends on the connected SPI slave.
     */
    for (i = 0;
         i < sizeof(rx_buffer);
         i++) {

        (void)rx_buffer[i];
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Full-Duplex Transfer Test                                                 */
/* ------------------------------------------------------------------------- */

static int spi_full_duplex_test(int fd)
{
    uint8_t tx_buffer[SPI_TEST_BUFFER_SIZE];
    uint8_t rx_buffer[SPI_TEST_BUFFER_SIZE];

    size_t i;

    printf("\n");
    printf("============================================================\n");
    printf(" SPI FULL-DUPLEX TRANSFER TEST\n");
    printf("============================================================\n");

    for (i = 0;
         i < SPI_TEST_BUFFER_SIZE;
         i++) {

        tx_buffer[i] =
            SPI_TEST_DATA_START + i;

        rx_buffer[i] = 0;
    }

    print_buffer("TX",
                 tx_buffer,
                 sizeof(tx_buffer));

    if (spi_transfer(fd,
                     tx_buffer,
                     rx_buffer,
                     sizeof(tx_buffer)) != 0) {

        return -1;
    }

    print_buffer("RX",
                 rx_buffer,
                 sizeof(rx_buffer));

    printf("SPI full-duplex transfer completed.\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* SPI Loopback Test                                                         */
/* ------------------------------------------------------------------------- */

static int spi_loopback_test(int fd)
{
    uint8_t tx_buffer[SPI_TEST_BUFFER_SIZE];
    uint8_t rx_buffer[SPI_TEST_BUFFER_SIZE];

    size_t i;

    printf("\n");
    printf("============================================================\n");
    printf(" SPI LOOPBACK TEST\n");
    printf("============================================================\n");

    /*
     * Prepare known pattern.
     */
    for (i = 0;
         i < SPI_TEST_BUFFER_SIZE;
         i++) {

        tx_buffer[i] =
            SPI_LOOPBACK_DATA_START + i;

        rx_buffer[i] = 0;
    }

    print_buffer("TX",
                 tx_buffer,
                 sizeof(tx_buffer));

    /*
     * Perform full-duplex transfer.
     *
     * For loopback testing, MOSI must be physically connected
     * to MISO.
     */
    if (spi_transfer(fd,
                     tx_buffer,
                     rx_buffer,
                     sizeof(tx_buffer)) != 0) {

        return -1;
    }

    print_buffer("RX",
                 rx_buffer,
                 sizeof(rx_buffer));

    /*
     * Compare transmitted and received data.
     */
    if (memcmp(tx_buffer,
               rx_buffer,
               sizeof(tx_buffer)) != 0) {

        printf("RESULT: FAIL\n");
        printf("TX and RX data do not match.\n");

        return -1;
    }

    printf("TX and RX data match.\n");
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
    printf(" BeagleBone Black - SPI Test\n");
    printf("============================================================\n");

    printf("SPI Device : %s\n", device);
    printf("Mode       : %s\n", mode);

    /*
     * Open SPI device.
     */
    fd = open_spi_device(device);

    if (fd < 0) {
        return EXIT_FAILURE;
    }

    /*
     * Configure SPI.
     */
    if (configure_spi(fd) != 0) {

        close(fd);

        return EXIT_FAILURE;
    }

    /*
     * Execute requested test.
     */
    if (strcmp(mode, SPI_MODE_READ) == 0) {

        result = spi_read_test(fd);

    } else if (strcmp(mode, SPI_MODE_WRITE) == 0) {

        result = spi_write_test(fd);

    } else if (strcmp(mode, SPI_MODE_TRANSFER) == 0) {

        result = spi_full_duplex_test(fd);

    } else if (strcmp(mode, SPI_MODE_LOOPBACK) == 0) {

        result = spi_loopback_test(fd);

    } else {

        fprintf(stderr,
                "ERROR: Unknown SPI mode: %s\n",
                mode);

        print_usage(argv[0]);
    }

    close(fd);

    printf("\n");

    if (result == 0) {

        printf("============================================================\n");
        printf(" SPI TEST: PASS\n");
        printf("============================================================\n");

        return EXIT_SUCCESS;
    }

    printf("============================================================\n");
    printf(" SPI TEST: FAIL\n");
    printf("============================================================\n");

    return EXIT_FAILURE;
}
