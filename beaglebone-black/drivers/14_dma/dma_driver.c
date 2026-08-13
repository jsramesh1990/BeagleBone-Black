/*
 * BeagleBone Black - DMA Driver Demo
 *
 * File:
 *     dma_driver.c
 *
 * Purpose:
 *     Demonstrates Linux DMA Engine API using a memory-to-memory
 *     DMA transfer.
 *
 * Flow:
 *     Device Tree
 *          |
 *          v
 *     Platform Driver
 *          |
 *          v
 *     DMA Channel
 *          |
 *          v
 *     Source Buffer
 *          |
 *          v
 *     DMA memcpy
 *          |
 *          v
 *     Destination Buffer
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "dma_driver.h"


/* ------------------------------------------------------------------------- */
/* Driver Data                                                               */
/* ------------------------------------------------------------------------- */

struct bbb_dma_device {
	struct device *dev;

	struct dma_chan *chan;

	void *src_buf;
	void *dst_buf;

	dma_addr_t src_dma;
	dma_addr_t dst_dma;

	struct completion completion;

	struct mutex lock;
};


/* ------------------------------------------------------------------------- */
/* DMA Completion Callback                                                   */
/* ------------------------------------------------------------------------- */

static void bbb_dma_complete_func(void *arg)
{
	struct bbb_dma_device *dma = arg;

	dev_info(dma->dev,
		 "DMA transfer completed\n");

	complete(&dma->completion);
}


/* ------------------------------------------------------------------------- */
/* DMA Transfer                                                              */
/* ------------------------------------------------------------------------- */

