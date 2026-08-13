Build
cd beaglebone-black/user-space/uart_test
make
Run
make config
make write
make read
make loopback

Or directly:

sudo ./uart_test /dev/ttyS1 config
sudo ./uart_test /dev/ttyS1 write
sudo ./uart_test /dev/ttyS1 read
sudo ./uart_test /dev/ttyS1 loopback

UART test flow:

uart_test.c
    │
    ▼
/dev/ttyS1
    │
    ▼
Linux TTY / Serial Driver
    │
    ▼
UART Controller
    │
    ├── TX ──────────► RX
    │
    └── RX ◄────────── TX
         Loopback

For the loopback test, connect UART TX → UART RX and ensure a common GND between the devices.
