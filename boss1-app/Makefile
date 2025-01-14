############################################################################
# boss1-apps/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

export APPDIR = $(CURDIR)
include $(APPDIR)/Make.defs

# The GNU make CURDIR will always be a POSIX-like path with forward slashes
# as path segment separators.  This is fine for the above inclusions but
# will cause problems later for the native build.  If we know that this is
# a native build, then we need to fix up the APPDIR path for subsequent
# use

ifeq ($(CONFIG_WINDOWS_NATIVE),y)
  export APPDIR = $(subst /,\,$(CURDIR))
endif

# Symbol table for loadable apps.
#   SYMTABEXT: Extra arguments for mksymtab.sh

SYMTABSRC = symtab_apps.c
SYMTABOBJ = $(SYMTABSRC:.c=$(OBJEXT))

# Build targets

# We first remove libapps.a before letting the other rules add objects to it
# so that we ensure libapps.a does not contain objects from prior build

all: $(BIN)

.PHONY: dirlinks .depdirs preconfig depend clean distclean
.PHONY: context clean_context context_all register register_all
.PRECIOUS: $(BIN)

$(foreach SDIR, $(CONFIGURED_APPS), $(eval $(call SDIR_template,$(SDIR),all)))
$(foreach SDIR, $(CONFIGURED_APPS), $(eval $(call SDIR_template,$(SDIR),install)))
$(foreach SDIR, $(CONFIGURED_APPS), $(eval $(call SDIR_template,$(SDIR),context)))
$(foreach SDIR, $(CONFIGURED_APPS), $(eval $(call SDIR_template,$(SDIR),register)))
$(foreach SDIR, $(CONFIGURED_APPS), $(eval $(call SDIR_template,$(SDIR),depend)))
$(foreach SDIR, $(CLEANDIRS), $(eval $(call SDIR_template,$(SDIR),clean)))
$(foreach SDIR, $(CLEANDIRS), $(eval $(call SDIR_template,$(SDIR),distclean)))

# In FLAT and protected modes, the modules have already been created.  A
# symbol table is required.

ifeq ($(CONFIG_MODULES),)
ifeq ($(CONFIG_WINDOWS_NATIVE),y)
$(BIN): $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_all)
else
$(BIN): $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_all)
endif

else

$(SYMTABSRC): $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_all)
	$(Q) $(MAKE) install
	$(Q) $(APPDIR)$(DELIM)tools$(DELIM)mksymtab.sh $(BINDIR) $(SYMTABEXT) >$@.tmp
	$(Q) $(call TESTANDREPLACEFILE, $@.tmp, $@)

ifneq ($(CONFIG_ARM_TOOLCHAIN_GHS),y)
$(SYMTABOBJ): %$(OBJEXT): %.c
	$(call COMPILE, $<, $@, -fno-lto -fno-builtin)
else
$(SYMTABOBJ): %$(OBJEXT): %.c
	$(call COMPILE, $<, $@, -Onolink)
endif

$(BIN): $(SYMTABOBJ)
	$(call ARLOCK, $(call CONVERT_PATH,$(BIN)), $^)

endif # !CONFIG_MODULES

install: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_install)

dirlinks:
	$(Q) $(MAKE) -C platform dirlinks

context_all: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_context)
register_all: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_register)

staging:
	$(Q) mkdir -p $@

context: | staging
	$(Q) echo "app context: $(CONFIGURED_APPS)"
	$(Q) $(MAKE) context_all
	$(Q) $(MAKE) register_all

Kconfig:
	$(foreach SDIR, $(CONFIGDIRS), $(call MAKE_template,$(SDIR),preconfig))
	$(Q) $(MKKCONFIG)

preconfig: Kconfig

.depdirs: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_depend)

.depend: Makefile .depdirs
	$(Q) touch $@

depend: .depend

clean_context:
	$(Q) $(MAKE) -C platform clean_context
	$(Q) $(MAKE) -C builtin clean_context

clean: $(foreach SDIR, $(CLEANDIRS), $(SDIR)_clean)
	$(call DELFILE, $(BIN))
	$(call DELFILE, Kconfig)
	$(call DELDIR, $(BINDIR))
	$(call CLEAN)

distclean: $(foreach SDIR, $(CLEANDIRS), $(SDIR)_distclean)
	$(call DELFILE, .depend)
	$(call DELFILE, $(BIN))
	$(call DELFILE, Kconfig)
	$(call DELDIR, $(BINDIR))
	$(call DELDIR, staging)
	$(call CLEAN)
