# BeagleBone Black UART Driver

## Overview

This directory contains a basic Linux kernel UART driver for the
BeagleBone Black.

The driver demonstrates:

- Platform driver registration
- Device Tree matching
- UART memory resource retrieval
- `devm_ioremap_resource()`
- UART register access using `readl()` and `writel()`
- UART transmit
- UART receive
- Character device creation
- User-space read/write interface
- Kernel module build and loading

---

## Directory Structure

```text
06_uart/
├── uart_driver.c
├── uart_driver.h
├── Makefile
└── README.md
Driver Architecture
                 Device Tree
                      |
                      v
              UART Platform Device
                      |
                      v
              uart_platform_driver
                      |
                      v
                 uart_probe()
                      |
             +--------+--------+
             |                 |
             v                 v
       Get MEM resource    ioremap
             |                 |
             +--------+--------+
                      |
                      v
                UART Registers
                      |
                      v
              /dev/bbb_uart
                      |
             +--------+--------+
             |                 |
             v                 v
          read()            write()
             |                 |
             v                 v
          UART RX            UART TX
Device Tree

The driver expects a Device Tree compatible string:

compatible = "bbb,uart-test";

Example structure:

uart_test {
    compatible = "bbb,uart-test";
    reg = <0x00000000 0x00001000>;
    status = "okay";
};

The actual reg address must be replaced with the UART peripheral
address for the target hardware.

Build

Run:

make

Expected output:

uart_driver.ko

Check:

ls -l uart_driver.ko
Load Driver
sudo insmod uart_driver.ko

Check:

lsmod | grep uart_driver

Check kernel messages:

dmesg | tail -50
Device Node

After successful probe:

/dev/bbb_uart

Check:

ls -l /dev/bbb_uart
Test TX

Send data from user space:

echo "Hello UART" > /dev/bbb_uart

The driver sends the characters through the UART TX register.

Test RX

A user-space program can read:

read(fd, buffer, sizeof(buffer));

The driver waits for the UART data-ready status and reads the
received character from the RX register.

Makefile Commands

Build:

make

Load:

make load

Unload:

make unload

Status:

make status

Logs:

make logs

Test:

make test

Clean:

make clean
Important UART Registers

The example driver uses logical register offsets defined in
uart_driver.h:

UART_RBR_REG
UART_THR_REG
UART_LSR_REG
UART_LCR_REG

Typical functions are:

RBR -> Receive Buffer Register
THR -> Transmit Holding Register
LSR -> Line Status Register
LCR -> Line Control Register
UART TX Flow
User Application
       |
       | write()
       v
/dev/bbb_uart
       |
       v
uart_write()
       |
       v
uart_send_char()
       |
       v
Check THRE
       |
       v
UART THR Register
       |
       v
UART TX Pin
UART RX Flow
UART RX Pin
     |
     v
UART RX Hardware
     |
     v
RBR Register
     |
     v
uart_receive_char()
     |
     v
uart_read()
     |
     v
User Application
Debugging

Check driver:

lsmod | grep uart_driver

Check device:

ls -l /dev/bbb_uart

Check kernel logs:

dmesg | grep -i uart

Check platform devices:

ls /sys/bus/platform/devices/

Check driver binding:

ls /sys/bus/platform/drivers/
Important Note

This is an educational/example UART platform driver.

A production BeagleBone Black UART normally uses the existing Linux
UART subsystem and the SoC-specific UART driver rather than creating
a second raw character driver for the same UART hardware.

The register offsets, base address, pinmux, clock configuration,
interrupt configuration, baud-rate setup, and Device Tree properties
must match the actual UART hardware being used.


### Directory

```text
beaglebone-black/
└── drivers/
    └── 06_uart/
        ├── uart_driver.c
        ├── uart_driver.h
        ├── Makefile
        └── README.md

Important: uart_driver.c includes uart_driver.h, so that header is also required. The register offsets in the header must match the actual UART IP used by your BeagleBone Black/AM335x target.

