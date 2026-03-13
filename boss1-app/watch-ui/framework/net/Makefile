############################################################################
# vendor/boss/app/boss1-app/watch-ui/framework/net/Makefile
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

# WiFi module
CSRCS += wifi.c

# WiFi Manager module
CSRCS += wifi_state.c
CSRCS += wifi_manager.c
CSRCS += pipeline_sta_connect.c
CSRCS += pipeline_ap_start.c

# DNS Server
CSRCS += dns_server.c

# DHCP Server
CSRCS += dhcp_server.c

# Include paths
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/framework/net
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/framework/middleware

# Dependencies
DEPPATH += --dep-path wifi
DEPPATH += --dep-path wifi_manager
DEPPATH += --dep-path dns_server
DEPPATH += --dep-path dhcp_server

VPATH += :wifi
VPATH += :wifi_manager
VPATH += :dns_server
VPATH += :dhcp_server

include $(APPDIR)/Application.mk
