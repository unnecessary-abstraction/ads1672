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
 * gpio.c
 * GPIO interface for ads1672 driver.
 */

#include <linux/gpio.h>
#include <plat/mux.h>

#include "gpio.h"

/* GPIO pin numbers. */
#define ADS1672_GPIO_START      138
#define ADS1672_GPIO_SELECT     139

int ads1672_gpio_start_get(void)
{
        return gpio_get_value(ADS1672_GPIO_START);
}

void ads1672_gpio_start_set(int value)
{
        gpio_set_value(ADS1672_GPIO_START, value);
}

int ads1672_gpio_select_get(void)
{
        return gpio_get_value(ADS1672_GPIO_SELECT);
}

void ads1672_gpio_select_set(int value)
{
        gpio_set_value(ADS1672_GPIO_SELECT, value);
}

int ads1672_gpio_init(void)
{
        int r;
        
        r = gpio_request_one(ADS1672_GPIO_START, GPIOF_OUT_INIT_LOW, "ADS1672 Start");
        if (r < 0)
                return r;
        
        r = gpio_request_one(ADS1672_GPIO_SELECT, GPIOF_OUT_INIT_HIGH, "ADS1672 Select");
        if (r < 0)
                return r;
                
        return 0;
}

void ads1672_gpio_exit(void)
{
        gpio_free(ADS1672_GPIO_START);
        gpio_free(ADS1672_GPIO_SELECT);
}
