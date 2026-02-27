#############################################################################
# apps/netutils/cjson/Makefile
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
#############################################################################

# Standard includes

include $(APPDIR)/Make.defs

# Set up build configuration and environment

WD := ${shell echo $(CURDIR) | sed -e 's/ /\\ /g'}

CONFIG_NETUTILS_CJSON_URL ?= "https://github.com/DaveGamble/cJSON/archive"
CONFIG_NETUTILS_CJSON_VERSION ?= "1.7.10"
CJSON_VERSION = $(patsubst "%",%,$(strip $(CONFIG_NETUTILS_CJSON_VERSION)))

CJSON_TARBALL = v$(CJSON_VERSION).tar.gz
CJSON_UNPACKNAME = cJSON

UNPACK ?= tar -zxf

CJSON_UNPACKDIR =  $(WD)/$(CJSON_UNPACKNAME)
CJSON_SRCDIR = $(CJSON_UNPACKNAME)

APPS_INCDIR = $(APPDIR)$(DELIM)include$(DELIM)netutils

CFLAGS += ${INCDIR_PREFIX}$(APPS_INCDIR)
CFLAGS += -DCJSON_INCLUDE_CONFIG_H

VPATH += :$(CJSON_SRCDIR)
DEPPATH += --dep-path $(CJSON_SRCDIR)

CSRCS = cJSON.c
CSRCS += cJSON_Utils.c

# Use local cJSON source directory (offline build)
# Source: https://github.com/DaveGamble/cJSON/archive/v1.7.12.tar.gz
ifeq ($(wildcard $(CJSON_UNPACKNAME)/cJSON.c),)
$(error cJSON source not found. Please unpack cJSON source to $(CJSON_UNPACKNAME) directory)
endif

# cJSON test (without unity framework)
ifneq ($(CONFIG_NETUTILS_CJSON_TEST),)
VPATH += :tests
DEPPATH += --dep-path tests
CSRCS += parse_tests.c print_tests.c misc_tests.c

PROGNAME += cjson_test
MAINSRC += test.c
PRIORITY  = $(CONFIG_CJSON_TEST_PRIORITY)
STACKSIZE = $(CONFIG_CJSON_TEST_STACKSIZE)
endif

$(CJSON_SRCDIR)$(DELIM)cJSON.h: $(CJSON_UNPACKNAME)

$(CJSON_SRCDIR)$(DELIM)cJSON_Utils.h: $(CJSON_UNPACKNAME)

$(APPS_INCDIR)$(DELIM)cJSON.h: $(CJSON_SRCDIR)$(DELIM)cJSON.h
	$(Q) cp $< $@

$(APPS_INCDIR)$(DELIM)cJSON_Utils.h: $(CJSON_SRCDIR)$(DELIM)cJSON_Utils.h
	$(Q) cp $< $@

context:: $(APPS_INCDIR)$(DELIM)cJSON.h $(APPS_INCDIR)$(DELIM)cJSON_Utils.h

distclean::
	@:

include $(APPDIR)/Application.mk
