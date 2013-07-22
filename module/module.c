/*
 * Copyright (C) 2011-2013 Paul Barker, Loughborough University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * module.c
 * Module init and exit for ads1672 driver.
 */

#include <ads1672.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "buffer.h"
#include "device.h"
#include "gpio.h"
#include "mcbsp.h"
#include "module.h"

void ads1672_cleanup(void)
{
	/* Delete device objects. */
	ads1672_device_exit();

	/* Delete hardware interface.*/
	ads1672_mcbsp_exit();
	ads1672_gpio_exit();

	/* Delete buffering. */
	ads1672_buf_exit();
}

int __init ads1672_init(void)
{
	int r;
	dma_addr_t dma_addr;

	/* Initialize buffering. */
	r = ads1672_buf_init();
	if (r < 0) {
		printk(KERN_ERR "ads1672: Failed to initialize buffering. "
				"Aborting module init...\n");
		ads1672_cleanup();
		return r;
	}

	dma_addr = ads1672_buf_get_dma_addr();
	if (dma_addr == 0) {
		printk(KERN_ERR "ads1672: Failed to get DMA address of buffer. "
				"Aborting module init...\n");
		ads1672_cleanup();
		return -EIO;
	}
	
	/* Initialize hardware interface. */
	r = ads1672_gpio_init();
	if (r < 0) {
		printk(KERN_ERR "ads1672: Failed to initialize GPIO. "
				"Aborting module init...\n");
		ads1672_cleanup();
		return r;
	}
		
	r = ads1672_mcbsp_init(dma_addr);
	if (r < 0) {
		printk(KERN_ERR "ads1672: Failed to initialize McBSP. "
				"Aborting module init...\n");
		ads1672_cleanup();
		return r;
	}

	/* Initialize character and platform device objects. */
	r = ads1672_device_init();
	if (r < 0) {
		printk(KERN_ERR "ads1672: Failed to initialize device objects. "
				"Aborting module init...\n");
		ads1672_cleanup();
		return r;
	}

	printk(KERN_ALERT "ads1672: Loaded\n");
	return 0;
}

void __exit ads1672_exit(void)
{
	ads1672_cleanup();

	printk(KERN_ALERT "ads1672: Unloaded\n");
}

module_init(ads1672_init);
module_exit(ads1672_exit);

MODULE_LICENSE("GPL");
