/*
 * BeagleBone Black - Ethernet Driver Demo
 *
 * File:
 *     ethernet_demo.c
 *
 * Purpose:
 *     Demonstrates the Linux networking driver framework using
 *     struct net_device and net_device_ops.
 *
 * NOTE:
 *     This is a framework/demo driver. It does NOT directly
 *     program the AM335x CPSW Ethernet hardware.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/spinlock.h>

#include "ethernet_demo.h"


/* ------------------------------------------------------------------------- */
/* Driver Private Data                                                       */
/* ------------------------------------------------------------------------- */

struct bbb_eth_priv {
	struct net_device *netdev;

	spinlock_t lock;

	unsigned long tx_packets;
	unsigned long tx_bytes;
	unsigned long rx_packets;
	unsigned long rx_bytes;
	unsigned long tx_errors;
	unsigned long rx_errors;
};


/* ------------------------------------------------------------------------- */
/* Network Device Open                                                       */
/* ------------------------------------------------------------------------- */

static int bbb_eth_open(struct net_device *netdev)
{
	struct bbb_eth_priv *priv =
		netdev_priv(netdev);

	netif_start_queue(netdev);

	dev_info(&netdev->dev,
		 "Ethernet interface opened\n");

	dev_info(&netdev->dev,
		 "Interface: %s\n",
		 netdev->name);

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Network Device Stop                                                       */
/* ------------------------------------------------------------------------- */

static int bbb_eth_stop(struct net_device *netdev)
{
	netif_stop_queue(netdev);

	dev_info(&netdev->dev,
		 "Ethernet interface stopped\n");

	return 0;
}


/* ------------------------------------------------------------------------- */
/* Packet Transmit                                                           */
/* ------------------------------------------------------------------------- */

static netdev_tx_t bbb_eth_start_xmit(
		struct sk_buff *skb,
		struct net_device *netdev)
{
	struct bbb_eth_priv *priv =
		netdev_priv(netdev);

	unsigned long flags;


	/*
	 * Validate packet.
	 */
	if (!skb) {

		priv->tx_errors++;

		return NETDEV_TX_OK;
	}


	/*
	 * In a real driver:
	 *
	 *     1. Map skb using DMA API
	 *     2. Place DMA address in TX descriptor
	 *     3. Notify Ethernet MAC
	 *     4. Hardware transmits packet
	 *
	 * This demo only records the packet.
	 */

	spin_lock_irqsave(
		&priv->lock,
		flags);


	priv->tx_packets++;

	priv->tx_bytes += skb->len;


	spin_unlock_irqrestore(
		&priv->lock,
		flags);


	dev_dbg(&netdev->dev,
		"TX packet: %u bytes\n",
		skb->len);


	/*
	 * Free skb because this demo does
	 * not have real hardware ownership.
	 */
	dev_kfree_skb(skb);


	return NETDEV_TX_OK;
}


/* ------------------------------------------------------------------------- */
/* Get Statistics                                                            */
/* ------------------------------------------------------------------------- */

static struct rtnl_link_stats64 *
bbb_eth_get_stats64(
		struct net_device *netdev,
		struct rtnl_link_stats64 *stats)
{
	struct bbb_eth_priv *priv =
		netdev_priv(netdev);

	unsigned long flags;


	spin_lock_irqsave(
		&priv->lock,
		flags);


	stats->tx_packets =
		priv->tx_packets;

	stats->tx_bytes =
		priv->tx_bytes;

	stats->rx_packets =
		priv->rx_packets;

	stats->rx_bytes =
		priv->rx_bytes;

	stats->tx_errors =
		priv->tx_errors;

	stats->rx_errors =
		priv->rx_errors;


	spin_unlock_irqrestore(
		&priv->lock,
		flags);


	return stats;
}


/* ------------------------------------------------------------------------- */
/* MAC Address Setup                                                         */
/* ------------------------------------------------------------------------- */

static int bbb_eth_set_mac_address(
		struct net_device *netdev,
		void *addr)
{
	struct sockaddr *sa =
		(struct sockaddr *)addr;


	if (!is_valid_ether_addr(
			sa->sa_data))
		return -EADDRNOTAVAIL;


	memcpy(netdev->dev_addr,
	       sa->sa_data,
	       ETH_ALEN);


	dev_info(&netdev->dev,
		 "MAC address changed to %pM\n",
		 netdev->dev_addr);


	return 0;
}


/* ------------------------------------------------------------------------- */
/* Network Device Operations                                                 */
/* ------------------------------------------------------------------------- */

static const struct net_device_ops
bbb_eth_netdev_ops = {

	.ndo_open =
		bbb_eth_open,

	.ndo_stop =
		bbb_eth_stop,

	.ndo_start_xmit =
		bbb_eth_start_xmit,

	.ndo_get_stats64 =
		bbb_eth_get_stats64,

	.ndo_set_mac_address =
		bbb_eth_set_mac_address,
};


/* ------------------------------------------------------------------------- */
/* Ethtool Operations                                                        */
/* ------------------------------------------------------------------------- */

static void bbb_eth_get_drvinfo(
		struct net_device *netdev,
		struct ethtool_drvinfo *info)
{
	strscpy(
		info->driver,
		ETHERNET_DRIVER_NAME,
		sizeof(info->driver));

	strscpy(
		info->version,
		ETHERNET_DRIVER_VERSION,
		sizeof(info->version));

	strscpy(
		info->bus_info,
		"platform",
		sizeof(info->bus_info));
}


static const struct ethtool_ops
bbb_eth_ethtool_ops = {

	.get_drvinfo =
		bbb_eth_get_drvinfo,
};


/* ------------------------------------------------------------------------- */
/* Setup Network Device                                                      */
/* ------------------------------------------------------------------------- */

static void bbb_eth_setup(
		struct net_device *netdev)
{
	ether_setup(netdev);


	netdev->netdev_ops =
		&bbb_eth_netdev_ops;


	netdev->ethtool_ops =
		&bbb_eth_ethtool_ops;


	netdev->watchdog_timeo =
		msecs_to_jiffies(
			ETHERNET_TX_TIMEOUT_MS);


	/*
	 * Default demo MAC address.
	 *
	 * Replace with a valid hardware MAC in
	 * a real Ethernet driver.
	 */
	eth_hw_addr_set(
		netdev,
		(unsigned char[]) {
			0x02,
			0x00,
			0x00,
			0xBB,
			0x00,
			0x01
		});
}


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int bbb_eth_probe(
		struct platform_device *pdev)
{
	struct net_device *netdev;

	struct bbb_eth_priv *priv;

	int ret;


	dev_info(&pdev->dev,
		 "Probing BBB Ethernet demo\n");


	/*
	 * Allocate network device.
	 */
	netdev =
		alloc_etherdev(
			sizeof(struct bbb_eth_priv));

	if (!netdev)
		return -ENOMEM;


	/*
	 * Initialize network device.
	 */
	bbb_eth_setup(netdev);


	/*
	 * Get private data.
	 */
	priv =
		netdev_priv(netdev);


	priv->netdev =
		netdev;


	spin_lock_init(
		&priv->lock);


	SET_NETDEV_DEV(
		netdev,
		&pdev->dev);


	/*
	 * Register with Linux networking
	 * subsystem.
	 */
	ret =
		register_netdev(netdev);

	if (ret) {

		dev_err(
			&pdev->dev,
			"Failed to register netdev: %d\n",
			ret);

		free_netdev(netdev);

		return ret;
	}


	platform_set_drvdata(
		pdev,
		netdev);


	dev_info(&pdev->dev,
		 "Ethernet interface %s registered\n",
		 netdev->name);


	dev_info(&pdev->dev,
		 "MAC address: %pM\n",
		 netdev->dev_addr);


	return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void bbb_eth_remove(
		struct platform_device *pdev)
{
	struct net_device *netdev =
		platform_get_drvdata(pdev);


	if (!netdev)
		return;


	unregister_netdev(
		netdev);


	free_netdev(
		netdev);


	dev_info(&pdev->dev,
		 "BBB Ethernet driver removed\n");
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                         */
/* ------------------------------------------------------------------------- */

static const struct of_device_id
bbb_eth_of_match[] = {

	{
		.compatible =
			"bbb,ethernet-demo",
	},

	{ }
};

MODULE_DEVICE_TABLE(
	of,
	bbb_eth_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                           */
/* ------------------------------------------------------------------------- */

static struct platform_driver
bbb_eth_driver = {

	.probe =
		bbb_eth_probe,

	.remove =
		bbb_eth_remove,

	.driver = {

		.name =
			ETHERNET_DRIVER_NAME,

		.of_match_table =
			bbb_eth_of_match,
	},
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_platform_driver(
		bbb_eth_driver);


MODULE_LICENSE("GPL");

MODULE_AUTHOR(
	"Embedded Software Engineer");

MODULE_DESCRIPTION(
	"BeagleBone Black Ethernet Network Driver Demo");

MODULE_VERSION(
	ETHERNET_DRIVER_VERSION);
