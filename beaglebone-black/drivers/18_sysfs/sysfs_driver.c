/*
 * BeagleBone Black - Sysfs Driver Demo
 *
 * File:
 *     sysfs_driver.c
 *
 * Purpose:
 *     Demonstrates creating a sysfs interface from a Linux
 *     platform driver.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/mutex.h>

#include "sysfs_driver.h"


/* ------------------------------------------------------------------------- */
/* Private Data                                                              */
/* ------------------------------------------------------------------------- */

struct bbb_sysfs_priv {
	struct device *dev;
	struct mutex lock;

	int value;
	bool enable;
};


/* ------------------------------------------------------------------------- */
/* value - Show                                                              */
/* ------------------------------------------------------------------------- */

static ssize_t value_show(
		struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct bbb_sysfs_priv *priv;

	priv = dev_get_drvdata(dev);

	mutex_lock(&priv->lock);

	snprintf(buf, PAGE_SIZE, "%d\n", priv->value);

	mutex_unlock(&priv->lock);

	return strlen(buf);
}


/* ------------------------------------------------------------------------- */
/* value - Store                                                             */
/* ------------------------------------------------------------------------- */

static ssize_t value_store(
		struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t count)
{
	struct bbb_sysfs_priv *priv;
	int value;
	int ret;

	priv = dev_get_drvdata(dev);

	ret = kstrtoint(buf, 10, &value);

	if (ret)
		return ret;

	mutex_lock(&priv->lock);

	priv->value = value;

	mutex_unlock(&priv->lock);

	dev_info(dev,
		 "sysfs value updated: %d\n",
		 value);

	return count;
}


/* ------------------------------------------------------------------------- */
/* enable - Show                                                             */
/* ------------------------------------------------------------------------- */

static ssize_t enable_show(
		struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct bbb_sysfs_priv *priv;
	bool enabled;

	priv = dev_get_drvdata(dev);

	mutex_lock(&priv->lock);

	enabled = priv->enable;

	mutex_unlock(&priv->lock);

	return scnprintf(
		buf,
		PAGE_SIZE,
		"%d\n",
		enabled ? 1 : 0);
}


/* ------------------------------------------------------------------------- */
/* enable - Store                                                            */
/* ------------------------------------------------------------------------- */

static ssize_t enable_store(
		struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t count)
{
	struct bbb_sysfs_priv *priv;
	bool enable;
	int ret;

	priv = dev_get_drvdata(dev);

	ret = kstrtobool(buf, &enable);

	if (ret)
		return ret;

	mutex_lock(&priv->lock);

	priv->enable = enable;

	mutex_unlock(&priv->lock);

	dev_info(dev,
		 "sysfs enable: %s\n",
		 enable ? "enabled" : "disabled");

	return count;
}


/* ------------------------------------------------------------------------- */
/* status - Show                                                             */
/* ------------------------------------------------------------------------- */

static ssize_t status_show(
		struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct bbb_sysfs_priv *priv;
	int value;
	bool enable;

	priv = dev_get_drvdata(dev);

	mutex_lock(&priv->lock);

	value = priv->value;
	enable = priv->enable;

	mutex_unlock(&priv->lock);

	return scnprintf(
		buf,
		PAGE_SIZE,
		"value=%d enable=%d\n",
		value,
		enable ? 1 : 0);
}


/* ------------------------------------------------------------------------- */
/* Sysfs Attributes                                                          */
/* ------------------------------------------------------------------------- */

static DEVICE_ATTR_RW(value);

static DEVICE_ATTR_RW(enable);

static DEVICE_ATTR_RO(status);


/* ------------------------------------------------------------------------- */
/* Attribute Group                                                           */
/* ------------------------------------------------------------------------- */

static struct attribute *bbb_sysfs_attrs[] = {
	&dev_attr_value.attr,
	&dev_attr_enable.attr,
	&dev_attr_status.attr,
	NULL,
};


static const struct attribute_group bbb_sysfs_group = {
	.attrs = bbb_sysfs_attrs,
};


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int bbb_sysfs_probe(
		struct platform_device *pdev)
{
	struct bbb_sysfs_priv *priv;
	int ret;

	dev_info(
		&pdev->dev,
		"BBB sysfs driver probe\n");

	priv = devm_kzalloc(
		&pdev->dev,
		sizeof(*priv),
		GFP_KERNEL);

	if (!priv)
		return -ENOMEM;

	priv->dev = &pdev->dev;

	priv->value = SYSFS_DEFAULT_VALUE;
	priv->enable = false;

	mutex_init(&priv->lock);

	platform_set_drvdata(
		pdev,
		priv);

	ret = sysfs_create_group(
		&pdev->dev.kobj,
		&bbb_sysfs_group);

	if (ret) {

		dev_err(
			&pdev->dev,
			"Failed to create sysfs group: %d\n",
			ret);

		return ret;
	}

	dev_info(
		&pdev->dev,
		"Sysfs attributes created\n");

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void bbb_sysfs_remove(
		struct platform_device *pdev)
{
	sysfs_remove_group(
		&pdev->dev.kobj,
		&bbb_sysfs_group);

	dev_info(
		&pdev->dev,
		"Sysfs attributes removed\n");
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                         */
/* ------------------------------------------------------------------------- */

static const struct of_device_id bbb_sysfs_of_match[] = {
	{
		.compatible = "bbb,sysfs-demo",
	},
	{ }
};

MODULE_DEVICE_TABLE(
	of,
	bbb_sysfs_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                           */
/* ------------------------------------------------------------------------- */

static struct platform_driver bbb_sysfs_driver = {

	.probe = bbb_sysfs_probe,

	.remove = bbb_sysfs_remove,

	.driver = {
		.name = SYSFS_DRIVER_NAME,
		.of_match_table = bbb_sysfs_of_match,
	},
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_platform_driver(bbb_sysfs_driver);


MODULE_LICENSE("GPL");

MODULE_AUTHOR(
	"Embedded Software Engineer");

MODULE_DESCRIPTION(
	"BeagleBone Black Sysfs Driver Demo");

MODULE_VERSION(
	SYSFS_DRIVER_VERSION);
