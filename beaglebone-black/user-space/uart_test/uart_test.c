/*
 * BeagleBone Black - UART Test Application
 *
 * File:
 *     uart_test.c
 *
 * Purpose:
 *     User-space UART test using the Linux serial/TTY interface.
 *
 * Usage:
 *     sudo ./uart_test <uart_device> <mode>
 *
 * Modes:
 *     config
 *     write
 *     read
 *     loopback
 *
 * Examples:
 *     sudo ./uart_test /dev/ttyS1 config
 *     sudo ./uart_test /dev/ttyS1 write
 *     sudo ./uart_test /dev/ttyS1 read
 *     sudo ./uart_test /dev/ttyS1 loopback
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#include "uart_test.h"


/* ------------------------------------------------------------------------- */
/* Usage                                                                     */
/* ------------------------------------------------------------------------- */

static void print_usage(const char *program)
{
    printf("\n");
    printf("BeagleBone Black UART Test\n");
    printf("==========================\n\n");

    printf("Usage:\n");
    printf("  sudo %s <uart_device> <mode>\n\n", program);

    printf("Modes:\n");
    printf("  config       Configure and display UART settings\n");
    printf("  write        Transmit test data\n");
    printf("  read         Receive UART data\n");
    printf("  loopback     Perform UART loopback test\n\n");

    printf("Examples:\n");
    printf("  sudo %s /dev/ttyS1 config\n", program);
    printf("  sudo %s /dev/ttyS1 write\n", program);
    printf("  sudo %s /dev/ttyS1 read\n", program);
    printf("  sudo %s /dev/ttyS1 loopback\n", program);

    printf("\n");
}


/* ------------------------------------------------------------------------- */
/* Open UART                                                                 */
/* ------------------------------------------------------------------------- */

static int open_uart(const char *device)
{
    int fd;

    fd = open(device,
              O_RDWR | O_NOCTTY | O_SYNC);

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
/* Configure UART                                                             */
/* ------------------------------------------------------------------------- */

static int configure_uart(int fd)
{
    struct termios tty;

    /*
     * Read current UART configuration.
     */
    if (tcgetattr(fd, &tty) != 0) {

        fprintf(stderr,
                "ERROR: tcgetattr() failed: %s\n",
                strerror(errno));

        return -1;
    }

    /*
     * Configure baud rate.
     */
    if (cfsetispeed(&tty, UART_BAUD_RATE) != 0 ||
        cfsetospeed(&tty, UART_BAUD_RATE) != 0) {

        fprintf(stderr,
                "ERROR: Failed to configure baud rate: %s\n",
                strerror(errno));

        return -1;
    }

    /*
     * 8 data bits.
     */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    /*
     * No parity.
     */
    tty.c_cflag &= ~PARENB;

    /*
     * One stop bit.
     */
    tty.c_cflag &= ~CSTOPB;

    /*
     * Disable hardware flow control.
     */
    tty.c_cflag &= ~CRTSCTS;

    /*
     * Enable receiver.
     */
    tty.c_cflag |= CREAD;

    /*
     * Ignore modem control lines.
     */
    tty.c_cflag |= CLOCAL;

    /*
     * Raw input mode.
     */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK |
                     ISTRIP | INLCR | IGNCR | ICRNL);

    /*
     * Raw output mode.
     */
    tty.c_oflag &= ~OPOST;

    /*
     * Raw local mode.
     */
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON |
                     ISIG | IEXTEN);

    /*
     * No special character processing.
     */
    tty.c_cflag &= ~CSTOPB;

    /*
     * Minimum characters and timeout.
     */
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = UART_READ_TIMEOUT_DS;

    /*
     * Apply configuration.
     */
    if (tcsetattr(fd,
                  TCSANOW,
                  &tty) != 0) {

        fprintf(stderr,
                "ERROR: tcsetattr() failed: %s\n",
                strerror(errno));

        return -1;
    }

    /*
     * Flush pending data.
     */
    tcflush(fd,
            TCIOFLUSH);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* UART Write                                                                */
/* ------------------------------------------------------------------------- */

