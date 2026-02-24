############################################################################
# vendor/boss/app/boss1-app/watch-ui/httpd/Makefile
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

include $(APPDIR)/Make.defs

# THTTPD Web Server
CSRCS = romfs.c
CSRCS += httpd_server.c
CSRCS += libhttpd.c
CSRCS += thttpd_cgi.c
CSRCS += thttpd_alloc.c
CSRCS += thttpd_strings.c
CSRCS += timers.c
CSRCS += fdwatch.c
CSRCS += tdate_parse.c
CSRCS += thttpd.c

CONTENT_MAKE = $(MAKE) -C content

VPATH += :content :thttpd_lib
DEPPATH += --dep-path content --dep-path thttpd_lib

content/romfs.c: build

# Include paths
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/net
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/net/pipeline
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/net/wifi
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/net/wifi_manager
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/net/dns_server
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/net/dhcp_server
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/httpd/thttpd_lib

.PHONY: build
build:
	+$(Q) $(CONTENT_MAKE) TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

# Common build

context::
	+$(Q) $(CONTENT_MAKE) context TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

clean::
	+$(Q) $(CONTENT_MAKE) clean TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

distclean::
	+$(Q) $(CONTENT_MAKE) distclean TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

include $(APPDIR)/Application.mk
