/*
 * BeagleBone Black - Procfs Driver Demo
 *
 * File:
 *     proc_driver.c
 *
 * Purpose:
 *     Demonstrates creating a /proc interface from a Linux
 *     kernel module.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/seq_file.h>

#include "proc_driver.h"


/* ------------------------------------------------------------------------- */
/* Private Data                                                              */
/* ------------------------------------------------------------------------- */

struct bbb_proc_data {
	struct mutex lock;

	int value;
	bool enable;

	char message[PROC_MESSAGE_SIZE];
};

static struct bbb_proc_data proc_data;

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_value;
static struct proc_dir_entry *proc_enable;
static struct proc_dir_entry *proc_status;
static struct proc_dir_entry *proc_message;


/* ------------------------------------------------------------------------- */
/* /proc/bbb_proc/value - Read                                               */
/* ------------------------------------------------------------------------- */

static ssize_t proc_value_read(
		struct file *file,
		char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char output[PROC_OUTPUT_SIZE];
	int len;
	int value;

	if (*ppos != 0)
		return 0;

	mutex_lock(&proc_data.lock);

	value = proc_data.value;

	mutex_unlock(&proc_data.lock);

	len = scnprintf(
		output,
		sizeof(output),
		"%d\n",
		value);

	if (copy_to_user(
			buffer,
			output,
			len))
		return -EFAULT;

	*ppos += len;

	return len;
}


/* ------------------------------------------------------------------------- */
/* /proc/bbb_proc/value - Write                                              */
/* ------------------------------------------------------------------------- */

static ssize_t proc_value_write(
		struct file *file,
		const char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char input[PROC_INPUT_SIZE];
	int value;
	int ret;

	if (count >= sizeof(input))
		return -EINVAL;

	if (copy_from_user(
			input,
			buffer,
			count))
		return -EFAULT;

	input[count] = '\0';

	ret = kstrtoint(
		input,
		10,
		&value);

	if (ret)
		return ret;

	mutex_lock(&proc_data.lock);

	proc_data.value = value;

	mutex_unlock(&proc_data.lock);

	pr_info(
		"BBB procfs: value updated to %d\n",
		value);

	return count;
}


/* ------------------------------------------------------------------------- */
/* /proc/bbb_proc/enable - Read                                              */
/* ------------------------------------------------------------------------- */

static ssize_t proc_enable_read(
		struct file *file,
		char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char output[PROC_OUTPUT_SIZE];
	int len;
	bool enable;

	if (*ppos != 0)
		return 0;

	mutex_lock(&proc_data.lock);

	enable = proc_data.enable;

	mutex_unlock(&proc_data.lock);

	len = scnprintf(
		output,
		sizeof(output),
		"%d\n",
		enable ? 1 : 0);

	if (copy_to_user(
			buffer,
			output,
			len))
		return -EFAULT;

	*ppos += len;

	return len;
}


/* ------------------------------------------------------------------------- */
/* /proc/bbb_proc/enable - Write                                             */
/* ------------------------------------------------------------------------- */

static ssize_t proc_enable_write(
		struct file *file,
		const char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char input[PROC_INPUT_SIZE];
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

	mutex_lock(&proc_data.lock);

	proc_data.enable = enable;

	mutex_unlock(&proc_data.lock);

	pr_info(
		"BBB procfs: enable = %d\n",
		enable ? 1 : 0);

	return count;
}


/* ------------------------------------------------------------------------- */
/* /proc/bbb_proc/message - Read                                             */
/* ------------------------------------------------------------------------- */

static ssize_t proc_message_read(
		struct file *file,
		char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	char output[PROC_MESSAGE_SIZE];
	int len;

	if (*ppos != 0)
		return 0;

	mutex_lock(&proc_data.lock);

	len = scnprintf(
		output,
		sizeof(output),
		"%s\n",
		proc_data.message);

	mutex_unlock(&proc_data.lock);

	if (copy_to_user(
			buffer,
			output,
			len))
		return -EFAULT;

	*ppos += len;

	return len;
}


/* ------------------------------------------------------------------------- */
/* /proc/bbb_proc/message - Write                                            */
/* ------------------------------------------------------------------------- */

static ssize_t proc_message_write(
		struct file *file,
		const char __user *buffer,
		size_t count,
		loff_t *ppos)
{
	size_t len;

	if (count >= PROC_MESSAGE_SIZE)
		len = PROC_MESSAGE_SIZE - 1;
	else
		len = count;

	mutex_lock(&proc_data.lock);

	memset(
		proc_data.message,
		0,
		sizeof(proc_data.message));

	if (copy_from_user(
			proc_data.message,
			buffer,
			len)) {

		mutex_unlock(&proc_data.lock);

		return -EFAULT;
	}

	proc_data.message[len] = '\0';

	mutex_unlock(&proc_data.lock);

	pr_info(
		"BBB procfs: message updated\n");

	return count;
}


