/*
 * BeagleBone Black - RTC Driver
 *
 * File:
 *     rtc_driver.c
 *
 * Purpose:
 *     Example Linux kernel RTC driver using the
 *     Linux RTC framework.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/rtc.h>
#include <linux/mutex.h>
#include <linux/bcd.h>

#include "rtc_driver.h"


/* ------------------------------------------------------------------------- */
/* Private Driver Data                                                       */
/* ------------------------------------------------------------------------- */

struct bbb_rtc {
	void __iomem *base;
	resource_size_t size;

	struct rtc_device *rtc;
	struct mutex lock;
};


/* ------------------------------------------------------------------------- */
/* Register Access                                                           */
/* ------------------------------------------------------------------------- */

static inline u32 rtc_read_reg(struct bbb_rtc *rtc,
			       u32 offset)
{
	return readl(rtc->base + offset);
}


static inline void rtc_write_reg(struct bbb_rtc *rtc,
				 u32 offset,
				 u32 value)
{
	writel(value,
	       rtc->base + offset);
}


/* ------------------------------------------------------------------------- */
/* RTC Read Time                                                             */
/* ------------------------------------------------------------------------- */

static int bbb_rtc_read_time(struct device *dev,
			     struct rtc_time *tm)
{
	struct bbb_rtc *rtc =
		dev_get_drvdata(dev);

	u32 sec;
	u32 min;
	u32 hour;
	u32 day;
	u32 month;
	u32 year;

	mutex_lock(&rtc->lock);

	sec = rtc_read_reg(rtc,
			   RTC_SECONDS_REG);

	min = rtc_read_reg(rtc,
			  RTC_MINUTES_REG);

	hour = rtc_read_reg(rtc,
			    RTC_HOURS_REG);

	day = rtc_read_reg(rtc,
			  RTC_DAY_REG);

	month = rtc_read_reg(rtc,
			    RTC_MONTH_REG);

	year = rtc_read_reg(rtc,
			   RTC_YEAR_REG);

	mutex_unlock(&rtc->lock);

	tm->tm_sec  = sec & 0xFF;
	tm->tm_min  = min & 0xFF;
	tm->tm_hour = hour & 0xFF;
	tm->tm_mday = day & 0xFF;
	tm->tm_mon  = (month & 0xFF) - 1;
	tm->tm_year = (year & 0xFF) + 100;

	return 0;
}


/* ------------------------------------------------------------------------- */
/* RTC Set Time                                                              */
/* ------------------------------------------------------------------------- */

static int bbb_rtc_set_time(struct device *dev,
			    struct rtc_time *tm)
{
	struct bbb_rtc *rtc =
		dev_get_drvdata(dev);

	mutex_lock(&rtc->lock);

	rtc_write_reg(rtc,
		      RTC_SECONDS_REG,
		      tm->tm_sec);

	rtc_write_reg(rtc,
		      RTC_MINUTES_REG,
		      tm->tm_min);

	rtc_write_reg(rtc,
		      RTC_HOURS_REG,
		      tm->tm_hour);

	rtc_write_reg(rtc,
		      RTC_DAY_REG,
		      tm->tm_mday);

	rtc_write_reg(rtc,
		      RTC_MONTH_REG,
		      tm->tm_mon + 1);

	rtc_write_reg(rtc,
		      RTC_YEAR_REG,
		      tm->tm_year - 100);

	mutex_unlock(&rtc->lock);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* RTC Read Alarm                                                            */
/* ------------------------------------------------------------------------- */

static int bbb_rtc_read_alarm(struct device *dev,
			      struct rtc_wkalrm *alarm)
{
	struct bbb_rtc *rtc =
		dev_get_drvdata(dev);

	u32 sec;
	u32 min;
	u32 hour;

	mutex_lock(&rtc->lock);

	sec = rtc_read_reg(rtc,
			   RTC_ALARM_SECONDS_REG);

	min = rtc_read_reg(rtc,
			  RTC_ALARM_MINUTES_REG);

	hour = rtc_read_reg(rtc,
			    RTC_ALARM_HOURS_REG);

	mutex_unlock(&rtc->lock);

	alarm->time.tm_sec =
		sec & 0xFF;

	alarm->time.tm_min =
		min & 0xFF;

	alarm->time.tm_hour =
		hour & 0xFF;

	alarm->enabled =
		!!(rtc_read_reg(rtc,
				RTC_CONTROL_REG) &
		   RTC_ALARM_ENABLE);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* RTC Set Alarm                                                             */
/* ------------------------------------------------------------------------- */

static int bbb_rtc_set_alarm(struct device *dev,
			     struct rtc_wkalrm *alarm)
{
	struct bbb_rtc *rtc =
		dev_get_drvdata(dev);

	u32 control;

	mutex_lock(&rtc->lock);

