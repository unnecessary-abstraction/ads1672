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
 * ads1672 status codes, currently only used by McBSP interface.
 */
enum ADS1672_STATUS {
	/**
	 * McBSP interface running.
	 */
	ADS1672_STATUS_RUNNING = 0x0001,

	/**
	 * McBSP interface ready.
	 */
	ADS1672_STATUS_READY = 0x0002
};

#endif /* !__ADS1672_IOCTL_H_INCLUDED__ */
