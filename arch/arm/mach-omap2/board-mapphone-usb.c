/*
 * kernel/arch/arm/mach-omap2/board-mapphone-usb.c
 *
 * Copyright (C) 2010-2012 Motorola Mobility, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <asm/mach-types.h>
#include <linux/of.h>
#include "dt_path.h"
#include "board-mapphone.h"
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <linux/gpio_mapping.h>
#include <plat/mux.h>
#include <plat/usb.h>
#include <linux/usb/otg.h>
#include <linux/usb/composite.h>
#include <linux/usb/mdm6600_usb.h>
#include <plat/omap-pm.h>

#if defined(CONFIG_USB_MOT_ANDROID) && defined(CONFIG_USB_MUSB_OTG)
#include <linux/spi/cpcap.h>
#include <linux/usb/musb.h>
#include <linux/usb/composite.h>
#endif
#include <linux/usb/oob_wake.h>

#include <plat/common.h>
#include "cm-regbits-34xx.h"
#include "clock.h"
#include "dvfs.h"

#define MAPPHONE_BP_READY2_AP_GPIO      59
#define MAPPHONE_IPC_USB_SUSP_GPIO	95
#define DIE_ID_REG_BASE			(L4_34XX_PHYS + 0xA000)
#define DIE_ID_REG_OFFSET		0x218
#define DIE_ID_REG_BASE_44XX		(L4_44XX_PHYS + 0x2000)
#define DIE_ID_REG_OFFSET_44XX		0x200
#define EHCI_IRQ                        (77 + OMAP44XX_IRQ_GIC_START)

void cpcap_musb_notifier_call(unsigned long event);
#if defined(CONFIG_USB_MOT_ANDROID) && defined(CONFIG_USB_MUSB_OTG)
struct cpcap_accy_platform_data {
	enum cpcap_accy accy;
};
#endif


static struct device_pid mot_android_pid[MAX_DEVICE_TYPE_NUM] = {
	{"mtp,usbnet",		0},
	{"mtp,usbnet,adb",	0},
	{"ptp",		0},
	{"ptp,adb",	0},
	{"rndis",			0},
	{"rndis,adb",		0},
	{"cdrom",               0},
	{"mass_storage",        0},
	{"mass_storage,adb",    0},
	{"cdrom2",              0},
	{}
};

static struct android_usb_platform_data andusb_plat = {
	.vendor			= "Motorola",
	.product_name	= "Android",
	.android_pid	= mot_android_pid,
	.nluns			= 1,
	.cdrom_lun_num          = 0,
};

static void set_usb_performance_mode(struct device *dev, bool enabled)
{
	struct device *mpu_dev;
	dev_dbg(dev, "Performance Mode %s\n", enabled ? "Set" : "Cleared");
	mpu_dev = omap2_get_mpuss_device();

	if (!mpu_dev) {
		pr_warning("%s: unable to get the mpu device\n", __func__);
		return;
	}

	if (enabled) {
		if (andusb_plat.bp_tools_mode)
			omap_device_scale(dev, mpu_dev, 800000000);
		else
			omap_device_scale(dev, mpu_dev, 600000000);
		if (num_possible_cpus() > 1
				&& cpu_online(num_possible_cpus()-1))
			irq_set_affinity(EHCI_IRQ,
					cpumask_of(num_possible_cpus()-1));
	} else {
		omap_device_scale(dev, mpu_dev, 300000000);
		irq_set_affinity(EHCI_IRQ, cpu_online_mask);
	}


}


static struct platform_device android_usb_platform_device = {
	.name	= "android_gadget",
	.id	= -1,
	.dev	= {
		.platform_data = &andusb_plat,
	},
};

static int cpcap_usb_connected_probe(struct platform_device *pdev)
{
	struct cpcap_accy_platform_data *pdata = pdev->dev.platform_data;

	if (pdata->accy == CPCAP_ACCY_USB_DEVICE) {
		printk(KERN_INFO "SW:CPCAP_ACCY_USB_DEVICE Connected\n");
#if defined(CONFIG_USB_MOT_ANDROID) && defined(CONFIG_USB_MUSB_OTG)
		/*
		 * First time connection in host mode fails in several
		 * cases due to dock issues. Retrying again fixes the issue.
		 */
		cpcap_musb_notifier_call(USB_EVENT_ID);
		msleep(5);
		cpcap_musb_notifier_call(USB_EVENT_NONE);
		msleep(5);
		cpcap_musb_notifier_call(USB_EVENT_ID);
