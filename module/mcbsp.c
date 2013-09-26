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
 * mcbsp.c
 * McBSP interface for ads1672 driver.
 */

#include <ads1672.h>
#include <linux/string.h>
#include <plat/mcbsp.h>
#include <plat/dma.h>

#include "buffer.h"
#include "mcbsp.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

/* McBSP1 data receive register. */
#define OMAP35XX_MCBSP1_DRR 0x48074000

/* Delay in McBSP clock cycles between frame sync pulse and first data bit.
 *
 * TODO: Set to correct value.
 */
#define ADS1672_DATA_DELAY	0

/* McBSP to which the ADS1672 is attached. */
#define ADS1672_MCBSP_ID	0

/* Allocated DMA channel */
static int dma_lch;

/* It's useful to keep track of the current status. */
static int mcbsp_status = 0;

/* DMA callback function */
static void ads1672_mcbsp_callback(int lch, u16 ch_status, void *data)
{
	/* We know we're running with synchronisation enabled so we don't care
	 * about the SYNC bit in ch_status. The flags in ch_status (CSR
	 * register) are conveniently the same as the flags in the IRQ enable
	 * register (CICR).
	 */
	ch_status &= ~ OMAP1_DMA_SYNC_IRQ;

	/* What we want is "End of frame" events - if any other bit is set it
	 * signals an error condition.
	 */
	if (ch_status == OMAP_DMA_FRAME_IRQ) {
		ads1672_buf_complete(ADS1672_COND_OK, ads1672_period_length);
		/* We'd like to reset the status bit incase we get an IRQ for an
		 * error condition before the next end of frame event, however
		 * there doesn't seem to be a function for this in the current
		 * API.
		 */	
	} else {
		printk(KERN_ERR "ads1672: Transfer error 0x%04x", ch_status);
		ads1672_buf_complete(ADS1672_COND_DMA_ERROR, 0);

		/* TODO: What happens to the internal pointer the DMA subsystem
		 * holds? It's incremented each sample and somehow needs to be
		 * bumped to the start of the next period after an error.
		 */
	}
}

/*******************************************************************************
	Public functions
*******************************************************************************/

void ads1672_mcbsp_start(void)
{
	omap_start_dma(dma_lch);

	/* Start transfer. */
	omap_mcbsp_start(ADS1672_MCBSP_ID, 0, 1);

	mcbsp_status |= ADS1672_STATUS_RUNNING;

	printk(KERN_ALERT "ads1672: Started\n");
}

void ads1672_mcbsp_stop(void)
{
	/* Stop McBSP. */
	omap_mcbsp_stop(ADS1672_MCBSP_ID, 0, 1);

	/* Stop dma transfer. */
	omap_stop_dma(dma_lch);

	mcbsp_status &= ~ADS1672_STATUS_RUNNING;

	printk(KERN_ALERT "ads1672: Stopped\n");
}

int ads1672_mcbsp_status(void)
{
	return mcbsp_status;
}

int ads1672_mcbsp_init(dma_addr_t dma_dest)
{
	int r;
	struct omap_mcbsp_reg_cfg config;

	/* Init mcbsp. */
	r = omap_mcbsp_request(ADS1672_MCBSP_ID);
	if (r < 0)
		return r;

	memset(&config, 0, sizeof(config));

	/* The data from the ads1672 comes in signed twos complement format,
	 * MSB-first. The 24 bit samples should be right-justified and
	 * sign-extended into the McBSP data register.
	 */
	config.spcr1 = RJUST(1);
	
	/* Delay between frame sync pulse and first data bit. */
	config.rcr2 = RDATDLY(ADS1672_DATA_DELAY);

	/* 24 bit word length. */
	config.rcr1 = RWDLEN1(4);

	/* Clock resynchronised on each frame sync pulse, with SCLKME=0, clock
	 * derived from MCBSPi_ICLK (96MHz).
	 */
	config.srgr2 = GSYNC | CLKSM;

	/* Divide clock by 3 giving approx 22MHz on the Beagleboard. */
	config.srgr1 = CLKGDV(3);

	/* CLKR is an output driven by internal clock. FSRM=0, FSR is an input
	 * and drives internal frame sync. CLKRP=1, sample of rising edge of
	 * clock signal rather than falling edge.
	 */
	config.pcr0 = CLKRM | CLKRP;

	/* Enable DMA on receive. */
	config.rccr = RDMAEN;

	omap_mcbsp_config(ADS1672_MCBSP_ID, &config);
	
	/* Init dma transfer. */
	r = omap_request_dma(OMAP24XX_DMA_MCBSP1_RX, "ads1672",
			ads1672_mcbsp_callback, NULL, &dma_lch);
	if (r < 0)
		return r;

	/* Select only desired interrupts. */
	omap_disable_dma_irq(dma_lch, 0xFFFF);
	omap_enable_dma_irq(dma_lch, OMAP_DMA_DROP_IRQ | OMAP_DMA_FRAME_IRQ |
			OMAP2_DMA_TRANS_ERR_IRQ | OMAP2_DMA_SUPERVISOR_ERR_IRQ |
			OMAP2_DMA_MISALIGNED_ERR_IRQ);

	omap_set_dma_transfer_params(dma_lch, OMAP_DMA_DATA_TYPE_S32,
			ads1672_period_length, ads1672_nr_periods, OMAP_DMA_SYNC_ELEMENT,
			OMAP24XX_DMA_MCBSP1_RX, OMAP_DMA_SRC_SYNC);
	
	omap_set_dma_src_params(dma_lch, 0, OMAP_DMA_AMODE_CONSTANT,
			OMAP35XX_MCBSP1_DRR, 0, 0);
				
	omap_set_dma_dest_params(dma_lch, 0, OMAP_DMA_AMODE_POST_INC, dma_dest,
			0, 0);

	/* Link the DMA channel to itself. */
	omap_dma_link_lch(dma_lch, dma_lch);

	mcbsp_status |= ADS1672_STATUS_READY;
	
	return 0;
}

void ads1672_mcbsp_exit(void)
{
	/* Ensure device is stopped. */
	ads1672_mcbsp_stop();
	
	omap_free_dma(dma_lch);

	/* Close mcbsp. */
	omap_mcbsp_free(ADS1672_MCBSP_ID);

	mcbsp_status &= ~ADS1672_STATUS_READY;
}
