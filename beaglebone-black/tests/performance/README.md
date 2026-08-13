Run:

chmod +x adc_sampling.sh
sudo ./adc_sampling.sh

Or:

sudo ./adc_sampling.sh 0 1000 100

This measures successful ADC samples, total sampling time, actual samples/sec, min/max value, average value, and sample failures, with results saved under logs/performance/

=====================================================
Run:

chmod +x gpio_latency.sh
sudo ./gpio_latency.sh

Or:

sudo ./gpio_latency.sh 60 10000

Note: This measures userspace GPIO sysfs read latency, so it is useful for performance comparison but is not a precise hardware interrupt latency measurement. For true GPIO edge-to-handler latency, use a kernel-level timestamp/IRQ test or a GPIO character-device/libgpiod-based test.
====================================================================================
Run
chmod +x i2c_throughput.sh
sudo ./i2c_throughput.sh

Example with an EEPROM at 0x50:

sudo ./i2c_throughput.sh 2 0x50 0x00 100 16

The test reports:

I2C Bus
Slave Address
Transaction Count
Successful/Failed Transactions
Total Bytes
Elapsed Time
Transactions/sec
Average Transaction Latency
Effective Throughput (bytes/sec)
Effective Throughput (KB/sec)

Log output is saved under:

beaglebone-black/
└── logs/
    └── performance/
        └── i2c_throughput_YYYYMMDD_HHMMSS.log

Note: This measures effective Linux userspace I²C throughput using i2cget. It includes userspace, ioctl, driver, and transaction overhead, so it should not be treated as the raw physical I²C bus bit rate.
========================================================================================
Run
chmod +x pwm_stability.sh
sudo ./pwm_stability.sh

Example:

sudo ./pwm_stability.sh 0 0 20000000 10000000 30

This tests a 50 Hz PWM with 50% duty cycle for 30 seconds.

The script verifies:

PWM controller availability
PWM channel export
Period configuration
Duty-cycle configuration
PWM enable state
Configuration stability over time
Calculated frequency
Duty-cycle percentage
Final PWM state
Kernel PWM messages

Logs are stored in:

beaglebone-black/
└── logs/
    └── performance/
        └── pwm_stability_YYYYMMDD_HHMMSS.log

Important: Reading the sysfs PWM parameters verifies that the Linux PWM configuration remains unchanged; it does not directly measure physical waveform jitter. For actual frequency/duty/jitter measurement, use an oscilloscope or logic analyzer..
===================================================================================================
Run
chmod +x spi_throughput.sh
sudo ./spi_throughput.sh

Example:

sudo ./spi_throughput.sh /dev/spidev1.0 1000000 256 100

For a 5 MHz SPI clock:

sudo ./spi_throughput.sh /dev/spidev1.0 5000000 1024 100
Output measures
SPI Clock
SPI Mode
Bits per Word
Bytes per Transfer
Transfer Count
Successful/Failed Transfers
Total Bytes
Transfer Rate
Average Transaction Latency
Effective Throughput
Effective Mbit/sec
Theoretical SPI Data Rate

Log:

beaglebone-black/
└── logs/
    └── performance/
        └── spi_throughput_YYYYMMDD_HHMMSS.log

        Note: The measured throughput includes Linux userspace and spidev overhead, so it will normally be lower than the theoretical SPI clock rate. A real SPI slave or suitable loopback setup should be connected for meaningful transfer testing.

=================================================================================================
Run
chmod +x uart_throughput.sh
sudo ./uart_throughput.sh

Example:

sudo ./uart_throughput.sh /dev/ttyS1 115200 256 100

For a higher baud rate:

sudo ./uart_throughput.sh /dev/ttyS1 921600 1024 100

Hardware requirement: connect the selected UART's TX ↔ RX for loopback testing.

The script measures successful transfers, data integrity, total bytes, transfer rate, average latency, measured throughput, and theoretical 8N1 throughput, with logs saved under logs/performance/.

