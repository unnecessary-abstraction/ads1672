################################################################################
#	Top-level rules.mk for ads1672.
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

# Required flags which cannot be configured by the user
CFLAGS_ALL := -I$(SRCDIR)/include
LDFLAGS_ALL := 

# Required libraries
LDLIBRARIES_ALL := 

# Initialise empty variables which each directory's `rules.mk` will append to.
TARGETS_BIN :=
TARGETS_SBIN :=
TARGETS_MODULE :=
INTERMEDIATES :=
INSTALL_DEPS :=
CLEAN_DEPS :=

# Building outside of the source tree doesn't work for the kernel module
#VPATH := $(SRCDIR)

# One rule to bring them all and in the darkness bind them
.PHONY: all
all: targets

# Pull in subdirectories
dir := bin
include $(SRCDIR)/$(dir)/rules.mk

dir := sbin
include $(SRCDIR)/$(dir)/rules.mk

dir := module
include $(SRCDIR)/$(dir)/rules.mk

dir := include
include $(SRCDIR)/$(dir)/rules.mk

# Combined list of targets
TARGETS_ALL := $(TARGETS_BIN) $(TARGETS_SBIN) $(TARGETS_MODULE)

.PHONY: targets
targets: $(TARGETS_ALL)

# Compiler rule
%.o: %.c
	@echo CC $@
	$(Q)$(CC) $(CFLAGS) $(CFLAGS_ALL) $(CFLAGS_TGT) $(DEPFLAGS) -o $@ -c $<
	$(Q)$(PYTHON) $(SRCDIR)/scripts/fixdeps.py $*.d $*.d.tmp
	$(Q)mv $*.d.tmp $*.d

# Linker rule
%: %.o
	@echo CCLD $@
	$(Q)$(CCLD) $(LDFLAGS) $(LDFLAGS_ALL) $(LDFLAGS_TGT) -o $@ $(filter %.o,$^) $(LDLIBRARIES_TGT) $(LDLIBRARIES_ALL) $(LDLIBRARIES)

# Clean rules
.PHONY: clean clean-intermediates
clean: clean-intermediates $(CLEAN_DEPS)
	@echo CLEAN
	$(Q)rm -rf $(TARGETS_ALL)

clean-intermediates:
	@echo CLEAN INTERMEDIATES
	$(Q)rm -rf $(INTERMEDIATES)

# Install rules
.PHONY: install 
install: $(INSTALL_DEPS)

# Phony rule to force a target to always be built
.PHONY: .FORCE
.FORCE:

# Ensure everything is rebuilt if top-level rules or config change
TOPLEVEL_DEPS := $(SRCDIR)/rules.mk $(SRCDIR)/Makefile unconfig.mk

$(INTERMEDIATES): $(TOPLEVEL_DEPS)

$(TARGETS_BIN): $(TOPLEVEL_DEPS)

$(TARGETS_SBIN): $(TOPLEVEL_DEPS)

$(TARGETS_MODULE): $(TOPLEVEL_DEPS)