#endif
	} else {
		android_usb_set_connected(1, pdata->accy);
#if defined(CONFIG_USB_MOT_ANDROID) && defined(CONFIG_USB_MUSB_OTG)
		cpcap_musb_notifier_call(USB_EVENT_VBUS);
#endif
	}

	return 0;
}

static int cpcap_usb_connected_remove(struct platform_device *pdev)
{
	struct cpcap_accy_platform_data *pdata = pdev->dev.platform_data;

	if (pdata->accy == CPCAP_ACCY_USB_DEVICE)
		printk(KERN_INFO "SW:CPCAP_ACCY_USB_DEVICE removed\n");
	else
		android_usb_set_connected(0, pdata->accy);

#if defined(CONFIG_USB_MOT_ANDROID) && defined(CONFIG_USB_MUSB_OTG)
	cpcap_musb_notifier_call(USB_EVENT_NONE);
#endif

	return 0;
}

static struct platform_driver cpcap_usb_connected_driver = {
	.probe		= cpcap_usb_connected_probe,
	.remove		= cpcap_usb_connected_remove,
	.driver		= {
		.name	= "cpcap_usb_connected",
		.owner	= THIS_MODULE,
	},
};

#define USB_FUNC_NAME_SIZE     20

struct omap_usb_pid_entry {
	char name[USB_FUNC_NAME_SIZE];
	u16 usb_pid;
} __attribute__ ((__packed__));

void trim_usb_name_string(char *s)
{
int i;

	/* ignore all characters behind space key */
	for (i = 0; i < USB_FUNC_NAME_SIZE; i++) {
		if (' ' == s[i]) {
			s[i] = '\0';
			return;
		}
	}

	printk(KERN_ERR "Gadget Driver - usb function name is too long!\n");
}

void android_usb_set_pid(char *fname, u16 usb_pid)
{
	int j;

	for (j = 0; j < MAX_DEVICE_TYPE_NUM; j++) {
		if (andusb_plat.android_pid[j].name == NULL)
			break;

		if (strncmp(andusb_plat.android_pid[j].name, fname,
			MAX_DEVICE_NAME_SIZE) == 0) {
			andusb_plat.android_pid[j].pid = usb_pid;
			break;
		}
	}

	if (j == MAX_DEVICE_TYPE_NUM)
		printk(KERN_ERR
			"Unable to find a match for the DT entry %s\n",
			fname);
	else
		printk(KERN_INFO
			"USB PID overwritten for  %s with 0x%4x\n",
			andusb_plat.android_pid[j].name,
			andusb_plat.android_pid[j].pid);
}

void __init usb_pid_mapping_init(void)
{
	struct device_node *node;
	const void *prop;
	int i, size, unit_size;
	char name[USB_FUNC_NAME_SIZE];

	node = of_find_node_by_path(DT_PATH_CHOSEN);
	if (node == NULL) {
		printk(KERN_ERR
			"Unable to read node %s from device tree!\n",
			DT_PATH_CHOSEN);
		return;
	}

	unit_size = sizeof(struct omap_usb_pid_entry);
	prop = of_get_property(node, DT_PROP_CHOSEN_USB_PIDS, &size);
	if ((!prop) || (size % unit_size)) {
		printk(KERN_ERR "Read property %s error!\n",
			DT_PROP_CHOSEN_USB_PIDS);
			of_node_put(node);
		return;
	}

	for (i = 0; i < size / unit_size; i++) {
		struct omap_usb_pid_entry *p =
		(struct omap_usb_pid_entry *) prop;

		memcpy((void *) name, p->name, USB_FUNC_NAME_SIZE);
		trim_usb_name_string(name);
		android_usb_set_pid(name, p->usb_pid);
		prop += unit_size;
	}

	of_node_put(node);
	printk(KERN_INFO "DT overwrite of  USB PID's done!\n");
}

