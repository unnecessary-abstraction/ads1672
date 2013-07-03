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
 * device.c
 * ads1672 character device.
 */

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include "buffer.h"
#include "device.h"
#include "gpio.h"
#include "mcbsp.h"

/*******************************************************************************
	Private declarations and data.
*******************************************************************************/

/* Handle read operation on an ADS1672 device. */
static ssize_t ads1672_read(struct file *f,
			    char __user *buf,
			    size_t count,
			    loff_t *offp);

/* Handle open operation on an ADS1672 device. */
static int ads1672_open(struct inode *inode, struct file *f);

/* Handle close operation on an ADS1672 device. */
static int ads1672_release(struct inode *inode, struct file *f);

static struct cdev		cdev;
static struct platform_device	plat;

/* Declare file operations for ADS1672 device. */
static struct file_operations fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.read		= ads1672_read,
	.open		= ads1672_open,
	.release	= ads1672_release,
};

/* Device major number, set as 0 to use dynamic allocation of device numbers. */
static int			major = 0;
module_param(major, int, S_IRUGO);

/* Device minor number. */
static int			minor = 0;
module_param(minor, int, S_IRUGO);

/*******************************************************************************
	Private functions.
*******************************************************************************/

static ssize_t ads1672_read(struct file *f,
			    char __user *buf,
			    size_t count,
			    loff_t *offp)
{
	int r;

	r = ads1672_buf_readu((int __user *)buf, count/4);

	/*
		Return value of ads1672_buf_readu is in samples not bytes but we
		don't want to multiply an error code.
	*/
	if (r < 0)
		return r;
	else
		return r * sizeof(int);
}

static int ads1672_open(struct inode *inode, struct file *f)
{
	if (inode->i_rdev != ads1672_get_dev())
		return -ENODEV;

	return 0;
}

static int ads1672_release(struct inode *inode, struct file *f)
{
	/* Nothing to do. */
	return 0;
}

static ssize_t ads1672_status_show(struct device *dev, struct device_attribute *unused, char *buf)
{
	int status = ads1672_mcbsp_status();

	return scnprintf(buf, PAGE_SIZE, "%d\n", status);
}

static ssize_t ads1672_status_store(struct device *dev, struct device_attribute *unused, const char *buf, size_t count)
{
	int status;
	int r = kstrtoint(buf, 0, &status);
	if (r < 0)
		return r;

	if (status & ADS1672_STATUS_RUNNING)
		ads1672_mcbsp_start();
	else
		ads1672_mcbsp_stop();

	return count;
}

static ssize_t ads1672_gpio_start_show(struct device *dev, struct device_attribute *unused, char *buf)
{
	int status = ads1672_gpio_start_get();

	return scnprintf(buf, PAGE_SIZE, "%d\n", status);
}

static ssize_t ads1672_gpio_start_store(struct device *dev, struct device_attribute *unused, const char *buf, size_t count)
{
	int status;
	int r = kstrtoint(buf, 0, &status);
	if (r < 0)
		return r;

	ads1672_gpio_start_set(status);

	return count;
}

static ssize_t ads1672_gpio_select_show(struct device *dev, struct device_attribute *unused, char *buf)
{
	int status = ads1672_gpio_select_get();

	return scnprintf(buf, PAGE_SIZE, "%d\n", status);
}

static ssize_t ads1672_gpio_select_store(struct device *dev, struct device_attribute *unused, const char *buf, size_t count)
{
	int status;
	int r = kstrtoint(buf, 0, &status);
	if (r < 0)
		return r;

	ads1672_gpio_select_set(status);

	return count;
}

/* Declare sysfs attributes for ADS1672 device. */
static DEVICE_ATTR(status, 0660, ads1672_status_show, ads1672_status_store);
static DEVICE_ATTR(gpio_start, 0660, ads1672_gpio_start_show, ads1672_gpio_start_store);
static DEVICE_ATTR(gpio_select, 0660, ads1672_gpio_select_show, ads1672_gpio_select_store);

/*******************************************************************************
	Public functions.
*******************************************************************************/

dev_t ads1672_get_dev(void)
{
	return MKDEV(major, minor);
}

int ads1672_device_init(void)
{
	int r;
	dev_t dev;

	if (major) {
		dev = ads1672_get_dev();
		r = register_chrdev_region(dev, 1, "ads1672");
		
		if (r < 0) {
			printk(KERN_WARNING "ads1672: "
					"Cannot register char device %d,%d\n",
					major, minor);
			return r;
		}
	} else {
		r = alloc_chrdev_region(&dev, minor, 1, "ads1672");
		major = MAJOR(dev);

		if (r < 0) {
			printk(KERN_WARNING "ads1672: "
					"Cannot allocate char device "
					"minor=%d\n", minor);
			return r;
		}
	}

	/* Blank structures so we know they don't contain garbage.*/
	memset(&cdev, 0, sizeof(cdev));
	memset(&plat, 0, sizeof(plat));

	/* Setup character device. */
	cdev_init(&cdev, &fops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &fops;
	
	r = cdev_add(&cdev, dev, 1);
	if (r < 0) {
		printk(KERN_WARNING "ads1672: "
				"Error %d when adding char device\n", r);
		return r;
	}

	/* Initialize platform device object so we appear in sysfs. */
	plat.name = "ads1672";
	plat.id = -1;
	plat.num_resources = 0;

	r = platform_device_register(&plat);
	if (r < 0) {
		printk(KERN_WARNING "ads1672: "
				"Error %d initializing platform device\n", r);
		cdev_del(&cdev);
		return r;
	}

	r = device_create_file(&plat.dev, &dev_attr_status);
	if (r < 0) {
		printk(KERN_WARNING "ads1672: "
				"Error %d creating 'status' device attribute\n",
				r);
		platform_device_unregister(&plat);
		cdev_del(&cdev);
		return r;
	}

	r = device_create_file(&plat.dev, &dev_attr_gpio_start);
	if (r < 0) {
		printk(KERN_WARNING "ads1672: "
				"Error %d creating 'gpio_start' device attribute\n",
				r);
		platform_device_unregister(&plat);
		cdev_del(&cdev);
		return r;
	}

	r = device_create_file(&plat.dev, &dev_attr_gpio_select);
	if (r < 0) {
		printk(KERN_WARNING "ads1672: "
				"Error %d creating 'gpio_select' device attribute\n",
				r);
		platform_device_unregister(&plat);
		cdev_del(&cdev);
		return r;
	}

	return 0;
}

void ads1672_device_exit(void)
{
	platform_device_unregister(&plat);
	cdev_del(&cdev);
}
