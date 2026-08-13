/*
 * BeagleBone Black - I2C EEPROM Driver
 *
 * File:
 *     i2c_eeprom.c
 *
 * Purpose:
 *     Example I2C EEPROM client driver.
 *
 * Device:
 *     /dev/bbb_i2c_eeprom
 *
 * The EEPROM is accessed through the Linux I2C framework.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "i2c_eeprom.h"


/* ------------------------------------------------------------------------- */
/* Driver Data                                                               */
/* ------------------------------------------------------------------------- */

static struct i2c_client *eeprom_client;

static dev_t eeprom_dev;

static struct cdev eeprom_cdev;

static struct class *eeprom_class;

static struct device *eeprom_device;

static DEFINE_MUTEX(eeprom_mutex);


/* ------------------------------------------------------------------------- */
/* EEPROM Read                                                               */
/* ------------------------------------------------------------------------- */

static int eeprom_read_data(u16 address,
                            u8 *buffer,
                            size_t length)
{
    u8 address_buffer[2];

    struct i2c_msg messages[2];

    int ret;

    /*
     * 16-bit EEPROM memory address.
     */
    address_buffer[0] =
        (address >> 8) & 0xFF;

    address_buffer[1] =
        address & 0xFF;

    messages[0].addr = eeprom_client->addr;
    messages[0].flags = 0;
    messages[0].len = 2;
    messages[0].buf = address_buffer;

    messages[1].addr = eeprom_client->addr;
    messages[1].flags = I2C_M_RD;
    messages[1].len = length;
    messages[1].buf = buffer;

    ret = i2c_transfer(eeprom_client->adapter,
                       messages,
                       2);

    if (ret < 0) {
        return ret;
    }

    if (ret != 2) {
        return -EIO;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* EEPROM Write                                                              */
/* ------------------------------------------------------------------------- */

static int eeprom_write_data(u16 address,
                             const u8 *buffer,
                             size_t length)
{
    u8 *write_buffer;

    int ret;

    /*
     * EEPROM write packet:
     *
     *     Address MSB
     *     Address LSB
     *     Data...
     */
    write_buffer =
        kmalloc(length + 2, GFP_KERNEL);

    if (!write_buffer) {
        return -ENOMEM;
    }

    write_buffer[0] =
        (address >> 8) & 0xFF;

    write_buffer[1] =
        address & 0xFF;

    memcpy(&write_buffer[2],
           buffer,
           length);

    ret = i2c_master_send(eeprom_client,
                          write_buffer,
                          length + 2);

    kfree(write_buffer);

    if (ret < 0) {
        return ret;
    }

    if (ret != length + 2) {
        return -EIO;
    }

    /*
     * Give EEPROM time to complete internal write cycle.
     */
    msleep(10);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

static int eeprom_open(struct inode *inode,
                       struct file *file)
{
    pr_info("%s: opened\n",
            EEPROM_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Read                                                                      */
/* ------------------------------------------------------------------------- */

static ssize_t eeprom_read(struct file *file,
                           char __user *buffer,
                           size_t count,
                           loff_t *offset)
{
    u8 data[EEPROM_MAX_TRANSFER];

    size_t length;

    int ret;

    if (*offset >= EEPROM_SIZE) {
        return 0;
    }

    length = min_t(size_t,
                   count,
                   EEPROM_MAX_TRANSFER);

    if (*offset + length > EEPROM_SIZE) {
        length = EEPROM_SIZE - *offset;
    }

    mutex_lock(&eeprom_mutex);

    ret = eeprom_read_data((u16)*offset,
                           data,
                           length);

    if (ret) {
        mutex_unlock(&eeprom_mutex);
        return ret;
    }

    if (copy_to_user(buffer,
                     data,
                     length)) {

        mutex_unlock(&eeprom_mutex);

        return -EFAULT;
    }

    *offset += length;

    mutex_unlock(&eeprom_mutex);

    return length;
}


/* ------------------------------------------------------------------------- */
/* Write                                                                     */
/* ------------------------------------------------------------------------- */

static ssize_t eeprom_write(struct file *file,
                            const char __user *buffer,
                            size_t count,
                            loff_t *offset)
{
    u8 data[EEPROM_MAX_TRANSFER];

    size_t length;

    int ret;

    if (*offset >= EEPROM_SIZE) {
        return -ENOSPC;
    }

    length = min_t(size_t,
                   count,
                   EEPROM_MAX_TRANSFER);

    if (*offset + length > EEPROM_SIZE) {
        length = EEPROM_SIZE - *offset;
    }

    if (copy_from_user(data,
                       buffer,
                       length)) {
        return -EFAULT;
    }

    mutex_lock(&eeprom_mutex);

    ret = eeprom_write_data((u16)*offset,
                            data,
                            length);

    mutex_unlock(&eeprom_mutex);

    if (ret) {
        return ret;
    }

    *offset += length;

    return length;
}


/* ------------------------------------------------------------------------- */
/* Release                                                                   */
/* ------------------------------------------------------------------------- */

static int eeprom_release(struct inode *inode,
                          struct file *file)
{
    pr_info("%s: closed\n",
            EEPROM_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* File Operations                                                           */
/* ------------------------------------------------------------------------- */

static const struct file_operations eeprom_fops = {
    .owner   = THIS_MODULE,
    .open    = eeprom_open,
    .read    = eeprom_read,
    .write   = eeprom_write,
    .release = eeprom_release,
};


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int eeprom_probe(struct i2c_client *client)
{
    int ret;

    eeprom_client = client;

    pr_info("%s: probing I2C address 0x%02X\n",
            EEPROM_DRIVER_NAME,
            client->addr);

    ret = alloc_chrdev_region(&eeprom_dev,
                              0,
                              1,
                              EEPROM_DRIVER_NAME);

    if (ret < 0) {
        return ret;
    }

    cdev_init(&eeprom_cdev,
              &eeprom_fops);

    eeprom_cdev.owner = THIS_MODULE;

    ret = cdev_add(&eeprom_cdev,
                   eeprom_dev,
                   1);

    if (ret) {
        unregister_chrdev_region(eeprom_dev, 1);
        return ret;
    }

    eeprom_class =
        class_create(EEPROM_CLASS_NAME);

    if (IS_ERR(eeprom_class)) {

        ret = PTR_ERR(eeprom_class);

        cdev_del(&eeprom_cdev);
        unregister_chrdev_region(eeprom_dev, 1);

        return ret;
    }

    eeprom_device =
        device_create(eeprom_class,
                      NULL,
                      eeprom_dev,
                      NULL,
                      EEPROM_DEVICE_NAME);

    if (IS_ERR(eeprom_device)) {

        ret = PTR_ERR(eeprom_device);

        class_destroy(eeprom_class);
        cdev_del(&eeprom_cdev);
        unregister_chrdev_region(eeprom_dev, 1);

        return ret;
    }

    pr_info("%s: created /dev/%s\n",
            EEPROM_DRIVER_NAME,
            EEPROM_DEVICE_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void eeprom_remove(struct i2c_client *client)
{
    device_destroy(eeprom_class,
                   eeprom_dev);

    class_destroy(eeprom_class);

    cdev_del(&eeprom_cdev);

    unregister_chrdev_region(eeprom_dev,
                             1);

    eeprom_client = NULL;

    pr_info("%s: removed\n",
            EEPROM_DRIVER_NAME);
}


/* ------------------------------------------------------------------------- */
/* I2C Device ID                                                             */
/* ------------------------------------------------------------------------- */

static const struct i2c_device_id eeprom_id[] = {
    {
        EEPROM_I2C_DEVICE_NAME,
        0
    },
    { }
};

MODULE_DEVICE_TABLE(i2c, eeprom_id);


/* ------------------------------------------------------------------------- */
/* I2C Driver                                                                */
/* ------------------------------------------------------------------------- */

static struct i2c_driver eeprom_driver = {
    .driver = {
        .name = EEPROM_DRIVER_NAME,
    },

    .probe = eeprom_probe,

    .remove = eeprom_remove,

    .id_table = eeprom_id,
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_i2c_driver(eeprom_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black I2C EEPROM Driver");
MODULE_VERSION("1.0");