static int bbb_dma_transfer(struct bbb_dma_device *dma)
{
	struct dma_async_tx_descriptor *desc;

	dma_cookie_t cookie;

	unsigned long timeout;

	int ret;


	mutex_lock(&dma->lock);


	/*
	 * Initialize completion.
	 */
	reinit_completion(&dma->completion);


	/*
	 * Fill source buffer with test pattern.
	 */
	memset(dma->src_buf,
	       DMA_TEST_PATTERN,
	       DMA_BUFFER_SIZE);


	/*
	 * Clear destination buffer.
	 */
	memset(dma->dst_buf,
	       0,
	       DMA_BUFFER_SIZE);


	/*
	 * Make sure CPU writes are visible to DMA.
	 */
	dma_sync_single_for_device(
		dma->dev,
		dma->src_dma,
		DMA_BUFFER_SIZE,
		DMA_TO_DEVICE);


	dma_sync_single_for_device(
		dma->dev,
		dma->dst_dma,
		DMA_BUFFER_SIZE,
		DMA_FROM_DEVICE);


	/*
	 * Prepare DMA memory-to-memory transfer.
	 */
	desc = dmaengine_prep_dma_memcpy(
		dma->chan,
		dma->dst_dma,
		dma->src_dma,
		DMA_BUFFER_SIZE,
		DMA_CTRL_ACK |
		DMA_PREP_INTERRUPT);

	if (!desc) {

		dev_err(dma->dev,
			"Failed to prepare DMA transfer\n");

		ret = -EIO;

		goto out_unlock;
	}


	/*
	 * Register completion callback.
	 */
	desc->callback =
		bbb_dma_complete_func;

	desc->callback_param =
		dma;


	/*
	 * Submit transfer.
	 */
	cookie = dmaengine_submit(desc);

	if (dma_submit_error(cookie)) {

		dev_err(dma->dev,
			"DMA submission failed\n");

		ret = -EIO;

		goto out_unlock;
	}


	/*
	 * Start DMA transfer.
	 */
	dma_async_issue_pending(dma->chan);


	dev_info(dma->dev,
		 "DMA transfer started: %u bytes\n",
		 DMA_BUFFER_SIZE);


	/*
	 * Wait for completion.
	 */
	timeout =
		wait_for_completion_timeout(
			&dma->completion,
			msecs_to_jiffies(
				DMA_TIMEOUT_MS));

	if (!timeout) {

		dev_err(dma->dev,
			"DMA transfer timeout\n");

		dmaengine_terminate_sync(
			dma->chan);

		ret = -ETIMEDOUT;

		goto out_unlock;
	}


	/*
	 * Make destination visible to CPU.
	 */
	dma_sync_single_for_cpu(
		dma->dev,
		dma->dst_dma,
		DMA_BUFFER_SIZE,
		DMA_FROM_DEVICE);


	/*
	 * Verify transfer.
	 */
	if (memcmp(dma->src_buf,
		   dma->dst_buf,
		   DMA_BUFFER_SIZE) != 0) {

		dev_err(dma->dev,
			"DMA data verification failed\n");

		ret = -EIO;

		goto out_unlock;
	}


	dev_info(dma->dev,
		 "DMA data verification successful\n");


	ret = 0;


out_unlock:

	mutex_unlock(&dma->lock);

	return ret;
}


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int bbb_dma_probe(struct platform_device *pdev)
{
	struct bbb_dma_device *dma;

	int ret;


	dev_info(&pdev->dev,
		 "Probing BBB DMA driver\n");


	/*
	 * Allocate driver structure.
	 */
	dma = devm_kzalloc(&pdev->dev,
			   sizeof(*dma),
			   GFP_KERNEL);

	if (!dma)
		return -ENOMEM;


	dma->dev = &pdev->dev;


	mutex_init(&dma->lock);

	init_completion(&dma->completion);


	/*
	 * Request DMA channel.
	 *
	 * Device Tree should provide:
	 *
	 *     dmas = <...>;
	 *     dma-names = "memcpy";
	 */
	dma->chan =
		dma_request_chan(
			&pdev->dev,
			DMA_CHANNEL_NAME);

	if (IS_ERR(dma->chan)) {

		ret = PTR_ERR(dma->chan);

		dev_err(&pdev->dev,
			"Failed to request DMA channel: %d\n",
			ret);

		return ret;
	}


	/*
	 * Allocate DMA-capable source buffer.
	 */
	dma->src_buf =
		dma_alloc_coherent(
			&pdev->dev,
			DMA_BUFFER_SIZE,
			&dma->src_dma,
			GFP_KERNEL);

	if (!dma->src_buf) {

		dev_err(&pdev->dev,
			"Failed to allocate source buffer\n");

		ret = -ENOMEM;

		goto release_channel;
	}


	/*
	 * Allocate DMA-capable destination buffer.
	 */
	dma->dst_buf =
		dma_alloc_coherent(
			&pdev->dev,
			DMA_BUFFER_SIZE,
			&dma->dst_dma,
			GFP_KERNEL);

	if (!dma->dst_buf) {

		dev_err(&pdev->dev,
			"Failed to allocate destination buffer\n");

		ret = -ENOMEM;

		goto free_src;
	}


	platform_set_drvdata(pdev,
			     dma);


	dev_info(&pdev->dev,
		 "DMA channel acquired\n");

	dev_info(&pdev->dev,
		 "Source DMA address      : %pad\n",
		 &dma->src_dma);

	dev_info(&pdev->dev,
		 "Destination DMA address : %pad\n",
		 &dma->dst_dma);


	/*
	 * Run one test DMA transfer during probe.
	 */
	ret = bbb_dma_transfer(dma);

	if (ret) {

		dev_err(&pdev->dev,
			"DMA test failed: %d\n",
			ret);

		goto free_dst;
	}


	dev_info(&pdev->dev,
		 "DMA driver initialized successfully\n");

	return 0;


free_dst:

	dma_free_coherent(
		&pdev->dev,
		DMA_BUFFER_SIZE,
		dma->dst_buf,
		dma->dst_dma);


free_src:

	dma_free_coherent(
		&pdev->dev,
		DMA_BUFFER_SIZE,
		dma->src_buf,
		dma->src_dma);


release_channel:

	dma_release_channel(dma->chan);

	return ret;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void bbb_dma_remove(
		struct platform_device *pdev)
{
	struct bbb_dma_device *dma =
		platform_get_drvdata(pdev);


	if (!dma)
		return;


	/*
	 * Stop any pending DMA operation.
	 */
	dmaengine_terminate_sync(
		dma->chan);


	/*
	 * Free destination buffer.
	 */
	dma_free_coherent(
		&pdev->dev,
		DMA_BUFFER_SIZE,
		dma->dst_buf,
		dma->dst_dma);


	/*
	 * Free source buffer.
	 */
	dma_free_coherent(
		&pdev->dev,
		DMA_BUFFER_SIZE,
		dma->src_buf,
		dma->src_dma);


	/*
	 * Release DMA channel.
	 */
	dma_release_channel(
		dma->chan);


	dev_info(&pdev->dev,
		 "BBB DMA driver removed\n");
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                         */
/* ------------------------------------------------------------------------- */

static const struct of_device_id
bbb_dma_of_match[] = {

	{
		.compatible = "bbb,dma-demo",
	},

	{ }
};

MODULE_DEVICE_TABLE(of,
		    bbb_dma_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                           */
/* ------------------------------------------------------------------------- */

static struct platform_driver
bbb_dma_driver = {

	.probe =
		bbb_dma_probe,

	.remove =
		bbb_dma_remove,

	.driver = {
		.name =
			DMA_DRIVER_NAME,

		.of_match_table =
			bbb_dma_of_match,
	},
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_platform_driver(
		bbb_dma_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION(
	"BeagleBone Black Linux DMA Engine Demo");
MODULE_VERSION("1.0");