void mapphone_init_cdrom_lun_num(void)
{
	struct device_node *node;
	const void *prop;

	node = of_find_node_by_path(DT_PATH_CHOSEN);
	if (node == NULL) {
		pr_err("Unable to read node %s from device tree!\n",
			DT_PATH_CHOSEN);
		return;
	}
	prop = of_get_property(node, DT_PROP_CHOSEN_USB_CDROM_LUN_NUM, NULL);
	if (prop) {
		pr_err("USB Overwrite CDROM Lun Num with %d\n", *(char *)prop);
		andusb_plat.cdrom_lun_num = *(char *)prop;
	}

	of_node_put(node);
	return;
}

void mapphone_init_nluns(void)
{
	struct device_node *node;
	const void *prop;

	node = of_find_node_by_path(DT_PATH_CHOSEN);
	if (node == NULL) {
		pr_err("Unable to read node %s from device tree!\n",
			DT_PATH_CHOSEN);
		return;
	}
	prop = of_get_property(node, DT_PROP_CHOSEN_USB_NLUNS, NULL);
	if (prop) {
		pr_err("USB Overwrite nLuns %d\n", *(char *)prop);
		andusb_plat.nluns = *(char *)prop;
	}

	of_node_put(node);
	return;
}

#define MODELNO_MAX_LEN 16
char cmdline_modelno[MODELNO_MAX_LEN] = {};

int __init board_modelno_init(char *s)
{
	strlcpy(cmdline_modelno, s, MODELNO_MAX_LEN);
	return 1;
}
__setup("androidboot.modelno=", board_modelno_init);

void mapphone_get_product_name(void)
{
	struct device_node *node;
	const void *prop;

	if (strlen(cmdline_modelno)) {
		andusb_plat.product_name = (char *)cmdline_modelno;
	} else {
		node = of_find_node_by_path(DT_PATH_CHOSEN);
		if (node == NULL) {
			pr_err("Unable to read node %s from device tree!\n",
				DT_PATH_CHOSEN);
			return;
		}

		prop = of_get_property(node, DT_PROP_CHOSEN_USB_PROD_NAME,
				       NULL);
		if (prop) {
			andusb_plat.product_name = (char *)prop;
		} else {
			pr_err("Read property %s error!\n",
			DT_PROP_CHOSEN_USB_PROD_NAME);
		}

		of_node_put(node);
	}

	return;
}

void mapphone_get_serial_number(void)
{
	unsigned int val[2];
	unsigned int reg;

	if (cpu_is_omap44xx())
		reg = DIE_ID_REG_BASE_44XX + DIE_ID_REG_OFFSET_44XX;
	else
		reg = DIE_ID_REG_BASE + DIE_ID_REG_OFFSET;

	val[0] = omap_readl(reg);

	if (cpu_is_omap44xx())
		/* OMAP4 has the id_code register at die_id_0 + 4*/
		val[1] = omap_readl(reg + 8);
	else
		val[1] = omap_readl(reg + 4);

	snprintf(andusb_plat.device_serial, MAX_USB_SERIAL_NUM, "%08X%08X",
					val[1], val[0]);

}

void mapphone_gadget_init(char *boot_mode)
{
	if (!strncmp(boot_mode, "bp-tools", BOOT_MODE_MAX_LEN))
		andusb_plat.bp_tools_mode = 1;

	andusb_plat.performance_mode = set_usb_performance_mode;

	mapphone_get_serial_number();
	mapphone_get_product_name();
	/* Initialize the USB nluns from device tree */
	mapphone_init_nluns();
	mapphone_init_cdrom_lun_num();
	/* Initialize the USB PID's from the device tree */
	usb_pid_mapping_init();

	platform_device_register(&android_usb_platform_device);
	platform_driver_register(&cpcap_usb_connected_driver);

}

/* USBHost Related  Board/Platform Data Starts Here*/

