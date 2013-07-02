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
 * \file device.h
 * ads1672 character device.
 */

#ifndef __ADS1672_DEVICE_H_INCLUDED__
#define __ADS1672_DEVICE_H_INCLUDED__

/**
 * Get first device number used by ADS1672 module.
 */
dev_t ads1672_get_dev(void);

/**
 * Initialise the ads1672 character and platform device objects.
 */
int ads1672_device_init(void);

/**
 * Destroy the ads1672 character and platform device objects.
 */
void ads1672_device_exit(void);

#endif /* !__ADS1672_DEVICE_H_INCLUDED__ */
