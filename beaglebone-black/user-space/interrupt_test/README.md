Build
cd beaglebone-black/user-space/interrupt_test
make
Run
make rising
make falling
make both

Or manually:

sudo ./interrupt_test /dev/gpiochip0 20 rising
sudo ./interrupt_test /dev/gpiochip0 20 falling
sudo ./interrupt_test /dev/gpiochip0 20 both
Directory
interrupt_test/
├── interrupt_test.c
├── interrupt_test.h
├── Makefile
└── README.md

Build dependency: libgpiod development headers/library must be installed on the target system.
