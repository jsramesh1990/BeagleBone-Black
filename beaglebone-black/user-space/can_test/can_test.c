/*
 * BeagleBone Black - CAN Test Application
 *
 * File:
 *     can_test.c
 *
 * Purpose:
 *     User-space CAN test application using the Linux SocketCAN
 *     interface.
 *
 * Usage:
 *     sudo ./can_test
 *     sudo ./can_test can0
 *     sudo ./can_test can0 tx
 *     sudo ./can_test can0 rx
 *     sudo ./can_test can0 loopback
 *
 * Examples:
 *     sudo ./can_test can0 tx
 *     sudo ./can_test can0 rx
 *     sudo ./can_test can0 loopback
 *
 * Before running:
 *     sudo ip link set can0 down
 *     sudo ip link set can0 type can bitrate 500000
 *     sudo ip link set can0 up
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "can_test.h"


/* ------------------------------------------------------------------------- */
/* Helper Functions                                                          */
/* ------------------------------------------------------------------------- */

/*
 * Print application usage.
 */
static void print_usage(const char *program)
{
    printf("\n");
    printf("BeagleBone Black CAN Test\n");
    printf("=========================\n\n");

    printf("Usage:\n");
    printf("  sudo %s <interface> <mode>\n\n", program);

    printf("Modes:\n");
    printf("  tx         Transmit CAN frames\n");
    printf("  rx         Receive CAN frames\n");
    printf("  loopback   Transmit and receive using CAN loopback\n\n");

    printf("Examples:\n");
    printf("  sudo %s can0 tx\n", program);
    printf("  sudo %s can0 rx\n", program);
    printf("  sudo %s can0 loopback\n", program);

    printf("\n");
}


/*
 * Display CAN frame information.
 */
static void print_can_frame(const struct can_frame *frame)
{
    int i;

    printf("CAN ID: 0x%03X | DLC: %d | Data:",
           frame->can_id & CAN_SFF_MASK,
           frame->can_dlc);

    for (i = 0; i < frame->can_dlc; i++) {
        printf(" %02X", frame->data[i]);
    }

    printf("\n");
}


/*
 * Open SocketCAN raw socket.
 */
static int can_socket_open(const char *interface)
{
    int socket_fd;

    struct ifreq ifr;
    struct sockaddr_can address;

    /*
     * Create raw CAN socket.
     */
    socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (socket_fd < 0) {
        perror("socket");
        return -1;
    }

    /*
     * Get interface index.
     */
    memset(&ifr, 0, sizeof(ifr));

    strncpy(ifr.ifr_name,
            interface,
            IFNAMSIZ - 1);

    if (ioctl(socket_fd,
              SIOCGIFINDEX,
              &ifr) < 0) {

        perror("SIOCGIFINDEX");

        close(socket_fd);

        return -1;
    }

    /*
     * Configure CAN address.
     */
    memset(&address, 0, sizeof(address));

    address.can_family = AF_CAN;
    address.can_ifindex = ifr.ifr_ifindex;

    /*
     * Bind socket to CAN interface.
     */
    if (bind(socket_fd,
             (struct sockaddr *)&address,
             sizeof(address)) < 0) {

        perror("bind");

        close(socket_fd);

        return -1;
    }

    return socket_fd;
}


/*
 * Send one CAN frame.
 */
static int can_send_frame(int socket_fd)
{
    struct can_frame frame;

    ssize_t bytes_written;

    memset(&frame, 0, sizeof(frame));

    /*
     * Standard 11-bit CAN identifier.
     */
    frame.can_id = 0x123;

    /*
     * Eight data bytes.
     */
    frame.can_dlc = 8;

    frame.data[0] = 0x11;
    frame.data[1] = 0x22;
    frame.data[2] = 0x33;
    frame.data[3] = 0x44;
    frame.data[4] = 0x55;
    frame.data[5] = 0x66;
    frame.data[6] = 0x77;
    frame.data[7] = 0x88;

    /*
     * Send frame.
     */
    bytes_written = write(socket_fd,
                           &frame,
                           sizeof(frame));

    if (bytes_written < 0) {
        perror("CAN write");
        return -1;
    }

    if ((size_t)bytes_written != sizeof(frame)) {

        fprintf(stderr,
                "ERROR: Incomplete CAN frame transmitted.\n");

        return -1;
    }

    printf("CAN TX: ");
    print_can_frame(&frame);

    return 0;
}


