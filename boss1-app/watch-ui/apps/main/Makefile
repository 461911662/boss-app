############################################################################
# apps/watch-ui/apps/login/Makefile
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

CXXEXT := .cpp

CXXSRCS += watchui.cpp

DEPPATH += --dep-path work
VPATH += :work
CXXSRCS += pwm.cpp

# Include paths
CXXFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/framework
CXXFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/framework/middleware
CXXFLAGS += ${INCDIR_PREFIX}$(APPDIR)/watch-ui/framework/httpd/lib

ifeq ($(CONFIG_OFFLINE_CXX_LIB),y)

LIBCXX_PATH = $(TOPDIR)$(DELIM)libs$(DELIM)libxx
OFFLINE_CXX_LIB_PATH = $(TOPDIR)$(DELIM)$(CONFIG_OFFLINE_CXX_LIB_PATH)

LIBCXX_VERSION=$(patsubst "%",%,$(CONFIG_LIBCXX_VERSION))
LIBCXXABI_VERSION=$(patsubst "%",%,$(CONFIG_LIBCXXABI_VERSION))
LIBCXX = $(LIBCXX_PATH)$(DELIM)libcxx-$(LIBCXX_VERSION).src.tar.xz
LIBCXXABI = $(LIBCXX_PATH)$(DELIM)libcxxabi-$(LIBCXXABI_VERSION).src.tar.xz

$(LIBCXX):
	$(Q) echo "GEN libcxx-$(LIBCXXABI_VERSION).src.tar.xz"
	$(Q) flock $(LIBCXX_PATH)$(DELIM)libcxx.lock \
			cp -rf $(OFFLINE_CXX_LIB_PATH)$(DELIM)libcxx-$(LIBCXXABI_VERSION).src.tar.xz $@

$(LIBCXXABI):
	$(Q) echo "GEN libcxxabi-$(LIBCXXABI_VERSION).src.tar.xz"
	$(Q) flock $(LIBCXX_PATH)$(DELIM)libcxxabi.lock \
			cp -rf  $(OFFLINE_CXX_LIB_PATH)$(DELIM)libcxxabi-$(LIBCXXABI_VERSION).src.tar.xz $@

context:: | $(LIBCXX) $(LIBCXXABI)
	@:

endif

include $(APPDIR)/Application.mk
