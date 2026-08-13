# BeagleBone Black GPIO Button Input Driver

## Overview

This driver demonstrates how to connect a physical push button to
the Linux input subsystem using a GPIO interrupt.

The driver uses:

- GPIO descriptor API
- GPIO interrupt
- Linux Input subsystem
- Device Tree
- GPIO debounce
- Rising and falling edge detection

---

## Directory Structure

```text
11_button_input/
├── button_input.c
├── button_input.h
├── Makefile
└── README.md
Driver Architecture
                 Push Button
                      |
                      v
                  GPIO Pin
                      |
                      v
                GPIO Controller
                      |
                      v
                  GPIO IRQ
                      |
                      v
             button_irq_handler()
                      |
                      v
              Linux Input Core
                      |
                      v
                 /dev/input/
                      |
                      v
              User Application
Button Event Flow
Button Press
     |
     v
GPIO changes state
     |
     v
Rising/Falling Edge
     |
     v
GPIO Interrupt
     |
     v
IRQ Handler
     |
     v
gpiod_get_value()
     |
     v
input_report_key()
     |
     v
input_sync()
     |
     v
User Space
Device Tree

The driver expects a GPIO property named:

button-gpios

Example:

button_test {
    compatible = "bbb,button-input";

    button-gpios = <&gpio1 28 GPIO_ACTIVE_LOW>;

    status = "okay";
};

The GPIO number and controller must be changed according to the
actual BeagleBone Black pin being used.

GPIO Active Level

For an active-low button:

button-gpios = <&gpio1 28 GPIO_ACTIVE_LOW>;

For an active-high button:

button-gpios = <&gpio1 28 GPIO_ACTIVE_HIGH>;

The GPIO descriptor API handles the active-level configuration.

Build
make

Expected module:

button_input.ko

Verify:

ls -l button_input.ko
Load Driver
sudo insmod button_input.ko

Check:

lsmod | grep button_input

Check kernel logs:

dmesg | grep -i button
Input Device

Check:

cat /proc/bus/input/devices

Look for:

BBB GPIO Button

Check input devices:

ls -l /dev/input/

Example:

/dev/input/event0
/dev/input/event1
/dev/input/event2
Test Button Events

Install evtest if it is available on the target:

sudo apt install evtest

List input devices:

sudo evtest

Select:

BBB GPIO Button

Press and release the physical button.

Expected events are similar to:

EV_KEY       BTN_0       1
EV_SYN       SYN_REPORT  0

EV_KEY       BTN_0       0
EV_SYN       SYN_REPORT  0

Where:

1 = Pressed
0 = Released
Interrupt Configuration

The driver requests:

Rising edge
+
Falling edge

Therefore:

Button Press
     |
     v
GPIO transition
     |
     v
IRQ
     |
     v
Report BTN_0 = 1

and:

Button Release
     |
     v
GPIO transition
     |
     v
IRQ
     |
     v
Report BTN_0 = 0
Debouncing

Mechanical buttons can generate multiple transitions when pressed.

Example:

Actual press:

______/‾‾‾‾‾‾‾

Bouncing:

____/\/\_/\/\____‾‾‾

The driver requests:

20 ms debounce

using:

gpiod_set_debounce()

Hardware support for debounce depends on the GPIO controller.

If hardware debounce is unavailable, the driver prints a warning.

Debugging

Check GPIO-related messages:

dmesg | grep -i gpio

Check button driver:

dmesg | grep -i button

Check IRQ information:

cat /proc/interrupts

Press the button and check whether the interrupt count changes.

Check Input Device
cat /proc/bus/input/devices

Example:

N: Name="BBB GPIO Button"
P: Phys=bbb/button0
H: Handlers=eventX
B: KEY=...
Remove Driver
sudo rmmod button_input

Verify:

lsmod | grep button_input
Makefile Commands

Build:

make

Clean:

make clean

Load:

make load

Unload:

make unload

Status:

make status

Information:

make info

Test:

make test

Logs:

make logs
Complete Flow
                Device Tree
                     |
                     v
              button-gpios
                     |
                     v
               GPIO Driver
                     |
                     v
                GPIO IRQ
                     |
                     v
             button_input.c
                     |
                     v
            Linux Input Core
                     |
                     v
              /dev/input/eventX
                     |
                     v
              evtest / Application
Important Note

For production Linux systems, the preferred architecture for a
physical button is generally:

Device Tree
     |
     v
GPIO Input Driver
     |
     v
Linux Input Subsystem
     |
     v
User Application

If the button is only required as a simple GPIO state and does not
need Linux input events, a GPIO consumer interface may be sufficient.

For buttons, keys, switches and user controls, however, the Linux
Input subsystem is normally the more appropriate abstraction.


### Final structure

```text
beaglebone-black/
└── drivers/
    └── 11_button_input/
        ├── button_input.c
        ├── button_input.h
        ├── Makefile
        └── README.md

Build on the target kernel with:

cd beaglebone-black/drivers/11_button_input
make
sudo insmod button_input.ko
dmesg | tail -20

Then use evtest to verify the press/release events generated by the GPIO button.
