/*
 * BeagleBone Black - ALSA Driver Demo
 *
 * File:
 *     alsa_driver.c
 *
 * Purpose:
 *     Demonstrates the Linux ALSA ASoC/PCM driver framework.
 *
 * NOTE:
 *     This is a framework/demo driver. It does not directly
 *     program the AM335x McASP hardware.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#include "alsa_driver.h"


/* ------------------------------------------------------------------------- */
/* Driver Private Data                                                       */
/* ------------------------------------------------------------------------- */

struct bbb_alsa_priv {
	struct snd_card *card;
	struct snd_pcm *pcm;

	spinlock_t lock;

	unsigned int sample_rate;
	unsigned int channels;
	unsigned int sample_bits;

	snd_pcm_uframes_t buffer_size;
	snd_pcm_uframes_t period_size;
};


/* ------------------------------------------------------------------------- */
/* PCM Hardware Capabilities                                                */
/* ------------------------------------------------------------------------- */

static const struct snd_pcm_hardware bbb_alsa_pcm_hw = {

	.info =
		SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_BLOCK_TRANSFER |
		SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID,

	.formats =
		SNDRV_PCM_FMTBIT_S16_LE |
		SNDRV_PCM_FMTBIT_S24_LE |
		SNDRV_PCM_FMTBIT_S32_LE,

	.rates =
		SNDRV_PCM_RATE_8000 |
		SNDRV_PCM_RATE_16000 |
		SNDRV_PCM_RATE_44100 |
		SNDRV_PCM_RATE_48000,

	.rate_min =
		8000,

	.rate_max =
		48000,

	.channels_min =
		1,

	.channels_max =
		2,

	.buffer_bytes_max =
		ALSA_BUFFER_BYTES,

	.period_bytes_min =
		ALSA_PERIOD_BYTES_MIN,

	.period_bytes_max =
		ALSA_PERIOD_BYTES_MAX,

	.periods_min =
		2,

	.periods_max =
		ALSA_PERIODS_MAX,
};


/* ------------------------------------------------------------------------- */
/* PCM Open                                                                  */
/* ------------------------------------------------------------------------- */

static int bbb_alsa_pcm_open(
		struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime;

	runtime =
		substream->runtime;

	runtime->hw =
		bbb_alsa_pcm_hw;

	pr_info(
		"BBB ALSA: PCM open - %s\n",
		substream->stream ==
		SNDRV_PCM_STREAM_PLAYBACK ?
		"playback" : "capture");

	return 0;
}


/* ------------------------------------------------------------------------- */
/* PCM Close                                                                 */
/* ------------------------------------------------------------------------- */

