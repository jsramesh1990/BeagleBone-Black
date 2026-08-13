/*
 * BeagleBone Black - Debugfs Driver Demo
 *
 * File:
 *     debugfs_driver.c
 *
 * Purpose:
 *     Demonstrates creating a debugfs interface for debugging
 *     and monitoring a Linux kernel driver.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "debugfs_driver.h"


/* ------------------------------------------------------------------------- */
/* Private Data                                                              */
/* ------------------------------------------------------------------------- */

struct bbb_debugfs_data {
	struct mutex lock;

	u32 value;
	u32 counter;
	bool enable;

	char message[DEBUGFS_MESSAGE_SIZE];
};

static struct bbb_debugfs_data debugfs_data;


/* ------------------------------------------------------------------------- */
/* Debugfs Root                                                              */
/* ------------------------------------------------------------------------- */

static struct dentry *debugfs_root;


/* ------------------------------------------------------------------------- */
/* Debugfs Entries                                                           */
/* ------------------------------------------------------------------------- */

static struct dentry *debugfs_value;
static struct dentry *debugfs_counter;
static struct dentry *debugfs_enable;
static struct dentry *debugfs_message;
static struct dentry *debugfs_status;


/* ------------------------------------------------------------------------- */
/* value                                                                      */
/* ------------------------------------------------------------------------- */

static ssize_t debugfs_value_read(
		struct file *file,
		char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char output[DEBUGFS_OUTPUT_SIZE];
	int len;
	u32 value;

	if (*ppos != 0)
		return 0;

	mutex_lock(&debugfs_data.lock);

	value = debugfs_data.value;

	mutex_unlock(&debugfs_data.lock);

	len = scnprintf(
		output,
		sizeof(output),
		"%u\n",
		value);

	return simple_read_from_buffer(
		buffer,
		count,
		ppos,
		output,
		len);
}


static ssize_t debugfs_value_write(
		struct file *file,
		const char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char input[DEBUGFS_INPUT_SIZE];
	unsigned long value;
	int ret;

	if (count >= sizeof(input))
		return -EINVAL;

	if (copy_from_user(
			input,
			buffer,
			count))
		return -EFAULT;

	input[count] = '\0';

	ret = kstrtoul(
		input,
		10,
		&value);

	if (ret)
		return ret;

	mutex_lock(&debugfs_data.lock);

	debugfs_data.value = (u32)value;
	debugfs_data.counter++;

	mutex_unlock(&debugfs_data.lock);

	pr_debug(
		"BBB debugfs: value=%lu\n",
		value);

	return count;
}


/* ------------------------------------------------------------------------- */
/* counter                                                                    */
/* ------------------------------------------------------------------------- */

static ssize_t debugfs_counter_read(
		struct file *file,
		char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char output[DEBUGFS_OUTPUT_SIZE];
	int len;
	u32 counter;

	if (*ppos != 0)
		return 0;

	mutex_lock(&debugfs_data.lock);

	counter = debugfs_data.counter;

	mutex_unlock(&debugfs_data.lock);

	len = scnprintf(
		output,
		sizeof(output),
		"%u\n",
		counter);

	return simple_read_from_buffer(
		buffer,
		count,
		ppos,
		output,
		len);
}


/* ------------------------------------------------------------------------- */
/* enable                                                                     */
/* ------------------------------------------------------------------------- */

static ssize_t debugfs_enable_read(
		struct file *file,
		char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char output[DEBUGFS_OUTPUT_SIZE];
	int len;
	bool enable;

	if (*ppos != 0)
		return 0;

	mutex_lock(&debugfs_data.lock);

	enable = debugfs_data.enable;

	mutex_unlock(&debugfs_data.lock);

	len = scnprintf(
		output,
		sizeof(output),
		"%u\n",
		enable ? 1 : 0);

	return simple_read_from_buffer(
		buffer,
		count,
		ppos,
		output,
		len);
}


static ssize_t debugfs_enable_write(
		struct file *file,
		const char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char input[DEBUGFS_INPUT_SIZE];
	bool enable;
	int ret;

	if (count >= sizeof(input))
		return -EINVAL;

	if (copy_from_user(
			input,
			buffer,
			count))
		return -EFAULT;

	input[count] = '\0';

	ret = kstrtobool(
		input,
		&enable);

	if (ret)
		return ret;

	mutex_lock(&debugfs_data.lock);

	debugfs_data.enable = enable;

	mutex_unlock(&debugfs_data.lock);

	pr_debug(
		"BBB debugfs: enable=%d\n",
		enable ? 1 : 0);

	return count;
}


/* ------------------------------------------------------------------------- */
/* message                                                                    */
/* ------------------------------------------------------------------------- */

static ssize_t debugfs_message_read(
		struct file *file,
		char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char output[DEBUGFS_MESSAGE_SIZE];
	int len;

	if (*ppos != 0)
		return 0;

	mutex_lock(&debugfs_data.lock);

	len = scnprintf(
		output,
		sizeof(output),
		"%s\n",
		debugfs_data.message);

	mutex_unlock(&debugfs_data.lock);

	return simple_read_from_buffer(
		buffer,
		count,
		ppos,
		output,
		len);
}


static ssize_t debugfs_message_write(
		struct file *file,
		const char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	size_t len;

	if (count >= DEBUGFS_MESSAGE_SIZE)
		len = DEBUGFS_MESSAGE_SIZE - 1;
	else
		len = count;

	mutex_lock(&debugfs_data.lock);

	memset(
		debugfs_data.message,
		0,
		sizeof(debugfs_data.message));

	if (copy_from_user(
			debugfs_data.message,
			buffer,
			len)) {

		mutex_unlock(&debugfs_data.lock);

		return -EFAULT;
	}

	debugfs_data.message[len] = '\0';

	mutex_unlock(&debugfs_data.lock);

	pr_debug(
		"BBB debugfs: message updated\n");

	return count;
}


