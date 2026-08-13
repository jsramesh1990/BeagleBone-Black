/*
 * BeagleBone Black - Character Device Driver
 *
 * File:
 *     char_driver.c
 *
 * Purpose:
 *     Basic Linux character device driver demonstrating:
 *       - Dynamic major/minor allocation
 *       - Character device registration
 *       - Device class creation
 *       - /dev node creation
 *       - open()
 *       - read()
 *       - write()
 *       - ioctl()
 *       - release()
 *
 * Device:
 *     /dev/bbb_char
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "char_driver.h"


/* ------------------------------------------------------------------------- */
/* Global Driver Data                                                        */
/* ------------------------------------------------------------------------- */

static dev_t bbb_dev;

static struct cdev bbb_cdev;

static struct class *bbb_class;

static struct device *bbb_device;


/*
 * Driver buffer used for read/write operations.
 */
static char driver_buffer[CHAR_DRIVER_BUFFER_SIZE];


/*
 * Protect buffer access from concurrent processes.
 */
static DEFINE_MUTEX(bbb_mutex);


/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

static int bbb_char_open(struct inode *inode,
                         struct file *file)
{
    pr_info("%s: device opened\n",
            CHAR_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Read                                                                      */
/* ------------------------------------------------------------------------- */

static ssize_t bbb_char_read(struct file *file,
                             char __user *buffer,
                             size_t count,
                             loff_t *offset)
{
    size_t data_length;
    size_t bytes_to_copy;

    /*
     * Prevent concurrent access.
     */
    if (mutex_lock_interruptible(&bbb_mutex)) {
        return -ERESTARTSYS;
    }

    data_length = strnlen(driver_buffer,
                          sizeof(driver_buffer));

    /*
     * End-of-file after the complete buffer
     * has been read.
     */
    if (*offset >= data_length) {

        mutex_unlock(&bbb_mutex);

        return 0;
    }

    bytes_to_copy = min(count,
                        data_length - (size_t)*offset);

    /*
     * Copy kernel buffer to user space.
     */
    if (copy_to_user(buffer,
                     driver_buffer + *offset,
                     bytes_to_copy) != 0) {

        mutex_unlock(&bbb_mutex);

        return -EFAULT;
    }

    *offset += bytes_to_copy;

    mutex_unlock(&bbb_mutex);

    pr_info("%s: read %zu bytes\n",
            CHAR_DRIVER_NAME,
            bytes_to_copy);

    return bytes_to_copy;
}


/* ------------------------------------------------------------------------- */
/* Write                                                                     */
/* ------------------------------------------------------------------------- */

static ssize_t bbb_char_write(struct file *file,
                              const char __user *buffer,
                              size_t count,
                              loff_t *offset)
{
    size_t bytes_to_copy;

    /*
     * Limit write size to driver buffer.
     */
    bytes_to_copy =
        min(count,
            (size_t)(CHAR_DRIVER_BUFFER_SIZE - 1));

    if (mutex_lock_interruptible(&bbb_mutex)) {
        return -ERESTARTSYS;
    }

    /*
     * Copy data from user space to kernel space.
     */
    if (copy_from_user(driver_buffer,
                       buffer,
                       bytes_to_copy) != 0) {

        mutex_unlock(&bbb_mutex);

        return -EFAULT;
    }

    /*
     * NULL terminate the string.
     */
    driver_buffer[bytes_to_copy] = '\0';

    mutex_unlock(&bbb_mutex);

    pr_info("%s: received %zu bytes\n",
            CHAR_DRIVER_NAME,
            bytes_to_copy);

    return bytes_to_copy;
}


/* ------------------------------------------------------------------------- */
/* IOCTL                                                                     */
/* ------------------------------------------------------------------------- */

static long bbb_char_ioctl(struct file *file,
                           unsigned int cmd,
                           unsigned long arg)
{
    int value;

    switch (cmd) {

    case CHAR_IOCTL_CLEAR:

        if (mutex_lock_interruptible(&bbb_mutex)) {
            return -ERESTARTSYS;
        }

        memset(driver_buffer,
               0,
               sizeof(driver_buffer));

        mutex_unlock(&bbb_mutex);

        pr_info("%s: buffer cleared\n",
                CHAR_DRIVER_NAME);

        break;


    case CHAR_IOCTL_GET_VALUE:

        value = CHAR_DRIVER_VERSION;

        if (copy_to_user((int __user *)arg,
                         &value,
                         sizeof(value)) != 0) {

            return -EFAULT;
        }

        pr_info("%s: version returned\n",
                CHAR_DRIVER_NAME);

        break;


    default:

        pr_warn("%s: unsupported ioctl 0x%x\n",
                CHAR_DRIVER_NAME,
                cmd);

        return -ENOTTY;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Release                                                                   */
/* ------------------------------------------------------------------------- */

static int bbb_char_release(struct inode *inode,
                            struct file *file)
{
    pr_info("%s: device closed\n",
            CHAR_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* File Operations                                                           */
/* ------------------------------------------------------------------------- */

static const struct file_operations bbb_fops = {
    .owner          = THIS_MODULE,
    .open           = bbb_char_open,
    .read           = bbb_char_read,
    .write          = bbb_char_write,
    .unlocked_ioctl = bbb_char_ioctl,
    .release        = bbb_char_release,
};


/* ------------------------------------------------------------------------- */
/* Module Initialization                                                     */
/* ------------------------------------------------------------------------- */

static int __init bbb_char_init(void)
{
    int ret;

    pr_info("%s: initializing driver\n",
            CHAR_DRIVER_NAME);

    /*
     * Dynamically allocate major/minor number.
     */
    ret = alloc_chrdev_region(&bbb_dev,
                              0,
                              1,
                              CHAR_DRIVER_NAME);

    if (ret < 0) {

        pr_err("%s: alloc_chrdev_region failed\n",
               CHAR_DRIVER_NAME);

        return ret;
    }

    pr_info("%s: major=%d minor=%d\n",
            CHAR_DRIVER_NAME,
            MAJOR(bbb_dev),
            MINOR(bbb_dev));

    /*
     * Initialize character device.
     */
    cdev_init(&bbb_cdev,
              &bbb_fops);

    bbb_cdev.owner = THIS_MODULE;

    /*
     * Add character device to kernel.
     */
    ret = cdev_add(&bbb_cdev,
                   bbb_dev,
                   1);

    if (ret < 0) {

        pr_err("%s: cdev_add failed\n",
               CHAR_DRIVER_NAME);

        unregister_chrdev_region(bbb_dev,
                                 1);

        return ret;
    }

    /*
     * Create device class.
     */
    bbb_class = class_create(CHAR_DRIVER_CLASS);

    if (IS_ERR(bbb_class)) {

        ret = PTR_ERR(bbb_class);

        pr_err("%s: class_create failed\n",
               CHAR_DRIVER_NAME);

        cdev_del(&bbb_cdev);

        unregister_chrdev_region(bbb_dev,
                                 1);

        return ret;
    }

    /*
     * Create /dev/bbb_char.
     */
    bbb_device = device_create(bbb_class,
                                NULL,
                                bbb_dev,
                                NULL,
                                CHAR_DRIVER_DEVICE);

    if (IS_ERR(bbb_device)) {

        ret = PTR_ERR(bbb_device);

        pr_err("%s: device_create failed\n",
               CHAR_DRIVER_NAME);

        class_destroy(bbb_class);

        cdev_del(&bbb_cdev);

        unregister_chrdev_region(bbb_dev,
                                 1);

        return ret;
    }

    /*
     * Initialize driver buffer.
     */
    memset(driver_buffer,
           0,
           sizeof(driver_buffer));

    strncpy(driver_buffer,
            CHAR_DRIVER_DEFAULT_MESSAGE,
            sizeof(driver_buffer) - 1);

    mutex_init(&bbb_mutex);

    pr_info("%s: driver loaded successfully\n",
            CHAR_DRIVER_NAME);

    pr_info("%s: device /dev/%s\n",
            CHAR_DRIVER_NAME,
            CHAR_DRIVER_DEVICE);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Module Exit                                                               */
/* ------------------------------------------------------------------------- */

static void __exit bbb_char_exit(void)
{
    pr_info("%s: removing driver\n",
            CHAR_DRIVER_NAME);

    /*
     * Remove device node.
     */
    device_destroy(bbb_class,
                   bbb_dev);

    /*
     * Remove class.
     */
    class_destroy(bbb_class);

    /*
     * Remove character device.
     */
    cdev_del(&bbb_cdev);

    /*
     * Release major/minor number.
     */
    unregister_chrdev_region(bbb_dev,
                             1);

    pr_info("%s: driver unloaded\n",
            CHAR_DRIVER_NAME);
}


/* ------------------------------------------------------------------------- */
/* Module Information                                                        */
/* ------------------------------------------------------------------------- */

module_init(bbb_char_init);

module_exit(bbb_char_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black Character Device Driver");
MODULE_VERSION("1.0");
