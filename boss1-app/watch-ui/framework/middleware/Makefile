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

# config mode
CSRCS += config_crypto.c
CSRCS += config_manager.c

# pipeline mode
CSRCS += pipeline.c

# ipc mode
CSRCS += ipc_server.c
CSRCS += ipc_client.c
CSRCS += ipc_checksum.c
CSRCS += ipc_handler_config.c
# CSRCS += ipc_handler_apwifi.c
# CSRCS += ipc_handler_stawifi.c
CSRCS += ipc_client_config.c

# Include paths
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/system/libuv/libuv/include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/framework/middleware
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/include

# Dependencies
DEPPATH += --dep-path config
DEPPATH += --dep-path config
DEPPATH += --dep-path ipc
DEPPATH += --dep-path ipc/ipc_server
DEPPATH += --dep-path ipc/ipc_client
DEPPATH += --dep-path pipeline

VPATH += :config
VPATH += :ipc
VPATH += :ipc/ipc_server
VPATH += :ipc/ipc_client
VPATH += :pipeline

include $(APPDIR)/Application.mk
