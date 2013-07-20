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
 * \file buffer.h
 * Input buffering for ads1672 driver.
 */

#ifndef __ADS1672_BUFFER_H_INCLUDED__
#define __ADS1672_BUFFER_H_INCLUDED__

#include <ads1672.h>
#include <asm/uaccess.h>
#include <plat/dma.h>

/**
 * Read data from ADS1672 device, kernel version.
 *	\param [out] out	A buffer in kernel space.
 *	\param [in] count	Maximum number of samples to read.
 *
 * \returns number of samples actually read or <0 on error.
 */
int ads1672_buf_readk(ads1672_sample_t * out, size_t count);

/**
 * Read data from ADS1672 device, user space version.
 *	\param [out] out	A buffer in user space.
 *	\param [in] count	Maximum number of samples to read.
 *
 * \returns number of samples actually read or <0 on error.
 *
 * Data is copied using copy_to_user.
 */
int ads1672_buf_readu(ads1672_sample_t __user * out, size_t count);

/**
 * Flip read and write buffers on successfully filling the write buffer.
 */
void ads1672_buf_flip(struct timespec ts);

/**
 * Flip read and write buffers on an error condition.
 */
void ads1672_buf_err_and_flip(int cond, int valid_samples, struct timespec ts);

/**
 * Discard remaining data in current buffer and perform flip.
 */
void ads1672_buf_flush(void);

/**
 * Reset condition code to OK.
 */
void ads1672_buf_clear_cond(void);

/**
 * Get the base DMA address of the buffer.
 */
dma_addr_t ads1672_buf_get_dma_addr(void);

/**
 * Get the current condition value of the buffer.
 */
int ads1672_buf_get_cond(void);

/**
 * Get the current timespec of the buffer.
 */
void ads1672_buf_get_timespec(struct timespec * ts);

/**
 * Initialize buffering for an ADS1672 device.
 */
int ads1672_buf_init(void);

/**
 * Delete buffering for an ADS1672 device.
 */
void ads1672_buf_exit(void);

#endif /* !__ADS1672_BUFFER_H_INCLUDED__ */
