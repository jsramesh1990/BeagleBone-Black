/*
 * BeagleBone Black - Button Input Driver
 *
 * File:
 *     button_input.c
 *
 * Purpose:
 *     GPIO push-button driver using the Linux Input subsystem.
 *
 * Features:
 *     - Device Tree GPIO configuration
 *     - GPIO input
 *     - Interrupt-based button detection
 *     - Input event reporting
 *     - Debounce support
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "button_input.h"


/* ------------------------------------------------------------------------- */
/* Private Driver Data                                                       */
/* ------------------------------------------------------------------------- */

struct button_input_device {
	struct device *dev;

	struct gpio_desc *button_gpio;

	struct input_dev *input;

	int irq;

	struct mutex lock;
};


/* ------------------------------------------------------------------------- */
/* Button IRQ Handler                                                        */
/* ------------------------------------------------------------------------- */

static irqreturn_t button_irq_handler(int irq,
				      void *data)
{
	struct button_input_device *button = data;

	int value;

	/*
	 * Read GPIO state.
	 */
	value = gpiod_get_value_cansleep(
			button->button_gpio);

	/*
	 * Report button state.
	 *
	 * BTN_0 becomes:
	 *     1 -> pressed
	 *     0 -> released
	 */
	input_report_key(button->input,
			 BTN_0,
			 value);

	input_sync(button->input);

	dev_dbg(button->dev,
		"Button event: %d\n",
		value);

	return IRQ_HANDLED;
}


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int button_input_probe(struct platform_device *pdev)
{
	struct button_input_device *button;

	int ret;

	dev_info(&pdev->dev,
		 "Probing button input driver\n");


	/*
	 * Allocate driver data.
	 */
	button = devm_kzalloc(&pdev->dev,
			      sizeof(*button),
			      GFP_KERNEL);

	if (!button)
		return -ENOMEM;


	button->dev = &pdev->dev;

	mutex_init(&button->lock);


	/*
	 * Get GPIO from Device Tree.
	 *
	 * Property:
	 *
	 *     button-gpios
	 */
	button->button_gpio =
		devm_gpiod_get(&pdev->dev,
			       "button",
			       GPIOD_IN);

	if (IS_ERR(button->button_gpio)) {

		ret = PTR_ERR(button->button_gpio);

		dev_err(&pdev->dev,
			"Failed to get button GPIO: %d\n",
			ret);

		return ret;
	}


	/*
	 * Configure GPIO debounce.
	 */
	ret = gpiod_set_debounce(
			button->button_gpio,
			BUTTON_DEBOUNCE_US);

	if (ret) {

		dev_warn(&pdev->dev,
			 "Hardware debounce unavailable: %d\n",
			 ret);
	}


	/*
	 * Get GPIO IRQ.
	 */
	button->irq =
		gpiod_to_irq(button->button_gpio);

	if (button->irq < 0) {

		dev_err(&pdev->dev,
			"Failed to get GPIO IRQ: %d\n",
			button->irq);

		return button->irq;
	}


	/*
	 * Allocate Linux input device.
	 */
	button->input =
		devm_input_allocate_device(
			&pdev->dev);

	if (!button->input)
		return -ENOMEM;


	button->input->name =
		BUTTON_INPUT_NAME;

	button->input->phys =
		"bbb/button0";

	button->input->id.bustype =
		BUS_HOST;


	/*
	 * Advertise button capability.
	 */
	input_set_capability(button->input,
			     EV_KEY,
			     BTN_0);


	/*
	 * Register input device.
	 */
	ret = input_register_device(
			button->input);

	if (ret) {

		dev_err(&pdev->dev,
			"Failed to register input device: %d\n",
			ret);

		return ret;
	}


	/*
	 * Request GPIO interrupt.
	 *
	 * Both rising and falling edges are used
	 * so that both press and release events
	 * are reported.
	 */
	ret = devm_request_threaded_irq(
			&pdev->dev,
			button->irq,
			NULL,
			button_irq_handler,
			IRQF_TRIGGER_RISING |
			IRQF_TRIGGER_FALLING |
			IRQF_ONESHOT,
			BUTTON_DRIVER_NAME,
			button);

	if (ret) {

		dev_err(&pdev->dev,
			"Failed to request IRQ: %d\n",
			ret);

		return ret;
	}


	platform_set_drvdata(pdev,
			     button);


	dev_info(&pdev->dev,
		 "Button input driver registered\n");

	dev_info(&pdev->dev,
		 "GPIO IRQ: %d\n",
		 button->irq);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void button_input_remove(
		struct platform_device *pdev)
{
	dev_info(&pdev->dev,
		 "Button input driver removed\n");
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                         */
/* ------------------------------------------------------------------------- */

static const struct of_device_id
button_input_of_match[] = {

	{
		.compatible = "bbb,button-input",
	},

	{ }
};

MODULE_DEVICE_TABLE(of,
		    button_input_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                           */
/* ------------------------------------------------------------------------- */

static struct platform_driver button_input_driver = {

	.probe = button_input_probe,

	.remove = button_input_remove,

	.driver = {
		.name = BUTTON_DRIVER_NAME,

		.of_match_table =
			button_input_of_match,
	},
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_platform_driver(button_input_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION(
	"BeagleBone Black GPIO Button Input Driver");
MODULE_VERSION("1.0");
