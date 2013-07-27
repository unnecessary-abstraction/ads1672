/*******************************************************************************
	ads1672_dump.c: Dirt-simple dumping of ADS1672 samples.

	Copyright (C) 2013 Paul Barker, Loughborough University
	
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

/* This program should be as simple as possible so that any errors are much more
 * likely to be in the ads1672 driver itself rather than in this code.
 *
 * Data comes out of the ads1672 driver in two's complement format, with the 24
 * bit samples right-justified and sign extended into the 32 bit per sample data
 * buffer. So we simply dump this data into a file at 32-bits per sample. The
 * data will be stored in the endian format of the processor used, ie.
 * little-endian on the Beagleboard or an x86 computer.
 */

#include <ads1672.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

/* Define readable and writable by user, group and other flags if they doesn't
 * already exist.
 */
#ifndef S_IRUGO
#define S_IRUGO (S_IRUSR | S_IRGRP | S_IROTH)
#endif

#ifndef S_IWUGO
#define S_IWUGO (S_IWUSR | S_IWGRP | S_IWOTH)
#endif

static int fh_in = -1;
static int fh_out = -1;
static ads1672_sample_t * buffer = NULL;
static size_t buffer_size = 0;
static bool ads1672_running = false;
static unsigned int max_periods = 0;

void cleanup(void)
{
	/* If the device is running, try to stop it. */
	if (ads1672_running) {
		ads1672_ioctl_stop(fh_in);
		ads1672_running = false;
	}

	/* If the device is open, try to reset the gpio pins to safe states. */
	if (fh_in >= 0) {
		ads1672_ioctl_gpio_select_set(fh_in, 1);
		ads1672_ioctl_gpio_start_set(fh_in, 0);
	}

	/* Close file handles if they don't refer to stdin, stdout or stderr. */
	if (fh_in > 2) {
		close(fh_in);
		fh_in = -1;
	}
	if (fh_out > 2) {
		close(fh_out);
		fh_out = -1;
	}

	/* Free memory, probably unnecessary as this is a standalone program but
	 * I like to be thorough.
	 */
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}
}

/* Simple error handler: Print message, cleanup and abort. */
void error(const char * failing_function)
{
	char s[256];
	snprintf(s, 256, "ads1672_dump: %s failed", failing_function);
	perror(s);
	cleanup();
	abort();
}

void init(void)
{
	mode_t m;
	int r;
	const char * outfile = "dump.dat";	/* TODO: Make configurable. */
	
	/* Open input file. */
	fh_in = open("/dev/ads1672", O_RDONLY);
	if (fh_in < 0)
		error("init: open");

	/* Set chip select pin low now. */
	r = ads1672_ioctl_gpio_select_set(fh_in, 0);
	if (r < 0)
		error("init: ads1672_ioctl_gpio_select_set");

	/* Set start pin low to ensure we're in a known safe state. */
	r = ads1672_ioctl_gpio_start_set(fh_in, 0);
	if (r < 0)
		error("init: ads1672_ioctl_gpio_start_set");

	/* Open output file, set mode 0666, umask will apply to this. */
	m = S_IWUGO | S_IRUGO;
	fh_out = creat(outfile, m);
	if (fh_out < 0)
		error("init: creat");

	/* Allocate a buffer for ADS1672_PERIOD_LENGTH samples so that reads
	 * line up with the periods of the underlying driver.
	 */
	buffer_size = ADS1672_PERIOD_LENGTH * sizeof(ads1672_sample_t);
	buffer = (ads1672_sample_t *) malloc(buffer_size);
	if (!buffer)
		error("init: malloc");

	/* Set maximum periods to dump to 64. At a period length of 64 kSamples
	 * and a sampling frequency of 625 kHz, this would give a recording
	 * length of 16MiB, ~ 4.2 million samples, ~ 6.7 seconds.
	 */
	max_periods = 64;
}

void start(void)
{
	int r;

	r = ads1672_ioctl_start(fh_in);
	if (r < 0)
		error("start: ads1672_ioctl_start");
	ads1672_running = true;

	r = ads1672_ioctl_gpio_start_set(fh_in, 1);
	if (r < 0)
		error("start: ads1672_ioctl_gpio_start_set");
}

void stop(void)
{
	int r;

	r = ads1672_ioctl_gpio_start_set(fh_in, 0);
	if (r < 0)
		error("stop: ads1672_ioctl_gpio_start_set");

	/* Set running flag to false before trying to stop the device. This
	 * prevents the stop from being re-tried in the error handler if it
	 * fails.
	 */
	ads1672_running = false;

	r = ads1672_ioctl_stop(fh_in);
	if (r < 0)
		error("stop: ads1672_ioctl_stop");
}

void run(void)
{
	unsigned int count;
	int r;

	for (count = 0; count < max_periods; count++) {
		r = read(fh_in, buffer, buffer_size);
		if (r < 0)
			error("run: read");

		r = write(fh_out, buffer, buffer_size);
		if (r < 0)
			error("run: write");
	}
}

int main(int argc, char * argv[])
{
	/* Suppress warnings about unused arguments. */
	(void) argc;
	(void) argv;

	init();
	start();
	run();
	stop();
	cleanup();

	return 0;
}
