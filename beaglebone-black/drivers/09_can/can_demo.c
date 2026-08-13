/*
 * BeagleBone Black - CAN Demo
 *
 * File:
 *     can_demo.c
 *
 * Purpose:
 *     SocketCAN demonstration program for:
 *       - CAN interface configuration
 *       - CAN frame transmission
 *       - CAN frame reception
 *       - CAN frame validation
 *
 * Usage:
 *     ./can_demo
 *
 * The program uses the Linux SocketCAN framework.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "can_demo.h"


/* ------------------------------------------------------------------------- */
/* Open CAN Socket                                                            */
/* ------------------------------------------------------------------------- */

static int can_open_socket(const char *interface)
{
    int sockfd;

    struct ifreq ifr;

    struct sockaddr_can addr;


    /*
     * Create raw CAN socket.
     */
    sockfd = socket(PF_CAN,
                    SOCK_RAW,
                    CAN_RAW);

    if (sockfd < 0) {

        perror("socket");

        return -1;
    }


    /*
     * Get CAN interface index.
     */
    memset(&ifr,
           0,
           sizeof(ifr));

    strncpy(ifr.ifr_name,
            interface,
            IFNAMSIZ - 1);

    if (ioctl(sockfd,
              SIOCGIFINDEX,
              &ifr) < 0) {

        perror("SIOCGIFINDEX");

        close(sockfd);

        return -1;
    }


    /*
     * Configure CAN socket address.
     */
    memset(&addr,
           0,
           sizeof(addr));

    addr.can_family = AF_CAN;

    addr.can_ifindex =
        ifr.ifr_ifindex;


    /*
     * Bind socket to CAN interface.
     */
    if (bind(sockfd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {

        perror("bind");

        close(sockfd);

        return -1;
    }


    return sockfd;
}


/* ------------------------------------------------------------------------- */
/* Send CAN Frame                                                             */
/* ------------------------------------------------------------------------- */

static int can_send_frame(int sockfd)
{
    struct can_frame frame;

    ssize_t bytes;


    memset(&frame,
           0,
           sizeof(frame));


    /*
     * Standard CAN ID.
     */
    frame.can_id =
        CAN_TX_ID;


    /*
     * CAN payload length.
     */
    frame.can_dlc =
        CAN_TEST_DATA_LENGTH;


    /*
     * Example payload.
     */
    frame.data[0] = 0x11;
    frame.data[1] = 0x22;
    frame.data[2] = 0x33;
    frame.data[3] = 0x44;
    frame.data[4] = 0x55;
    frame.data[5] = 0x66;
    frame.data[6] = 0x77;
    frame.data[7] = 0x88;


    bytes = write(sockfd,
                  &frame,
                  sizeof(frame));


    if (bytes != sizeof(frame)) {

        perror("CAN write");

        return -1;
    }


    printf("CAN TX: ID=0x%03X DLC=%d DATA=",
           frame.can_id,
           frame.can_dlc);


    for (int i = 0;
         i < frame.can_dlc;
         i++) {

        printf("%02X ",
               frame.data[i]);
    }

    printf("\n");


    return 0;
}


/* ------------------------------------------------------------------------- */
/* Receive CAN Frame                                                         */
/* ------------------------------------------------------------------------- */

static int can_receive_frame(int sockfd)
{
    struct can_frame frame;

    ssize_t bytes;


    bytes = read(sockfd,
                 &frame,
                 sizeof(frame));


    if (bytes < 0) {

        perror("CAN read");

        return -1;
    }


    if (bytes != sizeof(frame)) {

        fprintf(stderr,
                "Invalid CAN frame size\n");

        return -1;
    }


    printf("CAN RX: ID=0x%03X DLC=%d DATA=",
           frame.can_id &
           CAN_STANDARD_ID_MASK,
           frame.can_dlc);


    for (int i = 0;
         i < frame.can_dlc;
         i++) {

        printf("%02X ",
               frame.data[i]);
    }


    printf("\n");


    return 0;
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(void)
{
    int sockfd;


    printf("=====================================\n");
    printf(" BeagleBone Black CAN Demo\n");
    printf("=====================================\n");


    /*
     * Open CAN interface.
     */
    sockfd =
        can_open_socket(
            CAN_DEFAULT_INTERFACE);

    if (sockfd < 0) {

        fprintf(stderr,
                "Failed to open %s\n",
                CAN_DEFAULT_INTERFACE);

        return EXIT_FAILURE;
    }


    printf("CAN interface: %s\n",
           CAN_DEFAULT_INTERFACE);

    printf("CAN bitrate: %d\n",
           CAN_DEFAULT_BITRATE);


    /*
     * Send example frame.
     */
    if (can_send_frame(sockfd) < 0) {

        close(sockfd);

        return EXIT_FAILURE;
    }


    /*
     * Receive frame.
     *
     * This is blocking until a CAN frame
     * is received.
     */
    printf("Waiting for CAN frame...\n");

    can_receive_frame(sockfd);


    close(sockfd);


    printf("CAN demo completed\n");


    return EXIT_SUCCESS;
}
