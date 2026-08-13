/*
 * BeagleBone Black - ADC Driver
 *
 * File:
 *     adc_driver.c
 *
 * Purpose:
 *     Example ADC driver using the Linux IIO framework.
 *
 * Features:
 *     - Platform driver
 *     - Device Tree matching
 *     - ADC channel handling
 *     - Raw ADC value read
 *     - IIO interface
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/mutex.h>

#include "adc_driver.h"


/* ------------------------------------------------------------------------- */
/* Driver Private Data                                                       */
/* ------------------------------------------------------------------------- */

struct adc_device {
    void __iomem *base;
    resource_size_t size;

    struct mutex lock;

    u32 channel_count;
};


/* ------------------------------------------------------------------------- */
/* ADC Register Access                                                       */
/* ------------------------------------------------------------------------- */

static inline u32 adc_read_reg(struct adc_device *adc,
                               u32 offset)
{
    return readl(adc->base + offset);
}


static inline void adc_write_reg(struct adc_device *adc,
                                 u32 offset,
                                 u32 value)
{
    writel(value,
           adc->base + offset);
}


/* ------------------------------------------------------------------------- */
/* ADC Channel Read                                                          */
/* ------------------------------------------------------------------------- */

static int adc_read_channel(struct adc_device *adc,
                            unsigned int channel,
                            int *value)
{
    u32 reg;
    u32 raw_value;

    if (channel >= adc->channel_count) {
        return -EINVAL;
    }

    /*
     * Select ADC channel.
     */
    adc_write_reg(adc,
                  ADC_CHANNEL_SELECT_REG,
                  channel);

    /*
     * Start conversion.
     */
    adc_write_reg(adc,
                  ADC_CONTROL_REG,
                  ADC_START_CONVERSION);

    /*
     * In a production driver, conversion completion should normally
     * be handled using the ADC's interrupt/completion mechanism.
     *
     * This example uses a simple status polling loop.
     */
    {
        unsigned int timeout = ADC_TIMEOUT;

        while (!(adc_read_reg(
                     adc,
                     ADC_STATUS_REG) &
                 ADC_CONVERSION_DONE)) {

            if (--timeout == 0) {
                return -ETIMEDOUT;
            }

            cpu_relax();
        }
    }

    /*
     * Read ADC result.
     */
    reg = adc_read_reg(adc,
                       ADC_DATA_REG);

    raw_value =
        reg & ADC_DATA_MASK;

    *value = raw_value;

    return 0;
}


/* ------------------------------------------------------------------------- */
/* IIO Read Raw                                                              */
/* ------------------------------------------------------------------------- */

static int adc_read_raw(struct iio_dev *indio_dev,
                        struct iio_chan_spec const *channel,
                        int *val,
                        int *val2,
                        long mask)
{
    struct adc_device *adc =
        iio_priv(indio_dev);

    int ret;

    switch (mask) {

    case IIO_CHAN_INFO_RAW:

        mutex_lock(&adc->lock);

        ret = adc_read_channel(adc,
                               channel->channel,
                               val);

        mutex_unlock(&adc->lock);

        if (ret) {
            return ret;
        }

        return IIO_VAL_INT;


    default:

        return -EINVAL;
    }
}


/* ------------------------------------------------------------------------- */
/* IIO Operations                                                            */
/* ------------------------------------------------------------------------- */

static const struct iio_info adc_iio_info = {
    .read_raw = adc_read_raw,
};


/* ------------------------------------------------------------------------- */
/* ADC Channels                                                              */
/* ------------------------------------------------------------------------- */

