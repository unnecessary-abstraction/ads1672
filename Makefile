################################################################################
#	Makefile for ADS1672 driver.
#
#	Copyright (C) 2011-2013 Paul Barker, Loughborough University
#	
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
################################################################################

SRC := $(shell pwd)
KERNEL_SRC := $(SRC)/kernel
EXTRA_CFLAGS := -I$(SRC)/include

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)/module EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)/module modules_install

install: modules_install
	install -m 0755 -d $(INSTALL_MOD_PATH)/sbin
	install -m 0755 sbin/ads1672_load sbin/ads1672_unload \
		$(INSTALL_MOD_PATH)/sbin

clean:
	rm -rf module/*.o module/*.ko module/*.mod.c module/.*.cmd \
		module/.tmp_versions module/*.order module/*.symvers \
		module/*.o.d
