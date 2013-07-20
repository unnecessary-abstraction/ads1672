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
 * \file mcbsp.h
 * McBSP interface for ads1672 driver.
 */

#ifndef __ADS1672_MCBSP_H_INCLUDED__
#define __ADS1672_MCBSP_H_INCLUDED__

#include <plat/dma.h>

/**
 * Start McBSP streaming.
 */
void ads1672_mcbsp_start(void);

/**
 * Stop McBSP streaming.
 */
void ads1672_mcbsp_stop(void);

/**
 * Get the current status of the McBSP interface.
 *
 * \returns A combination of flags from ::ADS1672_STATUS describing the current
 * state.
 */
int ads1672_mcbsp_status(void);

/**
 * Initilaize the McBSP interface.
 */
int ads1672_mcbsp_init(dma_addr_t dma_dest, unsigned int frame_len,
		unsigned int frames);

/**
 * Close the McBSP interface.
 */
void ads1672_mcbsp_exit(void);

#endif /* !__ADS1672_MCBSP_H_INCLUDED__ */
