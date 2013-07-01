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

#include <asm/uaccess.h>
#include <plat/dma.h>

enum {
	/**
	* Size of each buffer in bytes.
	*
	* This is currently 256kB in order to fit 64k samples in each buffer for a
	* flip-rate of approximately 10Hz.
	*/
	ADS1672_BUFFER_SIZE = 256 * 1024,

	/**
	* Number of samples in each buffer, calculated from ADS1672_BUFFER_SIZE.
	*/
	ADS1672_BUFFER_COUNT = ADS1672_BUFFER_SIZE / sizeof(int)
};

/**
 * Condition codes.
 *	
 * These codes are only ever set at the start of a new buffer, the error will
 * typically have been in the previous buffer. This is done so that reading the
 * time will give the correct value for the next valid sample.
 *	
 * Thus if the user sees a code other than ADS1672_COND_OK, they should log the
 * condition and the new sample time, call ads1672_buf_clear_cond and then
 * continue reading data.
 */
enum ADS1672_COND {
	/**
	* Buffer filled ok.
	*/
	ADS1672_COND_OK = 0,

	/**
	* Buffer overrun - the device filled the write buffer before the user
	* had finished with the read buffer. The buffers had to be flipped to
	* make space for more data, meaning that several samples may have been
	* lost.
	*/
	ADS1672_COND_OVERRUN = -1,

	/**
	* DMA error received on previous buffer.
	*
	* Samples may have been lost in transit between the McBSP and main
	* memory or the FIFO on the McBSP may overrun before the DMA system is
	* running again.
	*/
	ADS1672_COND_DMA_ERROR = -2,

	/**
	* McBSP stopped.
	*
	* This typically signals that the ADS1672 has stopped producing samples
	* due to the start or power pins being taken low. This condition can be
	* cleared but no data will be available. A further read may cause the
	* user process to sleep indefinitely waiting for new data which will
	* never arrive.
	*/
	ADS1672_COND_STOP = -3,

	/**
	* Buffer is in use for DMA transfer.
	*
	* The user will not typically see this condition code as the read
	* function will sleep waiting for the DMA transfer to complete. If the
	* user does see it, it should be ignored and reading continued as usual.
	*/
	ADS1672_COND_IN_USE = -4,

	/**
	* Invalid or unitialized buffer.
	*
	* This is typically the result of an internal error or an attempt to use
	* an ADS1672 device which has been unloaded. Clearing this condition
	* code and attempting to read data has undefined results and may crash
	* the system.
	*/
	ADS1672_COND_INVALID = -5
};

/**
 * Read data from ADS1672 device, kernel version.
 *	\param [out] out	A buffer in kernel space.
 *	\param [in] count	Maximum number of samples to read.
 *
 * \returns number of samples actually read or <0 on error.
 */
int ads1672_buf_readk(int * out, size_t count);

/**
 * Read data from ADS1672 device, user space version.
 *	\param [out] out	A buffer in user space.
 *	\param [in] count	Maximum number of samples to read.
 *
 * \returns number of samples actually read or <0 on error.
 *
 * Data is copied using copy_to_user.
 */
int ads1672_buf_readu(int __user * out, size_t count);

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
 * Initialize buffering for an ADS1672 device.
 */
int ads1672_buf_init(void);

/**
 * Delete buffering for an ADS1672 device.
 */
void ads1672_buf_exit(void);

#endif /* !__ADS1672_BUFFER_H_INCLUDED__ */