/*
 * Receive one CAN frame.
 */
static int can_receive_frame(int socket_fd)
{
    struct can_frame frame;

    ssize_t bytes_read;

    memset(&frame, 0, sizeof(frame));

    /*
     * Wait for incoming CAN frame.
     */
    bytes_read = read(socket_fd,
                      &frame,
                      sizeof(frame));

    if (bytes_read < 0) {
        perror("CAN read");
        return -1;
    }

    if ((size_t)bytes_read != sizeof(frame)) {

        fprintf(stderr,
                "ERROR: Invalid CAN frame size: %zd\n",
                bytes_read);

        return -1;
    }

    printf("CAN RX: ");
    print_can_frame(&frame);

    return 0;
}


/*
 * Configure CAN loopback.
 *
 * CAN_RAW_LOOPBACK is a SocketCAN socket option. It controls whether
 * transmitted frames are looped back to local CAN sockets.
 */
static int enable_loopback(int socket_fd)
{
    int enable = 1;

    if (setsockopt(socket_fd,
                   SOL_CAN_RAW,
                   CAN_RAW_LOOPBACK,
                   &enable,
                   sizeof(enable)) < 0) {

        perror("CAN_RAW_LOOPBACK");

        return -1;
    }

    return 0;
}


/*
 * Check CAN interface state using ioctl.
 */
static int check_interface(const char *interface)
{
    int fd;

    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));

    strncpy(ifr.ifr_name,
            interface,
            IFNAMSIZ - 1);

    if (ioctl(fd,
              SIOCGIFFLAGS,
              &ifr) < 0) {

        fprintf(stderr,
                "ERROR: CAN interface '%s' not found.\n",
                interface);

        close(fd);

        return -1;
    }

    printf("Interface : %s\n",
           interface);

    if (ifr.ifr_flags & IFF_UP) {
        printf("State     : UP\n");
    } else {
        printf("State     : DOWN\n");
    }

    close(fd);

    return 0;
}


/*
 * Transmit multiple CAN frames.
 */
static int transmit_test(int socket_fd)
{
    int i;

    printf("\n");
    printf("Starting CAN TX test...\n");
    printf("---------------------------------------------\n");

    for (i = 0; i < 10; i++) {

        struct can_frame frame;

        ssize_t bytes_written;

        memset(&frame, 0, sizeof(frame));

        frame.can_id = 0x100 + i;
        frame.can_dlc = 8;

        frame.data[0] = i;
        frame.data[1] = 0x10;
        frame.data[2] = 0x20;
        frame.data[3] = 0x30;
        frame.data[4] = 0x40;
        frame.data[5] = 0x50;
        frame.data[6] = 0x60;
        frame.data[7] = 0x70;

        bytes_written = write(socket_fd,
                               &frame,
                               sizeof(frame));

        if (bytes_written < 0) {

            perror("CAN TX");

            return -1;
        }

        printf("Frame %d: ", i + 1);

        print_can_frame(&frame);

        usleep(100000);
    }

    printf("---------------------------------------------\n");
    printf("CAN TX test completed.\n");

    return 0;
}


/*
 * Receive multiple CAN frames.
 */
static int receive_test(int socket_fd)
{
    int i;

    printf("\n");
    printf("Starting CAN RX test...\n");
    printf("---------------------------------------------\n");

    for (i = 0; i < 10; i++) {

        if (can_receive_frame(socket_fd) != 0) {
            return -1;
        }
    }

    printf("---------------------------------------------\n");
    printf("CAN RX test completed.\n");

    return 0;
}


