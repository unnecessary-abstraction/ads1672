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
#include <asm/uaccess.h>

#include "buffer.h"

/******************************************************************************
	Private declarations and functions
*******************************************************************************/

/* A buffer used by the ADS1672 device. */
struct ads1672_buffer {
	/* Buffer start location. */
	ads1672_sample_t *	start;
	
	/* Number of valid samples. */
	int			nsamples;

	/* Condition code at end of buffer. */
	int			cond;

	/* Timespec at start of buffer. */
	struct timespec		ts;
};

static ads1672_sample_t *	buffer_base = NULL;
static dma_addr_t		buffer_base_dma = 0;

static ads1672_sample_t *	read_ptr = NULL;
static struct completion	on_flip;

static struct ads1672_buffer	buffers[2];
static struct ads1672_buffer *	read_buf = NULL;
static struct ads1672_buffer *	write_buf = NULL;

static int prep_read(size_t count)
{
	int len;
	
	if (count < 1)
		return -EINVAL;
	
	len = (read_buf->start + read_buf->nsamples) - read_ptr;
	
	/* If buffer is in use or all data has been read, wait for a flip. */
	if ( (read_buf->cond == ADS1672_COND_IN_USE) || (len == 0) )
		wait_for_completion(&on_flip);
	
	/* Check for error condition. */
	if (read_buf->cond != ADS1672_COND_OK)
		return -EIO;
	
	/* Read upto count samples from read_ptr to end of current buffer. */
	if (len > count)
		len = (int)count;

	return len;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int ads1672_buf_readk(ads1672_sample_t * out, size_t count)
{
	int len = prep_read(count);
	if (len < 0)
		return len;

	memcpy(out, read_ptr, len * sizeof(int));
	read_ptr += len;
	return len;
}

int ads1672_buf_readu(ads1672_sample_t __user * out, size_t count)
{
	unsigned long r;
	int len = prep_read(count);
	if (len < 0)
		return len;

	r = copy_to_user(out, read_ptr, len * sizeof(int));
	if (r != 0)
		return -1;
	
	read_ptr += len;
	return len;
}

void ads1672_buf_flip(struct timespec ts)
{
	ads1672_buf_err_and_flip(ADS1672_COND_OK, ADS1672_BUFFER_COUNT, ts);
}

void ads1672_buf_err_and_flip(int cond, int valid_samples, struct timespec ts)
{
	struct ads1672_buffer * tmp;
	
	/* Check whether previous buffer has been read. */
	if (read_ptr != (read_buf->start + read_buf->nsamples)) {
		/* Set condition to overrun if no other condition is set. */
		if (cond == ADS1672_COND_OK)
			cond = ADS1672_COND_OVERRUN;
	}
	
	/* Flip buffers. */
	tmp = read_buf;
	read_buf = write_buf;
	write_buf = tmp;
	
	/* Setup new read and write buffers. */
	read_buf->nsamples = valid_samples;
	read_buf->cond = cond;
	read_ptr = read_buf->start;
	
	write_buf->nsamples = 0;
	write_buf->ts = ts;
	write_buf->cond = ADS1672_COND_IN_USE;
	
	complete(&on_flip);
}

void ads1672_buf_flush(void)
{
	struct ads1672_buffer * tmp;
	
	/*
	   Discard current buffer and move to the opposite one. If a following
	   read operation is initiated before new data is available, it will
	   wait for the "on_flip" completion.
	
	   We assume that the reader knows what they are doing and will correct
	   any timing info it holds.
	*/
	
	/* Flip buffers. */
	tmp = read_buf;
	read_buf = write_buf;
	write_buf = tmp;
	
	read_ptr = read_buf->start;
}

void ads1672_buf_clear_cond(void)
{
	/* We assume the caller knows what they're doing. */
	read_buf->cond = ADS1672_COND_OK;
}

dma_addr_t ads1672_buf_get_dma_addr(void)
{
	return buffer_base_dma;
}

int ads1672_buf_get_cond(void)
{
	return read_buf->cond;
}

void ads1672_buf_get_timespec(struct timespec * ts)
{
	*ts = read_buf->ts;
}

int ads1672_buf_init(void)
{	
	buffer_base = (ads1672_sample_t *)dma_alloc_coherent(NULL,
			2 * ADS1672_BUFFER_SIZE, &buffer_base_dma, GFP_KERNEL);
	if (!buffer_base)
		return -ENOMEM;
	
	init_completion(&on_flip);
	
	read_buf = &buffers[0];
	read_buf->start = buffer_base;
	read_buf->cond = ADS1672_COND_OK;
	read_buf->nsamples = 0;
	read_ptr = read_buf->start;
	
	write_buf = &buffers[1];
	write_buf->start = buffer_base + ADS1672_BUFFER_SIZE;
	write_buf->cond = ADS1672_COND_IN_USE;
	
	return 0;
}

void ads1672_buf_exit(void)
{
	if (buffer_base) {
		dma_free_coherent(NULL, 2 * ADS1672_BUFFER_SIZE, buffer_base, buffer_base_dma);
		buffer_base = NULL;
		buffer_base_dma = 0;
	}
	
	read_buf = NULL;
	write_buf = NULL;
	
	buffers[0].cond = ADS1672_COND_INVALID;
	buffers[0].start = NULL;
	
	buffers[1].cond = ADS1672_COND_INVALID;
	buffers[1].start = NULL;
}