	rtc_write_reg(rtc,
		      RTC_ALARM_SECONDS_REG,
		      alarm->time.tm_sec);

	rtc_write_reg(rtc,
		      RTC_ALARM_MINUTES_REG,
		      alarm->time.tm_min);

	rtc_write_reg(rtc,
		      RTC_ALARM_HOURS_REG,
		      alarm->time.tm_hour);

	control =
		rtc_read_reg(rtc,
			     RTC_CONTROL_REG);

	if (alarm->enabled)
		control |= RTC_ALARM_ENABLE;
	else
		control &= ~RTC_ALARM_ENABLE;

	rtc_write_reg(rtc,
		      RTC_CONTROL_REG,
		      control);

	mutex_unlock(&rtc->lock);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* RTC Alarm IRQ Enable                                                      */
/* ------------------------------------------------------------------------- */

static int bbb_rtc_alarm_irq_enable(struct device *dev,
				    unsigned int enabled)
{
	struct bbb_rtc *rtc =
		dev_get_drvdata(dev);

	u32 control;

	mutex_lock(&rtc->lock);

	control =
		rtc_read_reg(rtc,
			     RTC_CONTROL_REG);

	if (enabled)
		control |= RTC_ALARM_ENABLE;
	else
		control &= ~RTC_ALARM_ENABLE;

	rtc_write_reg(rtc,
		      RTC_CONTROL_REG,
		      control);

	mutex_unlock(&rtc->lock);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* RTC Operations                                                            */
/* ------------------------------------------------------------------------- */

static const struct rtc_class_ops bbb_rtc_ops = {
	.read_time = bbb_rtc_read_time,
	.set_time = bbb_rtc_set_time,

	.read_alarm = bbb_rtc_read_alarm,
	.set_alarm = bbb_rtc_set_alarm,

	.alarm_irq_enable =
		bbb_rtc_alarm_irq_enable,
};


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int bbb_rtc_probe(struct platform_device *pdev)
{
	struct bbb_rtc *rtc;
	struct resource *res;

	int ret;

	dev_info(&pdev->dev,
		 "Probing BBB RTC driver\n");

	/*
	 * Allocate driver data.
	 */
	rtc = devm_kzalloc(&pdev->dev,
			   sizeof(*rtc),
			   GFP_KERNEL);

	if (!rtc)
		return -ENOMEM;

	mutex_init(&rtc->lock);


	/*
	 * Get RTC memory resource.
	 */
	res = platform_get_resource(pdev,
				    IORESOURCE_MEM,
				    0);

	if (!res) {

		dev_err(&pdev->dev,
			"RTC memory resource not found\n");

		return -ENODEV;
	}

	rtc->size =
		resource_size(res);


	/*
	 * Map RTC registers.
	 */
	rtc->base =
		devm_ioremap_resource(&pdev->dev,
				      res);

	if (IS_ERR(rtc->base)) {

		ret = PTR_ERR(rtc->base);

		dev_err(&pdev->dev,
			"Failed to map RTC registers: %d\n",
			ret);

		return ret;
	}


	/*
	 * Allocate RTC device.
	 */
	rtc->rtc =
		devm_rtc_allocate_device(&pdev->dev);

	if (IS_ERR(rtc->rtc)) {

		ret = PTR_ERR(rtc->rtc);

		return ret;
	}


	rtc->rtc->ops =
		&bbb_rtc_ops;

	rtc->rtc->range_min =
		RTC_TIMESTAMP_MIN;

	rtc->rtc->range_max =
		RTC_TIMESTAMP_MAX;


	/*
	 * Store driver data.
	 */
	platform_set_drvdata(pdev,
			     rtc);

	dev_set_drvdata(&rtc->rtc->dev,
			rtc);


	/*
	 * Register RTC device.
	 */
	ret = devm_rtc_register_device(rtc->rtc);

	if (ret) {

		dev_err(&pdev->dev,
			"RTC registration failed: %d\n",
			ret);

		return ret;
	}


	dev_info(&pdev->dev,
		 "RTC driver registered as %s\n",
		 rtc->rtc->name);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void bbb_rtc_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev,
		 "RTC driver removed\n");
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                         */
/* ------------------------------------------------------------------------- */

static const struct of_device_id bbb_rtc_of_match[] = {
	{
		.compatible = "bbb,rtc-test",
	},
	{ }
};

MODULE_DEVICE_TABLE(of,
		    bbb_rtc_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                           */
/* ------------------------------------------------------------------------- */

static struct platform_driver bbb_rtc_driver = {
	.probe = bbb_rtc_probe,
	.remove = bbb_rtc_remove,

	.driver = {
		.name = RTC_DRIVER_NAME,
		.of_match_table =
			bbb_rtc_of_match,
	},
};


module_platform_driver(bbb_rtc_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black RTC Driver");
MODULE_VERSION("1.0");
