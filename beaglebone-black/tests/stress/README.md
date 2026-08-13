Run
chmod +x adc_stress.sh
sudo ./adc_stress.sh

Custom duration and sampling interval:

sudo ./adc_stress.sh 300 10

This runs the ADC stress test for 300 seconds with a 10 ms sampling interval, checking IIO/ADC availability, channel discovery, continuous readings, invalid samples, read errors, sampling rate, and kernel ADC/IIO errors.


================================================================

Run
chmod +x gpio_stress.sh
sudo ./gpio_stress.sh

Custom GPIO, duration, and toggle count:

sudo ./gpio_stress.sh 60 120 100000

This performs continuous GPIO write → read-back → state verification, tracks read/write errors, calculates toggle rate, and captures GPIO/pinctrl kernel information.

========================================================================================

[200~Run
chmod +x i2c_stress.sh
sudo ./i2c_stress.sh

For a specific I²C bus/device:

sudo ./i2c_stress.sh 2 0x50 120 10000

Here:

2       → I2C bus /dev/i2c-2
0x50    → I2C slave address
120     → stress duration in seconds
10000   → maximum iterations

Note: The script uses register reads from 0x00 and 0x01, so those registers must be valid/readable on your actual I²C device. For an EEPROM, sensor, PMIC, or another specific device, the transaction section should be adapted to that device's register map.

===========================================================================

Run
chmod +x pwm_stress.sh
sudo ./pwm_stress.sh

For a specific PWM controller/channel:

sudo ./pwm_stress.sh 0 0 120

The test repeatedly changes PWM period → duty cycle → enable → read-back verification, then checks the final state and scans the kernel log for PWM/pinmux errors.

===========================================================================================

Run
chmod +x uart_stress.sh
sudo ./uart_stress.sh

For a specific UART:

sudo ./uart_stress.sh /dev/ttyS1 120 10000

Configuration:

/dev/ttyS1 → UART device
120        → stress duration in seconds
10000      → maximum iterations
115200     → default baud rate
8N1        → 8 data bits, no parity, 1 stop bit

The test performs UART TX → RX loopback → data verification → throughput measurement → error checking → kernel log analysis.



