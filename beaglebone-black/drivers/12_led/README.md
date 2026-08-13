# BeagleBone Black GPIO LED Driver

## Overview

This driver demonstrates GPIO-based LED control using the Linux GPIO
descriptor API.

The driver supports:

- Device Tree GPIO configuration
- GPIO output control
- LED ON/OFF
- Platform driver model
- Sysfs control interface
- Mutex protection
- Automatic LED OFF during driver removal

---

## Directory Structure

```text
12_led/
├── led_driver.c
├── led_driver.h
├── Makefile
└── README.md
Driver Architecture
                 Device Tree
                      |
                      v
                led-gpios
                      |
                      v
               Platform Device
                      |
                      v
                  probe()
                      |
                      v
              GPIO Descriptor
                      |
                      v
                GPIO Output
                      |
                      v
                   LED
LED Control Flow
User Space
    |
    v
Sysfs
    |
    v
state_store()
    |
    +---- 1 ----> LED ON
    |
    +---- 0 ----> LED OFF
    |
    v
gpiod_set_value_cansleep()
    |
    v
GPIO Controller
    |
    v
Physical LED
Device Tree

The driver expects:

led-gpios

Example:

led_test {
    compatible = "bbb,gpio-led";

    led-gpios = <&gpio1 28 GPIO_ACTIVE_HIGH>;

    status = "okay";
};

The GPIO controller and pin must be changed according to the actual
hardware connection.

Active High LED

For an active-high LED:

led-gpios = <&gpio1 28 GPIO_ACTIVE_HIGH>;

GPIO HIGH:

GPIO = 1
LED  = ON

GPIO LOW:

GPIO = 0
LED  = OFF
Active Low LED

For an active-low LED:

led-gpios = <&gpio1 28 GPIO_ACTIVE_LOW>;

The GPIO descriptor API handles the active-low polarity.

Therefore the driver can continue using:

gpiod_set_value_cansleep(gpio, 1);

for logical LED ON.

Build
make

Expected output:

led_driver.ko

Check:

ls -l led_driver.ko
Load Driver
sudo insmod led_driver.ko

Check:

lsmod | grep led_driver

Check logs:

dmesg | grep -i led
Find Device

After loading:

ls /sys/bus/platform/devices/

Find the platform device associated with:

bbb,gpio-led

Check the driver:

readlink /sys/bus/platform/devices/<device>/driver
LED ON

The driver exposes:

state

under the platform device.

Example:

echo 1 | sudo tee \
/sys/bus/platform/devices/<device>/state

Expected:

LED ON
LED OFF
echo 0 | sudo tee \
/sys/bus/platform/devices/<device>/state

Expected:

LED OFF
Read LED State
cat /sys/bus/platform/devices/<device>/state

Output:

1

means:

LED ON

Output:

0

means:

LED OFF
Driver Flow
Device Tree
     |
     v
compatible = "bbb,gpio-led"
     |
     v
Platform Driver Match
     |
     v
probe()
     |
     +---- Allocate driver data
     |
     +---- Get GPIO descriptor
     |
     +---- Configure GPIO output
     |
     +---- Create sysfs attribute
     |
     v
LED Ready
LED ON Flow
echo 1 > state
       |
       v
state_store()
       |
       v
bbb_led_on()
       |
       v
gpiod_set_value_cansleep()
       |
       v
GPIO HIGH
       |
       v
LED ON
LED OFF Flow
echo 0 > state
       |
       v
state_store()
       |
       v
bbb_led_off()
       |
       v
gpiod_set_value_cansleep()
       |
       v
GPIO LOW
       |
       v
LED OFF
Debugging

Check driver:

lsmod | grep led_driver

Check kernel messages:

dmesg | tail -30

Check GPIO:

cat /sys/kernel/debug/gpio

If debugfs is mounted:

mount | grep debugfs
Remove Driver
sudo rmmod led_driver

The driver automatically sets the LED to OFF before removal.

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

Logs:

make logs

Test:

make test
Complete Architecture
              BeagleBone Black
                    |
                    v
              Device Tree
                    |
                    v
             led-gpios property
                    |
                    v
            Linux GPIO Framework
                    |
                    v
             LED Kernel Driver
                    |
                    v
           gpiod_set_value_cansleep()
                    |
                    v
               GPIO Controller
                    |
                    v
                 GPIO Pin
                    |
                    v
                   LED
Important Note

For production Linux systems, the preferred approach for a simple
status LED is often the Linux LED subsystem rather than exposing a
custom sysfs attribute.

The production architecture can be:

Device Tree
     |
     v
Linux LED Class
     |
     v
/sys/class/leds/
     |
     v
User Application

This example intentionally demonstrates the lower-level GPIO
descriptor API and platform-driver flow for learning and driver
development.


### Final structure

```text
beaglebone-black/
└── drivers/
    └── 12_led/
        ├── led_driver.c
        ├── led_driver.h
        ├── Makefile
        └── README.md

Build and load:

cd beaglebone-black/drivers/12_led
make
sudo insmod led_driver.ko
dmesg | tail -20

For your BeagleBone Black driver-learning repository, this gives you the GPIO flow: Device Tree → platform driver → GPIO descriptor → GPIO output → physical LED.