/* ------------------------------------------------------------------------- */
/* Status - seq_file                                                         */
/* ------------------------------------------------------------------------- */

static int proc_status_show(
		struct seq_file *seq,
		void *v)
{
	int value;
	bool enable;
	char message[PROC_MESSAGE_SIZE];

	mutex_lock(&proc_data.lock);

	value = proc_data.value;
	enable = proc_data.enable;

	strscpy(
		message,
		proc_data.message,
		sizeof(message));

	mutex_unlock(&proc_data.lock);

	seq_printf(
		seq,
		"BeagleBone Black Procfs Driver\n");

	seq_printf(
		seq,
		"-----------------------------\n");

	seq_printf(
		seq,
		"Value   : %d\n",
		value);

	seq_printf(
		seq,
		"Enable  : %d\n",
		enable ? 1 : 0);

	seq_printf(
		seq,
		"Message : %s\n",
		message);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Status Open                                                               */
/* ------------------------------------------------------------------------- */

static int proc_status_open(
		struct inode *inode,
		struct file *file)
{
	return single_open(
		file,
		proc_status_show,
		NULL);
}


/* ------------------------------------------------------------------------- */
/* Proc File Operations                                                      */
/* ------------------------------------------------------------------------- */

static const struct proc_ops proc_value_ops = {

	.proc_read =
		proc_value_read,

	.proc_write =
		proc_value_write,
};


static const struct proc_ops proc_enable_ops = {

	.proc_read =
		proc_enable_read,

	.proc_write =
		proc_enable_write,
};


static const struct proc_ops proc_message_ops = {

	.proc_read =
		proc_message_read,

	.proc_write =
		proc_message_write,
};


static const struct proc_ops proc_status_ops = {

	.proc_open =
		proc_status_open,

	.proc_read =
		seq_read,

	.proc_lseek =
		seq_lseek,

	.proc_release =
		single_release,
};


/* ------------------------------------------------------------------------- */
/* Module Init                                                               */
/* ------------------------------------------------------------------------- */

static int __init bbb_proc_init(void)
{
	pr_info(
		"BBB procfs: initializing driver\n");

	mutex_init(
		&proc_data.lock);

	proc_data.value =
		PROC_DEFAULT_VALUE;

	proc_data.enable =
		PROC_DEFAULT_ENABLE;

	strscpy(
		proc_data.message,
		PROC_DEFAULT_MESSAGE,
		sizeof(proc_data.message));


	/*
	 * Create:
	 *
	 * /proc/bbb_proc
	 */
	proc_dir = proc_mkdir(
		PROC_DIR_NAME,
		NULL);

	if (!proc_dir) {

		pr_err(
			"BBB procfs: failed to create /proc/%s\n",
			PROC_DIR_NAME);

		return -ENOMEM;
	}


	/*
	 * /proc/bbb_proc/value
	 */
	proc_value = proc_create(
		PROC_VALUE_NAME,
		0666,
		proc_dir,
		&proc_value_ops);

	if (!proc_value)
		goto error_cleanup;


	/*
	 * /proc/bbb_proc/enable
	 */
	proc_enable = proc_create(
		PROC_ENABLE_NAME,
		0666,
		proc_dir,
		&proc_enable_ops);

	if (!proc_enable)
		goto error_cleanup;


	/*
	 * /proc/bbb_proc/status
	 */
	proc_status = proc_create(
		PROC_STATUS_NAME,
		0444,
		proc_dir,
		&proc_status_ops);

	if (!proc_status)
		goto error_cleanup;


	/*
	 * /proc/bbb_proc/message
	 */
	proc_message = proc_create(
		PROC_MESSAGE_NAME,
		0666,
		proc_dir,
		&proc_message_ops);

	if (!proc_message)
		goto error_cleanup;


	pr_info(
		"BBB procfs: driver loaded\n");

	pr_info(
		"BBB procfs: /proc/%s created\n",
		PROC_DIR_NAME);

	return 0;


error_cleanup:

	if (proc_message)
		proc_remove(proc_message);

	if (proc_status)
		proc_remove(proc_status);

	if (proc_enable)
		proc_remove(proc_enable);

	if (proc_value)
		proc_remove(proc_value);

	if (proc_dir)
		proc_remove(proc_dir);

	return -ENOMEM;
}


/* ------------------------------------------------------------------------- */
/* Module Exit                                                               */
/* ------------------------------------------------------------------------- */

static void __exit bbb_proc_exit(void)
{
	/*
	 * Removing the directory recursively removes
	 * all proc entries created below it.
	 */
	if (proc_dir)
		proc_remove(proc_dir);

	pr_info(
		"BBB procfs: driver unloaded\n");
}


module_init(bbb_proc_init);
module_exit(bbb_proc_exit);


MODULE_LICENSE("GPL");

MODULE_AUTHOR(
	"Embedded Software Engineer");

MODULE_DESCRIPTION(
	"BeagleBone Black Procfs Driver Demo");

MODULE_VERSION(
	PROC_DRIVER_VERSION);