static int mapphone_usb_fsport_startup(void)
{
	int r;
	r = gpio_request(MAPPHONE_IPC_USB_SUSP_GPIO, "ipc_usb_susp");
	if (r < 0) {
		printk(KERN_WARNING "Could not request GPIO %d"
		" for IPC_USB_SUSP\n",
		MAPPHONE_IPC_USB_SUSP_GPIO);
		return r;
	}
	gpio_direction_output(MAPPHONE_IPC_USB_SUSP_GPIO, 0);
	printk(KERN_INFO "%s - Configured GPIO 95 for USB Suspend \n",
			__func__);
	return 0;
}

static int mapphone_usb_fsport_suspend(int on)
{
	pr_debug("FSUSB PHY %s\n", on ? "suspended" : "resumed");
	if (on)
		gpio_direction_output(MAPPHONE_IPC_USB_SUSP_GPIO, 1);
	else {
		gpio_direction_output(MAPPHONE_IPC_USB_SUSP_GPIO, 0);
		/* Delay 100 usec before enabling clocks */
		udelay(100);
	}

	return 0;
}

static struct clk *ipc_ext_usb_clk;

static void mapphone_hsusb_ext_ts_init(int clk_num)
{
	int gpio, ret;
	char clk_name[16];

	gpio = get_gpio_by_name("ipc_ext_usb_ts_cs");
	if (gpio >= 0) {
		ret = gpio_request(gpio, "ipc ext usb ts cs");
		if (ret) {
			printk(KERN_ERR "Cannot request GPIO %d\n", gpio);
			return;
		}
		gpio_direction_output(gpio, 1);
	}

	if (clk_num >= 0 && clk_num < 6) {
		snprintf(clk_name, sizeof(clk_name), "auxclk%d_ck", clk_num);
		ipc_ext_usb_clk = clk_get(NULL, clk_name);
		if (!ipc_ext_usb_clk) {
			printk(KERN_ERR "Cannot get auxclk%d_ck\n", clk_num);
			goto failed_clk1;
		}

		ret = clk_enable(ipc_ext_usb_clk);
		if (ret) {
			printk(KERN_ERR "Cannot enable auxclk%d_ck\n", clk_num);
			goto failed_clk2;
		}
	}

	printk(KERN_INFO "External USB Transceiver for IPC init done\n");
	return;

failed_clk2:
	clk_put(ipc_ext_usb_clk);
	ipc_ext_usb_clk = 0;
failed_clk1:
	gpio_free(gpio);
}

static struct resource oob_wake_resources[] = {
        [0] = {
                .flags = IORESOURCE_IRQ,
        },
};

/* LTE USB Out-of-Band Wakeup Device */
static struct oob_wake_platform_data oob_wake_pdata = {
        .vendor = 0x22b8,
        .product = 0x4267,
};

static struct platform_device oob_wake_device = {
        .name   = "oob-wake",
        .id     = -1,
        .dev    = {
                .platform_data = &oob_wake_pdata,
        },
        .resource = oob_wake_resources,
        .num_resources = ARRAY_SIZE(oob_wake_resources),
};