static int bbb_alsa_pcm_close(
		struct snd_pcm_substream *substream)
{
	pr_info(
		"BBB ALSA: PCM close\n");

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Hardware Parameters                                                       */
/* ------------------------------------------------------------------------- */

static int bbb_alsa_hw_params(
		struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime;

	runtime =
		substream->runtime;

	runtime->dma_bytes =
		params_buffer_bytes(params);

	pr_info(
		"BBB ALSA: HW params\n");

	pr_info(
		"Rate     : %u Hz\n",
		params_rate(params));

	pr_info(
		"Channels : %u\n",
		params_channels(params));

	pr_info(
		"Format   : %u\n",
		params_format(params));

	pr_info(
		"Buffer   : %lu bytes\n",
		(unsigned long)
		params_buffer_bytes(params));

	pr_info(
		"Period   : %lu bytes\n",
		(unsigned long)
		params_period_bytes(params));

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Hardware Free                                                             */
/* ------------------------------------------------------------------------- */

static int bbb_alsa_hw_free(
		struct snd_pcm_substream *substream)
{
	pr_info(
		"BBB ALSA: HW free\n");

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Prepare                                                                   */
/* ------------------------------------------------------------------------- */

static int bbb_alsa_prepare(
		struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime;

	runtime =
		substream->runtime;

	pr_info(
		"BBB ALSA: prepare\n");

	pr_info(
		"Rate=%u Channels=%u\n",
		runtime->rate,
		runtime->channels);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Trigger                                                                   */
/* ------------------------------------------------------------------------- */

static int bbb_alsa_trigger(
		struct snd_pcm_substream *substream,
		int cmd)
{
	switch (cmd) {

	case SNDRV_PCM_TRIGGER_START:

		pr_info(
			"BBB ALSA: trigger START\n");

		break;


	case SNDRV_PCM_TRIGGER_STOP:

		pr_info(
			"BBB ALSA: trigger STOP\n");

		break;


	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:

		pr_info(
			"BBB ALSA: trigger PAUSE\n");

		break;


	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:

		pr_info(
			"BBB ALSA: trigger RESUME\n");

		break;


	default:

		return -EINVAL;
	}

	return 0;
}


/* ------------------------------------------------------------------------- */
/* PCM Pointer                                                               */
/* ------------------------------------------------------------------------- */

static snd_pcm_uframes_t bbb_alsa_pointer(
		struct snd_pcm_substream *substream)
{
	/*
	 * Real driver would return the current
	 * DMA hardware position.
	 */

	return 0;
}


/* ------------------------------------------------------------------------- */
/* PCM Operations                                                            */
/* ------------------------------------------------------------------------- */

static const struct snd_pcm_ops bbb_alsa_pcm_ops = {

	.open =
		bbb_alsa_pcm_open,

	.close =
		bbb_alsa_pcm_close,

	.ioctl =
		snd_pcm_lib_ioctl,

	.hw_params =
		bbb_alsa_hw_params,

	.hw_free =
		bbb_alsa_hw_free,

	.prepare =
		bbb_alsa_prepare,

	.trigger =
		bbb_alsa_trigger,

	.pointer =
		bbb_alsa_pointer,
};


/* ------------------------------------------------------------------------- */
/* PCM Create                                                                */
/* ------------------------------------------------------------------------- */

static int bbb_alsa_create_pcm(
		struct snd_card *card,
		struct bbb_alsa_priv *priv)
{
	struct snd_pcm *pcm;

	int ret;


	ret =
		snd_pcm_new(
			card,
			ALSA_PCM_NAME,
			0,
			ALSA_PLAYBACK_STREAMS,
			ALSA_CAPTURE_STREAMS,
			&pcm);

	if (ret < 0) {

		dev_err(
			card->dev,
			"Failed to create PCM: %d\n",
			ret);

		return ret;
	}


	priv->pcm =
		pcm;


	pcm->private_data =
		priv;


	pcm->info_flags =
		0;


	strscpy(
		pcm->name,
		ALSA_PCM_NAME,
		sizeof(pcm->name));


	/*
	 * Playback operations.
	 */
	snd_pcm_set_ops(
		pcm,
		SNDRV_PCM_STREAM_PLAYBACK,
		&bbb_alsa_pcm_ops);


	/*
	 * Capture operations.
	 */
	snd_pcm_set_ops(
		pcm,
		SNDRV_PCM_STREAM_CAPTURE,
		&bbb_alsa_pcm_ops);


	return 0;
}


/* ------------------------------------------------------------------------- */
/* Platform Probe                                                            */
/* ------------------------------------------------------------------------- */

static int bbb_alsa_probe(
		struct platform_device *pdev)
{
	struct snd_card *card;

	struct bbb_alsa_priv *priv;

	int ret;


	dev_info(
		&pdev->dev,
		"BBB ALSA driver probe\n");


	/*
	 * Allocate ALSA sound card.
	 */
	ret =
		snd_card_new(
			&pdev->dev,
			-1,
			ALSA_CARD_ID,
			THIS_MODULE,
			sizeof(struct bbb_alsa_priv),
			&card);

	if (ret < 0) {

		dev_err(
			&pdev->dev,
			"Failed to allocate sound card: %d\n",
			ret);

		return ret;
	}


	priv =
		card->private_data;


	priv->card =
		card;


	spin_lock_init(
		&priv->lock);


	/*
	 * Card information.
	 */
	strscpy(
		card->driver,
		ALSA_DRIVER_NAME,
		sizeof(card->driver));

	strscpy(
		card->shortname,
		ALSA_CARD_NAME,
		sizeof(card->shortname));

	snprintf(
		card->longname,
		sizeof(card->longname),
		"%s ALSA Sound Card",
		ALSA_CARD_NAME);


	/*
	 * Create PCM device.
	 */
	ret =
		bbb_alsa_create_pcm(
			card,
			priv);

	if (ret < 0) {

		snd_card_free(card);

		return ret;
	}


	/*
	 * Register ALSA sound card.
	 */
	ret =
		snd_card_register(card);

	if (ret < 0) {

		dev_err(
			&pdev->dev,
			"Failed to register sound card: %d\n",
			ret);

		snd_card_free(card);

		return ret;
	}


	platform_set_drvdata(
		pdev,
		card);


	dev_info(
		&pdev->dev,
		"ALSA sound card registered\n");


	dev_info(
		&pdev->dev,
		"Card: %s\n",
		card->longname);


	return 0;
}


/* ------------------------------------------------------------------------- */
/* Platform Remove                                                           */
/* ------------------------------------------------------------------------- */

static void bbb_alsa_remove(
		struct platform_device *pdev)
{
	struct snd_card *card;

	card =
		platform_get_drvdata(pdev);


	if (!card)
		return;


	snd_card_free(card);


	dev_info(
		&pdev->dev,
		"ALSA sound card removed\n");
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                         */
/* ------------------------------------------------------------------------- */

static const struct of_device_id
bbb_alsa_of_match[] = {

	{
		.compatible =
			"bbb,alsa-demo",
	},

	{ }
};

MODULE_DEVICE_TABLE(
	of,
	bbb_alsa_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                           */
/* ------------------------------------------------------------------------- */

static struct platform_driver
bbb_alsa_driver = {

	.probe =
		bbb_alsa_probe,

	.remove =
		bbb_alsa_remove,

	.driver = {

		.name =
			ALSA_DRIVER_NAME,

		.of_match_table =
			bbb_alsa_of_match,
	},
};


module_platform_driver(
	bbb_alsa_driver);


/* ------------------------------------------------------------------------- */
/* Module Information                                                        */
/* ------------------------------------------------------------------------- */

MODULE_LICENSE("GPL");

MODULE_AUTHOR(
	"Embedded Software Engineer");

MODULE_DESCRIPTION(
	"BeagleBone Black ALSA PCM Driver Demo");

MODULE_VERSION(
	ALSA_DRIVER_VERSION);
