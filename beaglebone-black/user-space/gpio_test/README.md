Build
cd beaglebone-black/user-space/gpio_test
make
Run manually
sudo ./gpio_test /dev/gpiochip0 20 input
sudo ./gpio_test /dev/gpiochip0 20 output
sudo ./gpio_test /dev/gpiochip0 20 toggle

Or use the Makefile shortcuts:

make input
make output
make toggle

If libgpiod is not installed on the BeagleBone Black, install the development package provided by your Linux distribution before building.
