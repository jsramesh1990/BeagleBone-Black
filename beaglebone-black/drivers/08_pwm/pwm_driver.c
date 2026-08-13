/*
 * BeagleBone Black - PWM Driver
 *
 * File:
 *     pwm_driver.c
 *
 * Purpose:
 *     Example Linux kernel PWM driver using the Linux PWM framework.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/pwm.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/device.h>

#include "pwm_driver.h"


struct bbb_pwm_device {
	struct pwm_device *pwm;
	struct mutex lock;

	u64 period_ns;
	u64 duty_ns;
	bool enabled;
};


static ssize_t period_show(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
	struct bbb_pwm_device *pwmdev = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%llu\n",
			  pwmdev->period_ns);
}


static ssize_t period_store(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf,
			    size_t count)
{
	struct bbb_pwm_device *pwmdev =
		dev_get_drvdata(dev);

	u64 period;
	int ret;

	ret = kstrtou64(buf, 10, &period);
	if (ret)
		return ret;

	if (period < PWM_MIN_PERIOD_NS ||
	    period > PWM_MAX_PERIOD_NS)
		return -EINVAL;

	mutex_lock(&pwmdev->lock);

	pwmdev->period_ns = period;

	mutex_unlock(&pwmdev->lock);

	return count;
}


static ssize_t duty_cycle_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct bbb_pwm_device *pwmdev =
		dev_get_drvdata(dev);

	return sysfs_emit(buf, "%llu\n",
			  pwmdev->duty_ns);
}


static ssize_t duty_cycle_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	struct bbb_pwm_device *pwmdev =
		dev_get_drvdata(dev);

	u64 duty;
	int ret;

	ret = kstrtou64(buf, 10, &duty);
	if (ret)
		return ret;

	if (duty > PWM_MAX_DUTY_NS)
		return -EINVAL;

	mutex_lock(&pwmdev->lock);

	pwmdev->duty_ns = duty;

	mutex_unlock(&pwmdev->lock);

	return count;
}


static ssize_t enable_show(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
	struct bbb_pwm_device *pwmdev =
		dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n",
			  pwmdev->enabled);
}


static ssize_t enable_store(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf,
			    size_t count)
{
	struct bbb_pwm_device *pwmdev =
		dev_get_drvdata(dev);

	bool enable;
	int ret;

	ret = kstrtobool(buf, &enable);
	if (ret)
		return ret;

	mutex_lock(&pwmdev->lock);

	if (enable) {
		struct pwm_state state;

		pwm_get_state(pwmdev->pwm, &state);

		state.period = pwmdev->period_ns;
		state.duty_cycle = pwmdev->duty_ns;
		state.enabled = true;

		ret = pwm_apply_might_sleep(
			pwmdev->pwm,
			&state);

		if (!ret)
			pwmdev->enabled = true;
	} else {
		struct pwm_state state;

		pwm_get_state(pwmdev->pwm, &state);

		state.enabled = false;

		ret = pwm_apply_might_sleep(
			pwmdev->pwm,
			&state);

		if (!ret)
			pwmdev->enabled = false;
	}

	mutex_unlock(&pwmdev->lock);

	if (ret)
		return ret;

	return count;
}


static DEVICE_ATTR_RW(period);
static DEVICE_ATTR_RW(duty_cycle);
static DEVICE_ATTR_RW(enable);


static struct attribute *pwm_attrs[] = {
	&dev_attr_period.attr,
	&dev_attr_duty_cycle.attr,
	&dev_attr_enable.attr,
	NULL,
};


static const struct attribute_group pwm_attr_group = {
	.attrs = pwm_attrs,
};


static const struct attribute_group *pwm_attr_groups[] = {
	&pwm_attr_group,
	NULL,
};


static int pwm_probe(struct platform_device *pdev)
{
	struct bbb_pwm_device *pwmdev;
	struct pwm_state state;

	int ret;

	dev_info(&pdev->dev,
		 "Probing BBB PWM driver\n");

	pwmdev = devm_kzalloc(&pdev->dev,
			      sizeof(*pwmdev),
			      GFP_KERNEL);

	if (!pwmdev)
		return -ENOMEM;

	mutex_init(&pwmdev->lock);

	/*
	 * Get PWM device from the PWM framework.
	 */
	pwmdev->pwm =
		devm_pwm_get(&pdev->dev, NULL);

	if (IS_ERR(pwmdev->pwm)) {
		ret = PTR_ERR(pwmdev->pwm);

		dev_err(&pdev->dev,
			"Failed to get PWM: %d\n",
			ret);

		return ret;
	}

	/*
	 * Get current PWM state.
	 */
	pwm_get_state(pwmdev->pwm, &state);

	if (state.period)
		pwmdev->period_ns =
			state.period;
	else
		pwmdev->period_ns =
			PWM_DEFAULT_PERIOD_NS;

	pwmdev->duty_ns =
		pwmdev->period_ns *
		PWM_DEFAULT_DUTY_PERCENT / 100;

	pwmdev->enabled =
		state.enabled;

	platform_set_drvdata(pdev,
			     pwmdev);

	/*
	 * Apply default configuration.
	 */
	state.period =
		pwmdev->period_ns;

	state.duty_cycle =
		pwmdev->duty_ns;

	state.enabled = false;

	ret = pwm_apply_might_sleep(
		pwmdev->pwm,
		&state);

	if (ret) {
		dev_err(&pdev->dev,
			"Failed to configure PWM: %d\n",
			ret);

		return ret;
	}

	dev_info(&pdev->dev,
		 "PWM driver registered\n");

	dev_info(&pdev->dev,
		 "Period: %llu ns\n",
		 pwmdev->period_ns);

	dev_info(&pdev->dev,
		 "Duty cycle: %llu ns\n",
		 pwmdev->duty_ns);

	return 0;
}


static void pwm_remove(struct platform_device *pdev)
{
	struct bbb_pwm_device *pwmdev =
		platform_get_drvdata(pdev);

	struct pwm_state state;

	pwm_get_state(pwmdev->pwm,
		      &state);

	state.enabled = false;

	pwm_apply_might_sleep(
		pwmdev->pwm,
		&state);

	dev_info(&pdev->dev,
		 "PWM driver removed\n");
}


static const struct of_device_id pwm_of_match[] = {
	{
		.compatible = "bbb,pwm-test",
	},
	{ }
};

MODULE_DEVICE_TABLE(of,
		    pwm_of_match);


static struct platform_driver pwm_platform_driver = {
	.probe = pwm_probe,
	.remove = pwm_remove,

	.driver = {
		.name = PWM_DRIVER_NAME,
		.of_match_table = pwm_of_match,
		.dev_groups = pwm_attr_groups,
	},
};


module_platform_driver(pwm_platform_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black PWM Driver");
MODULE_VERSION("1.0");
