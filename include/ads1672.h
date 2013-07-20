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

/**
 * \file ads1672.h
 * User header for operations on the ads1672 device.
 */

#ifndef __ADS1672_IOCTL_H_INCLUDED__
#define __ADS1672_IOCTL_H_INCLUDED__

#include <linux/ioctl.h>
#include <linux/time.h>

enum ADS1672_IOCTL {
	ADS1672_IOCTL_MAGIC = '=',

	ADS1672_IOCTL_START = _IO(ADS1672_IOCTL_MAGIC, 1),
	ADS1672_IOCTL_STOP = _IO(ADS1672_IOCTL_MAGIC, 2),
	ADS1672_IOCTL_GPIO_START_SET = _IOW(ADS1672_IOCTL_MAGIC, 3, int),
	ADS1672_IOCTL_GPIO_START_GET = _IOR(ADS1672_IOCTL_MAGIC, 4, int),
	ADS1672_IOCTL_GPIO_SELECT_SET = _IOW(ADS1672_IOCTL_MAGIC, 5, int),
	ADS1672_IOCTL_GPIO_SELECT_GET = _IOR(ADS1672_IOCTL_MAGIC, 6, int),
	ADS1672_IOCTL_CLEAR_CONDITION = _IO(ADS1672_IOCTL_MAGIC, 7),
	ADS1672_IOCTL_GET_TIMESPEC = _IOR(ADS1672_IOCTL_MAGIC, 8, struct timespec),
	ADS1672_IOCTL_GET_CONDITION = _IOR(ADS1672_IOCTL_MAGIC, 9, int),
};

#endif /* !__ADS1672_IOCTL_H_INCLUDED__ */
