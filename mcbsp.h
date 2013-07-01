/*******************************************************************************
	mcbsp.h: McBSP interface for ads1672 driver.

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

#ifndef __ADS1672_MCBSP_H_INCLUDED__
#define __ADS1672_MCBSP_H_INCLUDED__

#include <plat/dma.h>

/**
 * ads1672_mcbsp_start: Start McBSP streaming.
 */
void ads1672_mcbsp_start(void);

/**
 * ads1672_mcbsp_stop: Stop McBSP streaming.
 */
void ads1672_mcbsp_stop(void);

/**
 * ads1672_mcbsp_init: Initilaize the interface to the McBSP streaming driver.
 */
int ads1672_mcbsp_init(dma_addr_t dma_dest, unsigned int frame_len,
		unsigned int frames);

/**
 * ads1672_mcbsp_init: Close the interface to the McBSP streaming driver.
 */
void ads1672_mcbsp_exit(void);

#endif /* !__ADS1672_MCBSP_H_INCLUDED__ */