/*
 * CAN loopback test.
 */
static int loopback_test(int socket_fd)
{
    struct can_frame tx_frame;
    struct can_frame rx_frame;

    ssize_t bytes_written;
    ssize_t bytes_read;

    int i;

    memset(&tx_frame, 0, sizeof(tx_frame));
    memset(&rx_frame, 0, sizeof(rx_frame));

    /*
     * Enable SocketCAN local loopback.
     */
    if (enable_loopback(socket_fd) != 0) {
        return -1;
    }

    printf("\n");
    printf("Starting CAN loopback test...\n");
    printf("---------------------------------------------\n");

    /*
     * Prepare frame.
     */
    tx_frame.can_id = 0x321;
    tx_frame.can_dlc = 8;

    for (i = 0; i < 8; i++) {
        tx_frame.data[i] = (uint8_t)(i + 1);
    }

    /*
     * Transmit.
     */
    bytes_written = write(socket_fd,
                           &tx_frame,
                           sizeof(tx_frame));

    if (bytes_written < 0) {

        perror("CAN loopback TX");

        return -1;
    }

    printf("TX Frame: ");
    print_can_frame(&tx_frame);

    /*
     * Receive loopback frame.
     */
    bytes_read = read(socket_fd,
                      &rx_frame,
                      sizeof(rx_frame));

    if (bytes_read < 0) {

        perror("CAN loopback RX");

        return -1;
    }

    printf("RX Frame: ");
    print_can_frame(&rx_frame);

    /*
     * Validate received frame.
     */
    if (rx_frame.can_id != tx_frame.can_id ||
        rx_frame.can_dlc != tx_frame.can_dlc ||
        memcmp(rx_frame.data,
               tx_frame.data,
               tx_frame.can_dlc) != 0) {

        printf("RESULT: FAIL\n");

        return -1;
    }

    printf("---------------------------------------------\n");
    printf("RESULT: PASS\n");
    printf("CAN loopback test successful.\n");

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    const char *interface;
    const char *mode;

    int socket_fd;
    int result = EXIT_FAILURE;

    /*
     * Validate command line.
     */
    if (argc != 3) {

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }

    interface = argv[1];
    mode = argv[2];

    /*
     * Header.
     */
    printf("\n");
    printf("============================================================\n");
    printf(" BeagleBone Black - CAN Test\n");
    printf("============================================================\n");

    /*
     * Check interface.
     */
    if (check_interface(interface) != 0) {
        return EXIT_FAILURE;
    }

    /*
     * Open CAN SocketCAN interface.
     */
    socket_fd = can_socket_open(interface);

    if (socket_fd < 0) {

        fprintf(stderr,
                "ERROR: Failed to open CAN interface '%s'.\n",
                interface);

        return EXIT_FAILURE;
    }

    printf("SocketCAN  : READY\n");
    printf("Mode       : %s\n",
           mode);

    /*
     * Select test mode.
     */
    if (strcmp(mode, "tx") == 0) {

        result = transmit_test(socket_fd);

    } else if (strcmp(mode, "rx") == 0) {

        result = receive_test(socket_fd);

    } else if (strcmp(mode, "loopback") == 0) {

        result = loopback_test(socket_fd);

    } else {

        fprintf(stderr,
                "ERROR: Unknown CAN test mode: %s\n",
                mode);

        print_usage(argv[0]);
    }

    /*
     * Close socket.
     */
    close(socket_fd);

    /*
     * Final result.
     */
    printf("\n");

    if (result == 0) {

        printf("============================================================\n");
        printf(" CAN TEST: PASS\n");
        printf("============================================================\n");

        return EXIT_SUCCESS;
    }

    printf("============================================================\n");
    printf(" CAN TEST: FAIL\n");
    printf("============================================================\n");

    return EXIT_FAILURE;
}
