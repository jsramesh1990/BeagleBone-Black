Make it executable:

chmod +x beaglebone-black/tests/functional/test_uart.sh
Usage

Detect UART devices:

sudo ./tests/functional/test_uart.sh

Test a UART at 115200 baud:

sudo ./tests/functional/test_uart.sh /dev/ttyS1 115200

Run a physical TX ↔ RX loopback test:

sudo ./tests/functional/test_uart.sh /dev/ttyS1 115200 loopback
UART functional-test flow
                  test_uart.sh
                       |
                       v
                 UART subsystem
                       |
                       v
                  /dev/ttyS*
                       |
                       v
                 UART config
                       |
              +--------+--------+
              |                 |
              v                 v
             TX                RX
              |                 |
              +------<----------+
                    Loopback
                       |
                       v
                 Device Tree
                       |
                       v
                 Kernel dmesg
                       |
                       v
                PASS / FAIL

Log location:

beaglebone-black/
└── logs/
    └── functional/
        └── uart_YYYYMMDD_HHMMSS.log

Hardware note: for the loopback test, connect the selected UART's TX to RX and GND to GND using the correct 3.3-V UART levels. Do not connect the BeagleBone Black UART directly to RS-232 voltage levels.
