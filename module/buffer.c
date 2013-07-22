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
 * buffer.c
 * Input buffering for ads1672 driver.
 */

#include <ads1672.h>
#include <linux/dma-mapping.h>
#include <linux/completion.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "buffer.h"

/******************************************************************************
	Private declarations and functions
*******************************************************************************/

struct ads1672_period_status {
	/* Condition code. */
	int				cond;

	/* Number of valid samples. */
	int				nr_samples;

	/* Timespec at start of buffer. */
	struct timespec			ts;
};

static ads1672_sample_t *		buffer = NULL;
static dma_addr_t			buffer_dma = 0;

static struct completion		period_completion;
static uint				current_read_period;
static uint				current_write_period;
static uint				current_read_offset;
static struct ads1672_period_status *	period_status;

static int prep_read(uint * count)
{
	uint avail;
	
	if (*count < 1)
		return -EINVAL;
	
	/* If the current buffer is marked as in use, then the following should
	 * apply:
	 * 	period_status[current_read_period].nr_samples = 0
	 * 	current_read_offset = 0
	 *
	 * Therefore:
	 * 	available samples = 0
	 *
	 * So we need to check whether the current buffer is marked as in use
	 * before we calculate the number of available samples.
	 */
	/* If the current read period is the same as the current write period,
	 * wait for the write to finish. */
	if (current_read_period == current_write_period)
		wait_for_completion(&period_completion);

	/* On an error condition the number of available samples may also be
	 * zero, so again we need to check for this before calculating the
	 * number of available samples.
	 */
	if (period_status[current_read_period].cond != ADS1672_COND_OK)
		return -EIO;
	
	/* Now the number of available samples can only be zero if the current
	 * period is valid but all data has been read.
	 */
	avail = period_status[current_read_period].nr_samples - current_read_offset;
	if (avail == 0) {
		/* Move to the next period and wait if it is marked as in use.
		 */
		current_read_offset = 0;
		current_read_period++;
		if (current_read_period == ads1672_nr_periods)
			current_read_period = 0;
		if (current_read_period == current_write_period)
			wait_for_completion(&period_completion);

		/* We need to check for an error condition again. */
		if (period_status[current_read_period].cond != ADS1672_COND_OK)
			return -EIO;

		/* Recalculate available samples - this should never be zero */
		avail = period_status[current_read_period].nr_samples -
			current_read_offset;
	}

	/* Read upto count samples from the current offset to end of current
	 * period.
	 */
	if (*count > avail)
		*count = avail;

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

uint				ads1672_nr_periods;
uint				ads1672_period_length;

int ads1672_buf_readk(ads1672_sample_t * out, uint count)
{
	int r;
	uint index;
	
	r = prep_read(&count);
	if (r < 0)
		return r;

	index = current_read_period * ads1672_period_length + current_read_offset;

	memcpy(out, &buffer[index], count * sizeof(ads1672_sample_t));
	current_read_offset += count;
	return count;
}

int ads1672_buf_readu(ads1672_sample_t __user * out, uint count)
{
	int r;
	uint index;
	
	r = prep_read(&count);
	if (r < 0)
		return r;

	index = current_read_period * ads1672_period_length + current_read_offset;

	r = copy_to_user(out, &buffer[index], count * sizeof(ads1672_sample_t));
	if (r != 0)
		return -EIO;
	
	current_read_offset += count;
	return count;
}

void ads1672_buf_complete(int cond, uint nr_samples)
{
	/* Set values of the finished period. */
	period_status[current_write_period].cond = cond;
	period_status[current_write_period].nr_samples = nr_samples;
	period_status[current_write_period].ts.tv_sec = 0;
	period_status[current_write_period].ts.tv_nsec = 0;

	/* Advance the current write period. */
	current_write_period++;
	if (current_write_period == ads1672_nr_periods)
		current_write_period = 0;

	/* Check for overrun - if this has happened, advance the current read
	 * period and mark the overrun condition.
	 */
	if (current_write_period == current_read_period) {
		current_read_offset = 0;
		current_read_period++;
		if (current_read_period == ads1672_nr_periods)
			current_read_period = 0;
		period_status[current_read_period].cond = ADS1672_COND_OVERRUN;
	}

	/* Mark the new write period as in use just incase. */
	period_status[current_write_period].cond = ADS1672_COND_IN_USE;
	period_status[current_write_period].nr_samples = 0;

	/* Raise the completion incase someone was waiting for data. */
	complete(&period_completion);
}

void ads1672_buf_flush(void)
{
	/* Discard current buffer and move to the next one. We assume that the
	 * reader knows what they are doing and will correct any timing info it
	 * holds.
	 */
	current_read_period++;
	if (current_read_period == ads1672_nr_periods)
		current_read_period = 0;
	current_read_offset = 0;
}

void ads1672_buf_clear_cond(void)
{
	/* We assume the caller knows what they're doing. */
	period_status[current_read_period].cond = ADS1672_COND_OK;
}

dma_addr_t ads1672_buf_get_dma_addr(void)
{
	return buffer_dma;
}

int ads1672_buf_get_cond(void)
{
	return period_status[current_read_period].cond;
}

void ads1672_buf_get_timespec(struct timespec * ts)
{
	*ts = period_status[current_read_offset].ts;
}

int ads1672_buf_init(void)
{
	size_t buffer_size;

	/* Set ads1672_nr_periods and ads1672_period_length to constants from the header.
	 * Keeping these as variables allows them to be changed later.
	 */
	ads1672_nr_periods = ADS1672_NR_PERIODS;
	ads1672_period_length = ADS1672_PERIOD_LENGTH;

	buffer_size = ads1672_nr_periods * ads1672_period_length *
		sizeof(ads1672_sample_t);
	
	buffer = (ads1672_sample_t *) dma_alloc_coherent(NULL, buffer_size,
			&buffer_dma, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;
	
	period_status = (struct ads1672_period_status *) kmalloc(ads1672_nr_periods *
			sizeof(struct ads1672_period_status), GFP_KERNEL);
	if (!period_status) {
		dma_free_coherent(NULL, buffer_size, buffer, buffer_dma);
		return -ENOMEM;
	}
	
	init_completion(&period_completion);

	current_read_period = 0;
	current_write_period = 0;
	current_read_offset = 0;

	/* Mark the first period as in use. */
	period_status[0].cond = ADS1672_COND_IN_USE;
	period_status[0].nr_samples = 0;
	period_status[0].ts.tv_sec = 0;
	period_status[0].ts.tv_nsec = 0;
	
	return 0;
}

void ads1672_buf_exit(void)
{
	if (buffer) {
		size_t buffer_size = ads1672_nr_periods * ads1672_period_length *
			sizeof(ads1672_sample_t);

		dma_free_coherent(NULL, buffer_size, buffer, buffer_dma);
		buffer = NULL;
		buffer_dma = 0;
	}

	if (period_status) {
		kfree(period_status);
		period_status = NULL;
	}
}
