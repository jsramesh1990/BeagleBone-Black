/*
 * BeagleBone Black - LED Driver
 *
 * File:
 *     led_driver.c
 *
 * Purpose:
 *     GPIO LED driver using the Linux GPIO descriptor API.
 *
 * Features:
 *     - Device Tree GPIO configuration
 *     - LED ON/OFF control
 *     - sysfs interface
 *     - Platform driver
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/mutex.h>

#include "led_driver.h"


/* ------------------------------------------------------------------------- */
/* Driver Data                                                               */
/* ------------------------------------------------------------------------- */

struct bbb_led {
	struct device *dev;
	struct gpio_desc *gpio;
	struct mutex lock;
	bool state;
};


/* ------------------------------------------------------------------------- */
/* LED ON                                                                     */
/* ------------------------------------------------------------------------- */

static void bbb_led_on(struct bbb_led *led)
{
	mutex_lock(&led->lock);

	gpiod_set_value_cansleep(led->gpio, 1);

	led->state = true;

	mutex_unlock(&led->lock);

	dev_info(led->dev, "LED ON\n");
}


/* ------------------------------------------------------------------------- */
/* LED OFF                                                                    */
/* ------------------------------------------------------------------------- */

static void bbb_led_off(struct bbb_led *led)
{
	mutex_lock(&led->lock);

	gpiod_set_value_cansleep(led->gpio, 0);

	led->state = false;

	mutex_unlock(&led->lock);

	dev_info(led->dev, "LED OFF\n");
}


/* ------------------------------------------------------------------------- */
/* Sysfs State Attribute                                                      */
/* ------------------------------------------------------------------------- */

static ssize_t state_show(struct device *dev,
			  struct device_attribute *attr,
			  char *buf)
{
	struct bbb_led *led =
		dev_get_drvdata(dev);

	return sysfs_emit(buf,
			  "%d\n",
			  led->state);
}


static ssize_t state_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf,
			   size_t count)
{
	struct bbb_led *led =
		dev_get_drvdata(dev);

	long value;

	int ret;

	ret = kstrtol(buf, 10, &value);

	if (ret)
		return ret;

	switch (value) {

	case 0:
		bbb_led_off(led);
		break;

	case 1:
		bbb_led_on(led);
		break;

	default:
		dev_err(dev,
			"Invalid LED value: %ld\n",
			value);

		return -EINVAL;
	}

	return count;
}


static DEVICE_ATTR_RW(state);


/* ------------------------------------------------------------------------- */
/* Probe                                                                      */
/* ------------------------------------------------------------------------- */

static int bbb_led_probe(struct platform_device *pdev)
{
	struct bbb_led *led;

	int ret;

	dev_info(&pdev->dev,
		 "Probing BBB LED driver\n");


	/*
	 * Allocate driver data.
	 */
	led = devm_kzalloc(&pdev->dev,
			   sizeof(*led),
			   GFP_KERNEL);

	if (!led)
		return -ENOMEM;


	led->dev = &pdev->dev;

	mutex_init(&led->lock);


	/*
	 * Get LED GPIO from Device Tree.
	 *
	 * Property:
	 *
	 *     led-gpios
	 */
	led->gpio =
		devm_gpiod_get(&pdev->dev,
			       "led",
			       GPIOD_OUT_LOW);

	if (IS_ERR(led->gpio)) {

		ret = PTR_ERR(led->gpio);

		dev_err(&pdev->dev,
			"Failed to get LED GPIO: %d\n",
			ret);

		return ret;
	}


	/*
	 * Initial state = OFF.
	 */
	led->state = false;


	/*
	 * Store driver data.
	 */
	platform_set_drvdata(pdev,
			     led);


	/*
	 * Create sysfs attribute.
	 */
	ret = device_create_file(&pdev->dev,
				 &dev_attr_state);

	if (ret) {

		dev_err(&pdev->dev,
			"Failed to create state attribute: %d\n",
			ret);

		return ret;
	}


	dev_info(&pdev->dev,
		 "BBB LED driver registered\n");

	dev_info(&pdev->dev,
		 "LED control: /sys/bus/platform/devices/%s/state\n",
		 dev_name(&pdev->dev));

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                     */
/* ------------------------------------------------------------------------- */

static void bbb_led_remove(struct platform_device *pdev)
{
	struct bbb_led *led =
		platform_get_drvdata(pdev);

	/*
	 * Turn LED OFF before removing driver.
	 */
	bbb_led_off(led);

	device_remove_file(&pdev->dev,
			   &dev_attr_state);

	dev_info(&pdev->dev,
		 "BBB LED driver removed\n");
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                          */
/* ------------------------------------------------------------------------- */

static const struct of_device_id
bbb_led_of_match[] = {

	{
		.compatible = "bbb,gpio-led",
	},

	{ }
};

MODULE_DEVICE_TABLE(of,
		    bbb_led_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                            */
/* ------------------------------------------------------------------------- */

static struct platform_driver bbb_led_driver = {

	.probe = bbb_led_probe,

	.remove = bbb_led_remove,

	.driver = {
		.name = LED_DRIVER_NAME,

		.of_match_table =
			bbb_led_of_match,
	},
};


/* ------------------------------------------------------------------------- */
/* Module                                                                     */
/* ------------------------------------------------------------------------- */

module_platform_driver(bbb_led_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION(
	"BeagleBone Black GPIO LED Driver");
MODULE_VERSION("1.0");
