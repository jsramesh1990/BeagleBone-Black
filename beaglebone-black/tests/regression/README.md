Run it with:

chmod +x device_tree_regression.sh
sudo ./device_tree_regression.sh

The script checks Device Tree availability, board model, compatible strings, CPU, memory, interrupt controller, GPIO, I²C, SPI, UART, PWM, CAN nodes, node status, overlays, and kernel Device Tree messages, then stores the regression result under logs/regression/.


=======================================================================================

Run
chmod +x run_regression.sh
sudo ./run_regression.sh

Or stop immediately when one regression test fails:

sudo ./run_regression.sh --stop-on-fail

Your regression structure is now:

beaglebone-black/
└── tests/
    └── regression/
        ├── device_tree_regression.sh
        ├── driver_regression.sh
        └── run_regression.sh

The runner executes the Device Tree regression and Driver regression, collects system/peripheral information, scans kernel logs for errors, and generates a combined report under:

beaglebone-black/logs/regression/

