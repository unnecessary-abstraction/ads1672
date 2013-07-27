################################################################################
#	rules.mk for ads1672 kernel module.
#
#	Copyright (C) 2013 Paul Barker, Loughborough University
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
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
################################################################################

# Push directory stack
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

# If INSTALL_MOD_PATH hasn't been set but DESTDIR has, use that for installing
# kernel modules
INSTALL_MOD_PATH := $(DESTDIR)
export INSTALL_MOD_PATH

# Targets and intermediates in this directory
TGTS_$(d) := $(d)/ads1672.ko
TARGETS_MODULE += $(TGTS_$(d))

INSTALL_DEPS += install-$(d)
CLEAN_DEPS += clean-$(d)

# Rules for this directory
$(d)/ads1672.ko: .FORCE
	$(MAKE) -C "$(KERNEL_SRCDIR)" M="$(SRCDIR)/module" \
		EXTRA_CFLAGS="$(CFLAGS_ALL)" CC="$(KERNEL_CC)" \
		LD="$(KERNEL_LD)" AR="$(KERNEL_AR)" modules

.PHONY: install-$(d)
install-$(d): $(TGTS_$(d))
	@echo INSTALL $^
	$(MAKE) -C "$(KERNEL_SRCDIR)" M="$(SRCDIR)/module" \
		EXTRA_CFLAGS="$(CFLAGS_ALL)" CC="$(KERNEL_CC)" \
		LD="$(KERNEL_LD)" AR="$(KERNEL_AR)" modules_install

.PHONY: clean-$(d)
clean-$(d):
	@echo CLEAN $^
	$(Q)rm -rf $(d)/*.o $(d)/*.ko $(d)/*.mod.c $(d)/.*.cmd $(d)/.tmp_versions \
		$(d)/*.order $(d)/*.symvers $(d)/*.o.d

# Pop directory stack
d := $(dirstack_$(sp))
sp := $(basename $(sp))
