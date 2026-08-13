# `beaglebone-black/tests/interrupt/README.md`

````markdown
# BeagleBone Black - Interrupt Tests

This directory contains interrupt validation scripts for the BeagleBone Black.

## Directory Structure

```text
interrupt/
├── README.md
└── test_can_irq.sh
````

## Purpose

The interrupt tests verify that hardware peripherals correctly generate
interrupts and that Linux receives and handles those interrupts through the
appropriate kernel driver.

## Current Tests

| Test    | File              | Purpose                                  |
| ------- | ----------------- | ---------------------------------------- |
| CAN IRQ | `test_can_irq.sh` | Verify CAN controller interrupt activity |

## CAN Interrupt Test

The CAN interrupt test validates the interrupt path:

```text
CAN Bus Activity
       |
       v
CAN Controller
       |
       v
Hardware Interrupt
       |
       v
GIC / Interrupt Controller
       |
       v
Linux Kernel
       |
       v
CAN Driver ISR / Handler
       |
       v
CAN Network Stack
       |
       v
Application
```

## Run

Make the script executable:

```bash
chmod +x test_can_irq.sh
```

Run:

```bash
sudo ./test_can_irq.sh
```

## What the Test Checks

* CAN controller interrupt configuration
* Interrupt line registration
* `/proc/interrupts`
* CAN driver interrupt activity
* RX interrupt activity
* TX interrupt activity
* Interrupt counter changes
* Kernel interrupt messages
* CAN interface status

## Useful Commands

Check all interrupts:

```bash
cat /proc/interrupts
```

Search for CAN-related interrupts:

```bash
cat /proc/interrupts | grep -i can
```

Monitor interrupt counters:

```bash
watch -n 1 'cat /proc/interrupts | grep -i can'
```

Check CAN interface:

```bash
ip -details link show can0
```

Monitor CAN traffic:

```bash
candump can0
```

Generate a CAN frame:

```bash
cansend can0 123#11223344
```

## Expected Result

A successful test should show that the CAN interrupt counter increases when
CAN RX/TX activity occurs.

Example:

```text
CAN interrupt detected
Interrupt count before : 1250
Interrupt count after  : 1251