static struct resource mdm6600_resources[] = {
	[0] = {
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device mdm6600_usb_device = {
	.name   = "mdm6600_modem",
	.id     = -1,
	.resource = mdm6600_resources,
	.num_resources = ARRAY_SIZE(mdm6600_resources),
};

static struct mdm6600_usb_platform_data qsc_usb_pdata = {
	.modem_interface = 4,
	.remote_wake_gpio = -1
};

static struct platform_device qsc_usb_device = {
	.name   = "qsc6085_modem",
	.id     = -1,
	.dev    = {
		.platform_data = &qsc_usb_pdata,
	},
};

static struct usbhs_omap_board_data usbhs_bdata  = {
	.port_mode[0] = OMAP_OHCI_PORT_MODE_PHY_4PIN_DPDM,
	.port_mode[1] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
	.phy_reset  = false,
	.reset_gpio_port[0]  = -EINVAL,
	.reset_gpio_port[1]  = -EINVAL,
	.reset_gpio_port[2]  = -EINVAL,
	.ehci_phy_vbus_not_used = false,
};


void __init mapphone_usbhost_init(void)
{
	struct device_node *node;
	const void *prop;
	int i, size, rwkup_gpio;
	int feature_usbhost = 1;
	int feature_ipc_ohci_phy = 0;
	int feature_ipc_ehci_phy = 0;
	int ipc_hsusb_aux_clk = -1;

	node = of_find_node_by_path(DT_HIGH_LEVEL_FEATURE);
	if (node) {
		prop = of_get_property(node, "feature_usbhost_en", &size);
		if (prop && size) {
			feature_usbhost = *(u8 *)prop;
			printk(KERN_NOTICE "%s USBHost in the Platform \n",
				feature_usbhost ? "Enabling" : "Disabling");
		}
		of_node_put(node);
	}

	if (!feature_usbhost)
		return;

	node = of_find_node_by_path(DT_PATH_CHOSEN);
	if (node) {
		prop = of_get_property(node, DT_PROP_CHOSEN_USBHS_PORT0, &size);
		if (prop && size) {
			usbhs_bdata.port_mode[0] = *(u8 *)prop;
			printk(KERN_NOTICE "Overriding USBHS Port0 Mode : %d\n",
				*(u8 *)prop);
		}
		prop = of_get_property(node, DT_PROP_CHOSEN_USBHS_PORT1, &size);
		if (prop && size) {
			usbhs_bdata.port_mode[1] = *(u8 *)prop;
			printk(KERN_NOTICE "Overriding USBHS Port1 Mode : %d\n",
				*(u8 *)prop);
		}
		prop = of_get_property(node, DT_PROP_CHOSEN_USBHS_PORT2, &size);
		if (prop && size) {
			usbhs_bdata.port_mode[2] = *(u8 *)prop;
			printk(KERN_NOTICE "Overriding USBHS Port2 Mode : %d\n",
				*(u8 *)prop);
		}
		prop = of_get_property(node,
				DT_PROP_CHOSEN_IPC_USB_TS_CLK_SRC, &size);
		if (prop && size) {
			ipc_hsusb_aux_clk = *(int *)prop;
			printk(KERN_NOTICE "Used AUX Clock Out Pin : %d\n",
				*(u8 *)prop);
		}
		of_node_put(node);
	}

	for (i = 0; i < OMAP3_HS_USB_PORTS; i++) {
		if (usbhs_bdata.port_mode[i] ==
					OMAP_OHCI_PORT_MODE_PHY_4PIN_DPDM)
			feature_ipc_ohci_phy = 1;
		else if (usbhs_bdata.port_mode[i] == OMAP_EHCI_PORT_MODE_PHY)
			feature_ipc_ehci_phy = 1;
	}

	if (feature_ipc_ohci_phy) {
		usbhs_bdata.ohci_phy_suspend = mapphone_usb_fsport_suspend;
		mapphone_usb_fsport_startup();
	}

	if (feature_ipc_ehci_phy) {
		usbhs_bdata.ehci_phy_vbus_not_used = true;
		mapphone_hsusb_ext_ts_init(ipc_hsusb_aux_clk);
		for (i = 0; i < OMAP3_HS_USB_PORTS; i++) {
			if (usbhs_bdata.port_mode[i] == OMAP_EHCI_PORT_MODE_PHY)
				usbhs_bdata.transceiver_clk[i] =
					ipc_ext_usb_clk;
		}
	}

	usbhs_init(&usbhs_bdata);

	rwkup_gpio = get_gpio_by_name("mdm6600_usb_rwkup");

	if (rwkup_gpio >= 0) {
		gpio_request(rwkup_gpio, "mdm6600_usb_rwkup");
		gpio_direction_input(rwkup_gpio);
		mdm6600_resources[0].start = gpio_to_irq(rwkup_gpio);
		mdm6600_resources[0].end = gpio_to_irq(rwkup_gpio);
		platform_device_register(&mdm6600_usb_device);
	} else {
		platform_device_register(&qsc_usb_device);
	}

	rwkup_gpio = get_gpio_by_name("lte_wan_hostwake");

	if (rwkup_gpio >= 0) {
		oob_wake_resources[0].start = gpio_to_irq(rwkup_gpio);
		oob_wake_resources[0].end = gpio_to_irq(rwkup_gpio);
		platform_device_register(&oob_wake_device);
	} else
		pr_debug("Out of Band Remote Wakeup for CDC  not defined\n");
}
