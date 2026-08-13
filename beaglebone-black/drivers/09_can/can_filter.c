/*
 * BeagleBone Black - CAN Filter Demo
 *
 * File:
 *     can_filter.c
 *
 * Purpose:
 *     Demonstrates SocketCAN hardware-independent
 *     CAN ID filtering in user space.
 *
 * Usage:
 *     ./can_filter
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

#include "can_filter.h"


/* ------------------------------------------------------------------------- */
/* Open CAN Socket                                                           */
/* ------------------------------------------------------------------------- */

static int can_filter_open(void)
{
    int sockfd;

    struct ifreq ifr;

    struct sockaddr_can addr;


    /*
     * Create SocketCAN raw socket.
     */
    sockfd = socket(PF_CAN,
                    SOCK_RAW,
                    CAN_RAW);

    if (sockfd < 0) {

        perror("socket");

        return -1;
    }


    /*
     * Find CAN interface index.
     */
    memset(&ifr,
           0,
           sizeof(ifr));

    strncpy(ifr.ifr_name,
            "can0",
            IFNAMSIZ - 1);


    if (ioctl(sockfd,
              SIOCGIFINDEX,
              &ifr) < 0) {

        perror("SIOCGIFINDEX");

        close(sockfd);

        return -1;
    }


    /*
     * Configure CAN address.
     */
    memset(&addr,
           0,
           sizeof(addr));

    addr.can_family =
        AF_CAN;

    addr.can_ifindex =
        ifr.ifr_ifindex;


    /*
     * Bind socket.
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
/* Configure CAN Filter                                                      */
/* ------------------------------------------------------------------------- */

static int can_configure_filter(int sockfd)
{
    struct can_filter filter;


    memset(&filter,
           0,
           sizeof(filter));


    /*
     * Receive only CAN ID 0x200.
     */
    filter.can_id =
        CAN_FILTER_ID;

    filter.can_mask =
        CAN_FILTER_MASK;


    if (setsockopt(sockfd,
                   SOL_CAN_RAW,
                   CAN_RAW_FILTER,
                   &filter,
                   sizeof(filter)) < 0) {

        perror("setsockopt");

        return -1;
    }


    printf("CAN filter configured:\n");
    printf("  ID   : 0x%03X\n",
           CAN_FILTER_ID);
    printf("  MASK : 0x%03X\n",
           CAN_FILTER_MASK);


    return 0;
}


/* ------------------------------------------------------------------------- */
/* Receive Filtered Frame                                                    */
/* ------------------------------------------------------------------------- */

static int can_receive_filtered(int sockfd)
{
    struct can_frame frame;

    ssize_t bytes;


    while (1) {

        bytes = read(sockfd,
                     &frame,
                     sizeof(frame));


        if (bytes < 0) {

            if (errno == EINTR)
                continue;

            perror("read");

            return -1;
        }


        if (bytes != sizeof(frame)) {

            fprintf(stderr,
                    "Invalid CAN frame\n");

            continue;
        }


        printf("\nFiltered CAN frame:\n");

        printf("ID   : 0x%03X\n",
               frame.can_id &
               CAN_FILTER_MASK);

        printf("DLC  : %d\n",
               frame.can_dlc);

        printf("DATA : ");


        for (int i = 0;
             i < frame.can_dlc;
             i++) {

            printf("%02X ",
                   frame.data[i]);
        }


        printf("\n");
    }


    return 0;
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(void)
{
    int sockfd;


    printf("=====================================\n");
    printf(" BeagleBone Black CAN Filter Demo\n");
    printf("=====================================\n");


    sockfd =
        can_filter_open();

    if (sockfd < 0) {

        fprintf(stderr,
                "Failed to open CAN interface\n");

        return EXIT_FAILURE;
    }


    if (can_configure_filter(sockfd) < 0) {

        close(sockfd);

        return EXIT_FAILURE;
    }


    printf("Waiting for CAN ID 0x%03X...\n",
           CAN_FILTER_ID);


    can_receive_filtered(sockfd);


    close(sockfd);

    return EXIT_SUCCESS;
}
