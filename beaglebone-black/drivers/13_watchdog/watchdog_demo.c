/*
 * BeagleBone Black - Watchdog Demo
 *
 * File:
 *     watchdog_demo.c
 *
 * Purpose:
 *     Demonstrates Linux Watchdog Timer usage from a kernel module.
 *
 * Features:
 *     - Watchdog framework
 *     - /dev/watchdog interface
 *     - Start / stop watchdog
 *     - Set timeout
 *     - Ping watchdog
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/watchdog.h>
#include <linux/mutex.h>

#include "watchdog_demo.h"


/* ------------------------------------------------------------------------- */
/* Driver Data                                                               */
/* ------------------------------------------------------------------------- */

struct bbb_watchdog {
	struct device *dev;
	struct watchdog_device wdd;
	struct mutex lock;
};

static struct bbb_watchdog *watchdog_dev;


/* ------------------------------------------------------------------------- */
/* Watchdog Start                                                            */
/* ------------------------------------------------------------------------- */

static int bbb_watchdog_start(struct watchdog_device *wdd)
{
	struct bbb_watchdog *wd =
		watchdog_get_drvdata(wdd);

	mutex_lock(&wd->lock);

	dev_info(wd->dev,
		 "Watchdog started\n");

	mutex_unlock(&wd->lock);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Watchdog Stop                                                             */
/* ------------------------------------------------------------------------- */

static int bbb_watchdog_stop(struct watchdog_device *wdd)
{
	struct bbb_watchdog *wd =
		watchdog_get_drvdata(wdd);

	mutex_lock(&wd->lock);

	dev_info(wd->dev,
		 "Watchdog stopped\n");

	mutex_unlock(&wd->lock);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Watchdog Ping                                                             */
/* ------------------------------------------------------------------------- */

static int bbb_watchdog_ping(struct watchdog_device *wdd)
{
	struct bbb_watchdog *wd =
		watchdog_get_drvdata(wdd);

	mutex_lock(&wd->lock);

	dev_dbg(wd->dev,
		"Watchdog ping\n");

	mutex_unlock(&wd->lock);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Set Timeout                                                               */
/* ------------------------------------------------------------------------- */

static int bbb_watchdog_set_timeout(
		struct watchdog_device *wdd,
		unsigned int timeout)
{
	struct bbb_watchdog *wd =
		watchdog_get_drvdata(wdd);

	if (timeout < WATCHDOG_MIN_TIMEOUT ||
	    timeout > WATCHDOG_MAX_TIMEOUT)
		return -EINVAL;

	mutex_lock(&wd->lock);

	wdd->timeout = timeout;

	dev_info(wd->dev,
		 "Watchdog timeout set to %u seconds\n",
		 timeout);

	mutex_unlock(&wd->lock);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Watchdog Operations                                                       */
/* ------------------------------------------------------------------------- */

static const struct watchdog_ops bbb_watchdog_ops = {

	.owner = THIS_MODULE,

	.start =
		bbb_watchdog_start,

	.stop =
		bbb_watchdog_stop,

	.ping =
		bbb_watchdog_ping,

	.set_timeout =
		bbb_watchdog_set_timeout,
};


/* ------------------------------------------------------------------------- */
/* Watchdog Information                                                      */
/* ------------------------------------------------------------------------- */

static const struct watchdog_info bbb_watchdog_info = {

	.options =
		WDIOF_SETTIMEOUT |
		WDIOF_KEEPALIVEPING,

	.identity =
		WATCHDOG_DRIVER_NAME,

	.firmware_version =
		1,
};


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int bbb_watchdog_probe(
		struct platform_device *pdev)
{
	struct bbb_watchdog *wd;

	int ret;

	dev_info(&pdev->dev,
		 "Probing BBB watchdog demo\n");


	/*
	 * Allocate driver data.
	 */
	wd = devm_kzalloc(&pdev->dev,
			  sizeof(*wd),
			  GFP_KERNEL);

	if (!wd)
		return -ENOMEM;


	wd->dev = &pdev->dev;

	mutex_init(&wd->lock);


	/*
	 * Configure watchdog device.
	 */
	wd->wdd.info =
		&bbb_watchdog_info;

	wd->wdd.ops =
		&bbb_watchdog_ops;

	wd->wdd.min_timeout =
		WATCHDOG_MIN_TIMEOUT;

	wd->wdd.max_timeout =
		WATCHDOG_MAX_TIMEOUT;

	wd->wdd.timeout =
		WATCHDOG_DEFAULT_TIMEOUT;


	/*
	 * Store private driver data.
	 */
	watchdog_set_drvdata(&wd->wdd,
			     wd);

	platform_set_drvdata(pdev,
			     wd);


	/*
	 * Register watchdog with Linux
	 * watchdog subsystem.
	 */
	ret = devm_watchdog_register_device(
			&pdev->dev,
			&wd->wdd);

	if (ret) {

		dev_err(&pdev->dev,
			"Failed to register watchdog: %d\n",
			ret);

		return ret;
	}


	watchdog_dev = wd;


	dev_info(&pdev->dev,
		 "Watchdog registered\n");

	dev_info(&pdev->dev,
		 "Default timeout: %u seconds\n",
		 wd->wdd.timeout);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void bbb_watchdog_remove(
		struct platform_device *pdev)
{
	dev_info(&pdev->dev,
		 "BBB watchdog demo removed\n");

	watchdog_dev = NULL;
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                         */
/* ------------------------------------------------------------------------- */

static const struct of_device_id
bbb_watchdog_of_match[] = {

	{
		.compatible =
			"bbb,watchdog-demo",
	},

	{ }
};

MODULE_DEVICE_TABLE(of,
		    bbb_watchdog_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                           */
/* ------------------------------------------------------------------------- */

static struct platform_driver
bbb_watchdog_driver = {

	.probe =
		bbb_watchdog_probe,

	.remove =
		bbb_watchdog_remove,

	.driver = {
		.name =
			WATCHDOG_DRIVER_NAME,

		.of_match_table =
			bbb_watchdog_of_match,
	},
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_platform_driver(
		bbb_watchdog_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION(
	"BeagleBone Black Watchdog Demo");
MODULE_VERSION("1.0");