[PASS] CAN interrupt is active
```

## Debugging

If the interrupt count does not increase, check:

1. CAN Device Tree configuration
2. CAN pinmux configuration
3. CAN controller status
4. CAN driver probe
5. IRQ registration
6. CAN transceiver
7. CAN bus termination
8. CANH/CANL wiring
9. Kernel logs

Check kernel messages:

```bash
dmesg | grep -i can
```

Check registered interrupts:

```bash
cat /proc/interrupts
```

Check Device Tree:

```bash
ls /proc/device-tree/
```

## Result

The test should finish with:

```text
PASS
```

or

```text
FAIL
```

based on the observed CAN interrupt activity.

```
```
==============================================================================================================
[200~Make it executable:

chmod +x beaglebone-black/tests/interrupt/test_can_irq.sh

Run for can0:

sudo ./tests/interrupt/test_can_irq.sh can0
Interrupt test flow
                    test_can_irq.sh
                           |
                           v
                    CAN interface
                           |
                           v
                    CAN controller
                           |
                           v
                    CAN traffic
                           |
                           v
                   Hardware IRQ
                           |
                           v
                    GIC / IRQ
                    Controller
                           |
                           v
                    CAN driver
                           |
                           v
                  /proc/interrupts
                           |
                           v
                 IRQ counter change
                           |
                    +------+------+
                    |             |
                  Increase      No change
                    |             |
                    v             v
                  PASS          DEBUG

The log is generated under:

beaglebone-black/
└── logs/
    └── interrupt/
        └── can_irq_YYYYMMDD_HHMMSS.log

Important: An IRQ counter may not increase from a single cansend if the CAN controller does not receive the expected bus acknowledgment. For a reliable interrupt test, use a second CAN node/transceiver or a properly configured CAN loopback setup.
===============================================================
[200~Make it executable:

chmod +x beaglebone-black/tests/interrupt/test_gpio_irq.sh

Run:

sudo ./tests/interrupt/test_gpio_irq.sh 60
GPIO IRQ flow
        External GPIO Signal
                 |
                 v
          GPIO Pin Transition
          HIGH <-> LOW
                 |
                 v
          GPIO Controller
                 |
                 v
          Hardware IRQ
                 |
                 v
          ARM GIC / IRQ
             Controller
                 |
                 v
           Linux GPIO Driver
                 |
                 v
          Interrupt Handler
                 |
                 v
        /proc/interrupts
                 |
                 v
          IRQ Counter
                 |
                 v
             PASS/FAIL

Important: the script uses the legacy /sys/class/gpio interface for simplicity. On newer kernels, GPIO may instead be exposed through /dev/gpiochipN using the GPIO character-device API. Also, merely reading /sys/class/gpio/gpioN/value does not itself generate an interrupt; the GPIO pin must actually experience the configured edge.
===========================================================
Make it executable:

chmod +x beaglebone-black/tests/interrupt/test_i2c_irq.sh

Run basic I²C IRQ detection:

sudo ./tests/interrupt/test_i2c_irq.sh 2

Run with a connected I²C device at address 0x50:

sudo ./tests/interrupt/test_i2c_irq.sh 2 0x50
I²C IRQ flow
              I2C Transaction
                    |
                    v
              I2C Controller
                    |
                    v
             Hardware IRQ
                    |
                    v
              ARM GIC/IRQ
                Controller
                    |
                    v
              Linux I2C Driver
                    |
                    v
             Interrupt Handler
                    |
                    v
              I2C Completion
                    |
                    v
              /proc/interrupts
                    |
                    v
              IRQ Counter
                    |
                    v
                PASS/FAIL

Log location:

beaglebone-black/
└── logs/
    └── interrupt/
        └── i2c_irq_YYYYMMDD_HHMMSS.log

Note: I²C is normally interrupt-driven in the Linux controller driver, but the exact interrupt behavior depends on the BeagleBone Black kernel/driver implementation. A valid I²C transaction is needed to observe the corresponding controller interrupt activity.
=================================================
Make it executable:

chmod +x beaglebone-black/tests/interrupt/test_spi_irq.sh

Run:

sudo ./tests/interrupt/test_spi_irq.sh /dev/spidev1.0

Or with speed:

sudo ./tests/interrupt/test_spi_irq.sh /dev/spidev1.0 1000000
SPI IRQ flow
             SPI Transfer
                  |
                  v
            SPI Controller
                  |
                  v
           SPI Hardware IRQ
                  |
                  v
           ARM GIC / IRQ
             Controller
                  |
                  v
             SPI Driver
                  |
                  v
          Interrupt Handler
                  |
                  v
            SPI Transfer
             Completion
                  |
                  v
          /proc/interrupts
                  |
                  v
            IRQ Counter
                  |
                  v
              PASS/FAIL

Log file:

beaglebone-black/
└── logs/
    └── interrupt/
        └── spi_irq_YYYYMMDD_HHMMSS.log

Note: The script checks for an increase in SPI-related IRQ counters after an SPI transaction. The exact interrupt behavior depends on the BeagleBone Black kernel's SPI controller driver and configuration.
==================================================================================
Run
chmod +x beaglebone-black/tests/interrupt/test_uart_irq.sh
sudo ./tests/interrupt/test_uart_irq.sh /dev/ttyS2

With a different baud rate:

sudo ./tests/interrupt/test_uart_irq.sh /dev/ttyS2 115200
UART IRQ flow
             UART TX/RX Activity
                     |
                     v
               UART Controller
                     |
                     v
               UART Hardware
                     |
                     v
                  IRQ
                     |
                     v
              ARM GIC / IRQ
                Controller
                     |
                     v
                UART Driver
                     |
                     v
             Interrupt Handler
                     |
                     v
              TTY Subsystem
                     |
                     v
              /dev/ttySx
                     |
                     v
                Application

Log file:

beaglebone-black/
└── logs/
    └── interrupt/
        └── uart_irq_YYYYMMDD_HHMMSS.log

Note: For meaningful RX interrupt validation, use a UART loopback (TX ↔ RX) or a second UART device. TX-only testing can validate transmit-side interrupt activity, while loopback validates both TX and RX paths.