/* ------------------------------------------------------------------------- */
/* status                                                                     */
/* ------------------------------------------------------------------------- */

static ssize_t debugfs_status_read(
		struct file *file,
		char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char output[DEBUGFS_STATUS_SIZE];
	int len;
	u32 value;
	u32 counter;
	bool enable;
	char message[DEBUGFS_MESSAGE_SIZE];

	if (*ppos != 0)
		return 0;

	mutex_lock(&debugfs_data.lock);

	value = debugfs_data.value;
	counter = debugfs_data.counter;
	enable = debugfs_data.enable;

	strscpy(
		message,
		debugfs_data.message,
		sizeof(message));

	mutex_unlock(&debugfs_data.lock);

	len = scnprintf(
		output,
		sizeof(output),
		"BeagleBone Black Debugfs Driver\n"
		"-------------------------------\n"
		"Value   : %u\n"
		"Counter : %u\n"
		"Enable  : %u\n"
		"Message : %s\n",
		value,
		counter,
		enable ? 1 : 0,
		message);

	return simple_read_from_buffer(
		buffer,
		count,
		ppos,
		output,
		len);
}


/* ------------------------------------------------------------------------- */
/* File Operations                                                            */
/* ------------------------------------------------------------------------- */

static const struct file_operations debugfs_value_fops = {
	.owner = THIS_MODULE,
	.read = debugfs_value_read,
	.write = debugfs_value_write,
};


static const struct file_operations debugfs_counter_fops = {
	.owner = THIS_MODULE,
	.read = debugfs_counter_read,
};


static const struct file_operations debugfs_enable_fops = {
	.owner = THIS_MODULE,
	.read = debugfs_enable_read,
	.write = debugfs_enable_write,
};


static const struct file_operations debugfs_message_fops = {
	.owner = THIS_MODULE,
	.read = debugfs_message_read,
	.write = debugfs_message_write,
};


static const struct file_operations debugfs_status_fops = {
	.owner = THIS_MODULE,
	.read = debugfs_status_read,
};


/* ------------------------------------------------------------------------- */
/* Module Init                                                               */
/* ------------------------------------------------------------------------- */

static int __init bbb_debugfs_init(void)
{
	pr_info(
		"BBB debugfs: initializing\n");

	mutex_init(
		&debugfs_data.lock);

	debugfs_data.value =
		DEBUGFS_DEFAULT_VALUE;

	debugfs_data.counter =
		0;

	debugfs_data.enable =
		DEBUGFS_DEFAULT_ENABLE;

	strscpy(
		debugfs_data.message,
		DEBUGFS_DEFAULT_MESSAGE,
		sizeof(debugfs_data.message));


	/*
	 * Create:
	 *
	 * /sys/kernel/debug/bbb_debugfs
	 */
	debugfs_root = debugfs_create_dir(
		DEBUGFS_DIR_NAME,
		NULL);

	if (IS_ERR_OR_NULL(debugfs_root)) {

		pr_err(
			"BBB debugfs: failed to create directory\n");

		return -ENOMEM;
	}


	/*
	 * value
	 */
	debugfs_value = debugfs_create_file(
		DEBUGFS_VALUE_NAME,
		0644,
		debugfs_root,
		NULL,
		&debugfs_value_fops);

	if (!debugfs_value)
		goto error_cleanup;


	/*
	 * counter
	 */
	debugfs_counter = debugfs_create_file(
		DEBUGFS_COUNTER_NAME,
		0444,
		debugfs_root,
		NULL,
		&debugfs_counter_fops);

	if (!debugfs_counter)
		goto error_cleanup;


	/*
	 * enable
	 */
	debugfs_enable = debugfs_create_file(
		DEBUGFS_ENABLE_NAME,
		0644,
		debugfs_root,
		NULL,
		&debugfs_enable_fops);

	if (!debugfs_enable)
		goto error_cleanup;


	/*
	 * message
	 */
	debugfs_message = debugfs_create_file(
		DEBUGFS_MESSAGE_NAME,
		0644,
		debugfs_root,
		NULL,
		&debugfs_message_fops);

	if (!debugfs_message)
		goto error_cleanup;


	/*
	 * status
	 */
	debugfs_status = debugfs_create_file(
		DEBUGFS_STATUS_NAME,
		0444,
		debugfs_root,
		NULL,
		&debugfs_status_fops);

	if (!debugfs_status)
		goto error_cleanup;


	pr_info(
		"BBB debugfs: driver loaded\n");

	pr_info(
		"BBB debugfs: %s created\n",
		DEBUGFS_DIR_PATH);

	return 0;


error_cleanup:

	debugfs_remove_recursive(
		debugfs_root);

	debugfs_root = NULL;

	return -ENOMEM;
}


/* ------------------------------------------------------------------------- */
/* Module Exit                                                               */
/* ------------------------------------------------------------------------- */

static void __exit bbb_debugfs_exit(void)
{
	debugfs_remove_recursive(
		debugfs_root);

	debugfs_root = NULL;

	pr_info(
		"BBB debugfs: driver unloaded\n");
}


module_init(bbb_debugfs_init);
module_exit(bbb_debugfs_exit);


MODULE_LICENSE("GPL");

MODULE_AUTHOR(
	"Embedded Software Engineer");

MODULE_DESCRIPTION(
	"BeagleBone Black Debugfs Driver Demo");

MODULE_VERSION(
	DEBUGFS_DRIVER_VERSION);
