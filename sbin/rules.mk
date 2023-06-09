################################################################################
#	rules.mk for ads1672 system binaries.
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

# Targets and intermediates in this directory
TGTS_$(d) := $(d)/ads1672_load $(d)/ads1672_unload

INSTALL_DEPS += install-$(d)

# Rules for this directory
.PHONY: install-$(d)
install-$(d): $(TGTS_$(d))
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(sbindir)
	$(Q)$(INSTALL) -m 0755 $^ $(DESTDIR)$(sbindir)

# Pop directory stack
d := $(dirstack_$(sp))
sp := $(basename $(sp))
