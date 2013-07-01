/*******************************************************************************
        gpio.h: GPIO interface for ads1672 driver.

        Copyright (C) 2011-2013 Paul Barker, Loughborough University

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*******************************************************************************/

#ifndef __ADS1672_GPIO_H_INCLUDED__
#define __ADS1672_GPIO_H_INCLUDED__

/**
 * ads1672_gpio_start_get: Get the status of the start pin. The return value is
 * is 1 for a high pin state or 0 for a low pin state.
 */
int ads1672_gpio_start_get(void);

/**
 * ads1672_gpio_start_set: Set the start pin status to value. A value of 1 means
 * high and a value of 0 means low.
 */
void ads1672_gpio_start_set(int value);

/**
 * ads1672_gpio_select_get: Get the status of the chip select pin. The return
 * value is 1 for a high pin state or 0 for a low pin state.
 */
int ads1672_gpio_select_get(void);

/**
 * ads1672_gpio_select_set: Set the start pin status to value. A value of 1
 * means high and a value of 0 means low.
 */
void ads1672_gpio_select_set(int value);

/**
 * ads1672_gpio_init: Initialize GPIO pins used by ADS1672 device.
 */
int ads1672_gpio_init(void);

/**
 * ads1672_gpio_exit: Free GPIO pins used by ADS1672 device.
 */
void ads1672_gpio_exit(void);

#endif /* !__ADS1672_GPIO_H_INCLUDED__ */