static const struct iio_chan_spec adc_channels[] = {

    {
        .type = IIO_VOLTAGE,
        .indexed = 1,
        .channel = 0,
        .info_mask_separate =
            BIT(IIO_CHAN_INFO_RAW),
    },

    {
        .type = IIO_VOLTAGE,
        .indexed = 1,
        .channel = 1,
        .info_mask_separate =
            BIT(IIO_CHAN_INFO_RAW),
    },

    {
        .type = IIO_VOLTAGE,
        .indexed = 1,
        .channel = 2,
        .info_mask_separate =
            BIT(IIO_CHAN_INFO_RAW),
    },

    {
        .type = IIO_VOLTAGE,
        .indexed = 1,
        .channel = 3,
        .info_mask_separate =
            BIT(IIO_CHAN_INFO_RAW),
    },

    {
        .type = IIO_VOLTAGE,
        .indexed = 1,
        .channel = 4,
        .info_mask_separate =
            BIT(IIO_CHAN_INFO_RAW),
    },

    {
        .type = IIO_VOLTAGE,
        .indexed = 1,
        .channel = 5,
        .info_mask_separate =
            BIT(IIO_CHAN_INFO_RAW),
    },

    {
        .type = IIO_VOLTAGE,
        .indexed = 1,
        .channel = 6,
        .info_mask_separate =
            BIT(IIO_CHAN_INFO_RAW),
    },

    {
        .type = IIO_VOLTAGE,
        .indexed = 1,
        .channel = 7,
        .info_mask_separate =
            BIT(IIO_CHAN_INFO_RAW),
    },
};


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int adc_probe(struct platform_device *pdev)
{
    struct iio_dev *indio_dev;

    struct adc_device *adc;

    struct resource *res;

    int ret;


    pr_info("%s: probing ADC driver\n",
            ADC_DRIVER_NAME);


    /*
     * Allocate IIO device and private data.
     */
    indio_dev =
        devm_iio_device_alloc(&pdev->dev,
                              sizeof(*adc));

    if (!indio_dev) {
        return -ENOMEM;
    }


    adc = iio_priv(indio_dev);


    /*
     * Initialize driver lock.
     */
    mutex_init(&adc->lock);


    /*
     * Get ADC memory resource from Device Tree.
     */
    res = platform_get_resource(pdev,
                                IORESOURCE_MEM,
                                0);

    if (!res) {

        dev_err(&pdev->dev,
                "ADC memory resource not found\n");

        return -ENODEV;
    }


    adc->size =
        resource_size(res);


    /*
     * Map ADC registers.
     */
    adc->base =
        devm_ioremap_resource(&pdev->dev,
                              res);

    if (IS_ERR(adc->base)) {

        ret = PTR_ERR(adc->base);

        dev_err(&pdev->dev,
                "Failed to map ADC registers: %d\n",
                ret);

        return ret;
    }


    /*
     * Number of ADC channels.
     */
    adc->channel_count =
        ADC_CHANNEL_COUNT;


    /*
     * Configure IIO device.
     */
    indio_dev->name =
        ADC_DEVICE_NAME;

    indio_dev->info =
        &adc_iio_info;

    indio_dev->modes =
        INDIO_DIRECT_MODE;

    indio_dev->channels =
        adc_channels;

    indio_dev->num_channels =
        ARRAY_SIZE(adc_channels);


    /*
     * Register with IIO subsystem.
     */
    ret =
        devm_iio_device_register(&pdev->dev,
                                 indio_dev);

    if (ret) {

        dev_err(&pdev->dev,
                "IIO registration failed: %d\n",
                ret);

        return ret;
    }


    platform_set_drvdata(pdev,
                         indio_dev);


    pr_info("%s: ADC driver registered\n",
            ADC_DRIVER_NAME);

    pr_info("%s: channels=%u\n",
            ADC_DRIVER_NAME,
            adc->channel_count);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void adc_remove(struct platform_device *pdev)
{
    pr_info("%s: ADC driver removed\n",
            ADC_DRIVER_NAME);
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                         */
/* ------------------------------------------------------------------------- */

static const struct of_device_id adc_of_match[] = {

    {
        .compatible = "bbb,adc-test",
    },

    { }
};

MODULE_DEVICE_TABLE(of,
                    adc_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                           */
/* ------------------------------------------------------------------------- */

static struct platform_driver adc_platform_driver = {

    .probe = adc_probe,

    .remove = adc_remove,

    .driver = {
        .name = ADC_DRIVER_NAME,

        .of_match_table =
            adc_of_match,
    },
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_platform_driver(adc_platform_driver);


MODULE_LICENSE("GPL");

MODULE_AUTHOR("Embedded Software Engineer");

MODULE_DESCRIPTION("BeagleBone Black ADC Driver using Linux IIO");

MODULE_VERSION("1.0");