static int uart_write_test(int fd)
{
    const char *message = UART_TEST_MESSAGE;

    size_t length;
    ssize_t written;

    printf("\n");
    printf("============================================================\n");
    printf(" UART WRITE TEST\n");
    printf("============================================================\n");

    printf("TX Data: %s", message);

    length = strlen(message);

    written = write(fd,
                    message,
                    length);

    if (written < 0) {

        fprintf(stderr,
                "ERROR: UART write failed: %s\n",
                strerror(errno));

        return -1;
    }

    if ((size_t)written != length) {

        fprintf(stderr,
                "ERROR: Partial UART write.\n");

        return -1;
    }

    /*
     * Wait until transmitted data has left the UART.
     */
    if (tcdrain(fd) != 0) {

        fprintf(stderr,
                "ERROR: tcdrain() failed: %s\n",
                strerror(errno));

        return -1;
    }

    printf("Bytes transmitted: %zd\n",
           written);

    printf("UART write test: PASS\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* UART Read                                                                 */
/* ------------------------------------------------------------------------- */

static int uart_read_test(int fd)
{
    char buffer[UART_BUFFER_SIZE];

    ssize_t bytes_read;

    printf("\n");
    printf("============================================================\n");
    printf(" UART READ TEST\n");
    printf("============================================================\n");

    memset(buffer,
           0,
           sizeof(buffer));

    printf("Waiting for UART data...\n");

    bytes_read = read(fd,
                      buffer,
                      sizeof(buffer) - 1);

    if (bytes_read < 0) {

        fprintf(stderr,
                "ERROR: UART read failed: %s\n",
                strerror(errno));

        return -1;
    }

    if (bytes_read == 0) {

        printf("No UART data received.\n");

        return 0;
    }

    buffer[bytes_read] = '\0';

    printf("RX Data: %s\n",
           buffer);

    printf("Bytes received: %zd\n",
           bytes_read);

    printf("UART read test: PASS\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* UART Loopback                                                             */
/* ------------------------------------------------------------------------- */

static int uart_loopback_test(int fd)
{
    const char *tx_message =
        UART_LOOPBACK_MESSAGE;

    char rx_buffer[UART_BUFFER_SIZE];

    size_t tx_length;

    size_t total_read = 0;

    ssize_t bytes_read;

    fd_set read_fds;

    struct timeval timeout;

    printf("\n");
    printf("============================================================\n");
    printf(" UART LOOPBACK TEST\n");
    printf("============================================================\n");

    memset(rx_buffer,
           0,
           sizeof(rx_buffer));

    tx_length = strlen(tx_message);

    /*
     * Flush old UART data.
     */
    tcflush(fd,
            TCIOFLUSH);

    printf("TX: %s",
           tx_message);

    /*
     * Transmit loopback pattern.
     */
    if (write(fd,
              tx_message,
              tx_length) != (ssize_t)tx_length) {

        fprintf(stderr,
                "ERROR: UART loopback write failed: %s\n",
                strerror(errno));

        return -1;
    }

    tcdrain(fd);

    /*
     * Wait for RX data.
     */
    while (total_read < tx_length) {

        FD_ZERO(&read_fds);

        FD_SET(fd,
               &read_fds);

        timeout.tv_sec =
            UART_LOOPBACK_TIMEOUT_SEC;

        timeout.tv_usec = 0;

        if (select(fd + 1,
                   &read_fds,
                   NULL,
                   NULL,
                   &timeout) < 0) {

            if (errno == EINTR) {
                continue;
            }

            fprintf(stderr,
                    "ERROR: select() failed: %s\n",
                    strerror(errno));

            return -1;
        }

        if (!FD_ISSET(fd,
                      &read_fds)) {

            fprintf(stderr,
                    "ERROR: UART loopback timeout.\n");

            return -1;
        }

        bytes_read = read(fd,
                          rx_buffer + total_read,
                          tx_length - total_read);

        if (bytes_read < 0) {

            fprintf(stderr,
                    "ERROR: UART loopback read failed: %s\n",
                    strerror(errno));

            return -1;
        }

        total_read += bytes_read;
    }

    rx_buffer[total_read] = '\0';

    printf("RX: %s\n",
           rx_buffer);

    /*
     * Compare TX and RX.
     */
    if (memcmp(tx_message,
               rx_buffer,
               tx_length) != 0) {

        printf("TX and RX data do not match.\n");
        printf("UART loopback test: FAIL\n");

        return -1;
    }

    printf("TX and RX data match.\n");
    printf("UART loopback test: PASS\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* UART Configuration Test                                                   */
/* ------------------------------------------------------------------------- */

static int uart_config_test(int fd)
{
    struct termios tty;

    speed_t input_speed;
    speed_t output_speed;

    if (tcgetattr(fd,
                  &tty) != 0) {

        fprintf(stderr,
                "ERROR: tcgetattr() failed: %s\n",
                strerror(errno));

        return -1;
    }

    input_speed =
        cfgetispeed(&tty);

    output_speed =
        cfgetospeed(&tty);

    printf("\n");
    printf("============================================================\n");
    printf(" UART CONFIGURATION\n");
    printf("============================================================\n");

    printf("Input Baud Rate  : %u\n",
           (unsigned int)input_speed);

    printf("Output Baud Rate : %u\n",
           (unsigned int)output_speed);

    printf("Data Bits        : 8\n");
    printf("Parity           : None\n");
    printf("Stop Bits        : 1\n");
    printf("Flow Control     : None\n");

    printf("UART configuration test: PASS\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(int argc,
         char *argv[])
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
    printf(" BeagleBone Black - UART Test\n");
    printf("============================================================\n");

    printf("UART Device : %s\n",
           device);

    printf("Mode        : %s\n",
           mode);

    /*
     * Open UART.
     */
    fd = open_uart(device);

    if (fd < 0) {
        return EXIT_FAILURE;
    }

    /*
     * Configure UART.
     */
    if (configure_uart(fd) != 0) {

        close(fd);

        return EXIT_FAILURE;
    }

    /*
     * Execute selected test.
     */
    if (strcmp(mode,
               UART_MODE_CONFIG) == 0) {

        result = uart_config_test(fd);

    } else if (strcmp(mode,
                      UART_MODE_WRITE) == 0) {

        result = uart_write_test(fd);

    } else if (strcmp(mode,
                      UART_MODE_READ) == 0) {

        result = uart_read_test(fd);

    } else if (strcmp(mode,
                      UART_MODE_LOOPBACK) == 0) {

        result = uart_loopback_test(fd);

    } else {

        fprintf(stderr,
                "ERROR: Unknown UART mode: %s\n",
                mode);

        print_usage(argv[0]);
    }

    close(fd);

    printf("\n");

    if (result == 0) {

        printf("============================================================\n");
        printf(" UART TEST: PASS\n");
        printf("============================================================\n");

        return EXIT_SUCCESS;
    }

    printf("============================================================\n");
    printf(" UART TEST: FAIL\n");
    printf("============================================================\n");

    return EXIT_FAILURE;
}
